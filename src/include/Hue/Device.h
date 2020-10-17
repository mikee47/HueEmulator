/**
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
 * You should have received a copy of the GNU General Public License along with FlashString.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#pragma once

#include <Network/UPnP/Enumerator.h>
#include <WString.h>
#include <BitManipulations.h>
#include <ArduinoJson6.h>

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

String getErrorDesc(Error error);
JsonObject createSuccess(JsonDocument& result);
JsonObject createError(JsonDocument& result, const String& path, Error error, String description);

class Device : public UPnP::Item
{
public:
	typedef uint32_t ID;

	enum class Attribute {
#define XX(t) t,
		HUE_DEVICE_ATTR_MAP(XX)
#undef XX
	};

	enum class ColorMode {
#define XX(t) t,
		HUE_COLORMODE_MAP(XX)
#undef XX
	};

	/**
	 * @brief Callback invoked when setAttribute() has completed
	 * @param errorCode Internal error code, >= 0 for success, < 0 for error
	 */
	using Callback = Delegate<void(int errorCode)>;

	virtual ~Device()
	{
	}

	virtual ID getId() const = 0;

	virtual String getName() const = 0;

	/**
	 * @brief Set a device attribute asynchronously
	 * @param attr
	 * @param value
	 * @param callback Invoked when attribute has been set
	 * @retval bool true on success, false if attribute not supported or failed to queue I/O request
	 */
	virtual bool setAttribute(Attribute attr, unsigned value, Callback callback) = 0;

	/**
	 * @brief Get the (cachced) device attribute value
	 * @param attr
	 * @param value
	 * @retval bool true on success, false if attribute not supported or value unknown
	 */
	virtual bool getAttribute(Attribute attr, unsigned& value) = 0;

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

class OnOffDevice : public Device
{
public:
	OnOffDevice(ID id, const String& name) : id(id), name(name)
	{
	}

	ID getId() const override
	{
		return id;
	}

	String getName() const override
	{
		return name;
	}

	bool getAttribute(Attribute attr, unsigned& value) override
	{
		switch(attr) {
		case Attribute::on:
			value = on;
			return true;
		default:
			return false;
		}
	}

	bool setAttribute(Attribute attr, unsigned value, Callback callback) override
	{
		switch(attr) {
		case Attribute::on:
			this->on = value;
			return true;
		default:
			return false;
		}
	}

private:
	ID id;
	String name;
	bool on = false;
};

class DimmableDevice : public OnOffDevice
{
public:
	DimmableDevice(ID id, const String& name) : OnOffDevice(id, name)
	{
	}

	bool getAttribute(Attribute attr, unsigned& value) override
	{
		switch(attr) {
		case Attribute::bri:
			value = bri;
			return true;
		default:
			return OnOffDevice::getAttribute(attr, value);
		}
	}

	bool setAttribute(Attribute attr, unsigned value, Callback callback) override
	{
		switch(attr) {
		case Attribute::bri:
			this->bri = value;
			return true;
		default:
			return OnOffDevice::setAttribute(attr, value, callback);
		}
	}

private:
	uint8_t bri = 1;
};

class ColourDevice : public DimmableDevice
{
public:
	ColourDevice(ID id, const String& name) : DimmableDevice(id, name)
	{
	}

	bool getAttribute(Attribute attr, unsigned& value) override
	{
		switch(attr) {
		case Attribute::sat:
			value = sat;
			return true;
		case Attribute::hue:
			value = hue;
			return true;
		case Attribute::ct:
			value = ct;
			return true;
		default:
			return DimmableDevice::getAttribute(attr, value);
		}
	}

	bool setAttribute(Attribute attr, unsigned value, Callback callback) override
	{
		switch(attr) {
		case Attribute::sat:
			sat = value;
			return true;
		case Attribute::hue:
			hue = value;
			return true;
		case Attribute::ct:
			ct = value;
			return true;
		default:
			return DimmableDevice::setAttribute(attr, value, callback);
		}
	}

private:
	uint8_t sat = 0;
	uint16_t hue = 0;
	uint16_t ct = 234;
};

String toString(Device::Attribute attr);
bool fromString(const char* tag, Device::Attribute& attr);
String toString(Device::ColorMode mode);

/**
 * @brief Abstract class to manage a list of devices
 * @note Applications must provide an implementation of this for the bridge.
 * Returned device objects may only be considered valid for the duration of the current
 * task call as they may be destroyed at any time.
 */
class DeviceEnumerator : public UPnP::Enumerator<Device, DeviceEnumerator>
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

} // namespace Hue
