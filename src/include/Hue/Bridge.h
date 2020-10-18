/**
 * Bridge.h
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

#include "Device.h"
#include <Network/HttpServer.h>
#include <Network/WebConstants.h>
#include <SimpleTimer.h>
#include <Network/UPnP/RootDevice.h>

namespace Hue
{
enum class Model {
	LWB004, ///< Dimmable white
	LWB007, ///< Colour
};

struct Stats {
	struct {
		uint16_t count;   ///< Total number of HTTP requests
		uint16_t root;	///< eRequests handled by root UPnP device
		uint16_t ignored; ///< Requests not starting with /api
		uint16_t getAllDeviceInfo;
		uint16_t getDeviceInfo;
		uint16_t setDeviceInfo;
	} request;
	struct {
		uint16_t count;
		size_t size; ///< Total size of response data
	} response;
	struct {
		uint16_t count;
		uint16_t resourceNotAvailable;
		uint16_t methodNotAvailable;
		uint16_t unauthorizedUser;
	} error;

	void serialize(JsonObject json) const;
};

struct User {
	String deviceType;
	uint16_t count = 0;
	bool authorized = false;
};

// Mapped by username
using UserMap = HashMap<String, User>;

class Bridge : public UPnP::RootDevice
{
public:
	struct Config {
		enum class Type {
			AuthorizeUser,
			RevokeUser,
		};

		Type type;
		String deviceType; // How device identifies itself
		String name;	   // randomly generated key
	};

	/**
	 * @brief Called when a new user key is created. This must be stored permanently somewhere
	 * and passed back via `configure()` during system startup.
	 */
	using ConfigDelegate = Delegate<void(const Config& config)>;

	using StateChangeDelegate = Delegate<void(const Hue::Device& device, Hue::Device::Attributes attr)>;

	Bridge(DeviceEnumerator& devices) : devices(devices)
	{
		// Gen 3+, old versions use 1901
		setTcpPort(80);
	}

	void configure(const Config& config);

	/**
	 * @brief Enable creation of new users
	 * @note DO NOT leave this permanently enabled!
	 * This could be enabled via web page on local Access Point, or physical push-button.
	 * It should also be time limited, so exits pairing mode after maybe 30 seconds.
	 * If a user creation request is received then this is disabled automatically.
	 */
	void enablePairing(bool enable)
	{
		pairingEnabled = enable;
	}

	String getField(Field desc) override;

	void onConfigChange(ConfigDelegate delegate)
	{
		configDelegate = delegate;
	}

	void onStateChanged(StateChangeDelegate delegate)
	{
		stateChangeDelegate = delegate;
	}

	void begin();

	String getSerial() const;

	bool formatMessage(SSDP::Message& msg, SSDP::MessageSpec& ms) override;

	bool onHttpRequest(HttpServerConnection& connection) override;

	const Stats& getStats()
	{
		return stats;
	}

	void resetStats()
	{
		memset(&stats, 0, sizeof(stats));
	}

	const UserMap& getUsers() const
	{
		return users;
	}

	void getStatusInfo(JsonObject json);

	void deviceStateChanged(const Hue::Device& device, Hue::Device::Attributes changed)
	{
		if(stateChangeDelegate) {
			stateChangeDelegate(device, changed);
		}
	}

private:
	void createUser(JsonObjectConst request, JsonDocument& result, const String& path);
	bool validateUser(const char* userName);
	void handleApiRequest(HttpServerConnection& connection);

private:
	UserMap users;
	bool pairingEnabled = false;
	DeviceEnumerator& devices;
	ConfigDelegate configDelegate;
	StateChangeDelegate stateChangeDelegate;
	Stats stats;
};

} // namespace Hue
