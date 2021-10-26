/****
 * Device.h
 *
 * Copyright 2019 mikee47 <mike@sillyhouse.net>
 *
 * This file is part of the HueEmulator Library
 *
 * This library is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, version 3 or later.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this library.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#pragma once

#include <Network/UPnP/Enumerator.h>
#include <WString.h>
#include <BitManipulations.h>
#include <ArduinoJson6.h>
#include <Data/BitSet.h>

#define HUE_ERROR_CODE_MAP(XX)                                                                                         \
	XX(1, UnauthorizedUser, "unauthorized user")                                                                       \
	XX(3, ResourceNotAvailable, "resource, <resource>, not available")                                                 \
	XX(4, MethodNotAvailable, "method, <method_name>, not available for resource, <resource>")                         \
	XX(6, ParameterNotAvailable, "parameter, <parameter>, not available")                                              \
	XX(101, LinkButtonNotPressed, "link button not pressed")                                                           \
	XX(901, InternalError, "Internal error, <error_code>")

namespace Hue
{
#define HUE_DEVICE_ATTR_MAP(XX)                                                                                        \
	XX(on)                                                                                                             \
	XX(bri)                                                                                                            \
	XX(ct)                                                                                                             \
	XX(hue)                                                                                                            \
	XX(sat)

#define HUE_COLORMODE_MAP(XX)                                                                                          \
	XX(none)                                                                                                           \
	XX(hs)                                                                                                             \
	XX(ct)                                                                                                             \
	XX(xy)

enum class Error {
#define XX(code, tag, desc) tag = code,
	HUE_ERROR_CODE_MAP(XX)
#undef XX
};

/**
 * @brief Status of a `setAttribute` request
 */
enum class Status {
	/**
	 * @brief The action was performed immediately without error
	 */
	success,
	/**
	 * @brief The action was accepted but requires further processing
	 *
	 * Use this to perform requests asynchronously.
	 * You MUST invoked the provided `Callback` function to complete the request.
	 *
	 * When controlling remote devices, for example connected via serial link,
	 * you might issue the command immediately and then return `pending`. When the serial
	 * response is received, or a timeout occurs, then the request can be completed.
	 * Note that the error code passed to the callback is optional and will be specific
	 * to your application: it will be output in verbose debug mode so may be useful.
	 *
	 */
	pending,
	/**
	 *  @brief Action could not be completed
	 *
	 *  If the Attribute not supported by your device, or an internal I/O error occured
	 *  then return this value.
	 */
	error,
};

String toString(Error error);
JsonObject createSuccess(JsonDocument& result);
JsonObject createError(JsonDocument& result, const String& path, Error error, String description);

class Device : public UPnP::Item
{
public:
	using ID = uint32_t;

	enum class Attribute {
#define XX(t) t,
		HUE_DEVICE_ATTR_MAP(XX)
#undef XX
	};

	using Attributes = BitSet<uint8_t, Attribute>;

	enum class ColorMode {
#define XX(t) t,
		HUE_COLORMODE_MAP(XX)
#undef XX
	};

	/**
	 * @brief Callback invoked when setAttribute() has completed
	 * @param status Result of the operation
	 * @param errorCode Application-specific error code
	 * @note Any status other than `success` is considered a failure
	 */
	using Callback = Delegate<void(Status status, int errorCode)>;

	/**
	 * @brief Abstract class to manage a list of devices
	 * @note Applications must provide an implementation of this for the bridge.
	 * Returned device objects may only be considered valid for the duration of the current
	 * task call as they may be destroyed at any time.
	 */
	class Enumerator : public UPnP::Enumerator<Device, Enumerator>
	{
	public:
		/**
		 * @brief Lookup device by ID
		 * @retval Device* nullptr if not found
		 * @note With default implementation, enumerator position is updated
		 */
		virtual Device* find(Device::ID id);

		/**
		 * @brief Lookup device by name
		 * @retval Device* nullptr if not found
		 * @note With default implementation, enumerator position is updated
		 */
		virtual Device* find(const String& name);
	};

	virtual ~Device()
	{
	}

	virtual ID getId() const = 0;

	virtual String getName() const = 0;

	/**
	 * @brief Set a device attribute
	 * @param attr The attribute to change
	 * @param value Value for the attribute (exact type is attribute-specific)
	 * @param callback If you return Status::pending, invoke this callback when completed
	 * @retval Status
	 * @note DO NOT invoke the callback directly: only use it if pended.
	 */
	virtual Status setAttribute(Attribute attr, unsigned value, Callback callback) = 0;

	/**
	 * @brief Get the (cached) device attribute value
	 * @param attr
	 * @param value
	 * @retval bool true on success, false if attribute not supported or value unknown
	 */
	virtual bool getAttribute(Attribute attr, unsigned& value) const = 0;

	/**
	 * @brief Returns the unique device ID string
	 * @retval String Unique ID of the form AA:BB:CC:DD:EE:FF:00:11-XX,
	 * consisting of a 64-bit Zigbee MAC address plus unique endpoint ID.
	 * @note Other forms of ID string may be used, however for maximum
	 * compatibility the standard format should be used.
	 * By default, this method uses the WiFi station MAC address,
	 * with 00:11 appended plus the 8-bit device ID.
	 */
	virtual String getUniqueId() const;

	virtual ColorMode getColorMode() const
	{
		return ColorMode::none;
	}

	virtual void getInfo(JsonObject json);

	/**
	 * @brief Two devices are considered equal if they have the same ID
	 */
	bool operator==(const Device& dev) const
	{
		return getId() == dev.getId();
	}

	bool operator==(const String& name) const
	{
		return this->getName() == name;
	}

	bool operator==(ID id) const
	{
		return getId() == id;
	}
};

String toString(Device::Attribute attr);
String toString(Device::Attributes attr);
bool fromString(const char* tag, Device::Attribute& attr);
String toString(Device::ColorMode mode);

} // namespace Hue
