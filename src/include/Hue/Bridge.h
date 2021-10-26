/****
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
 * You should have received a copy of the GNU General Public License along with this library.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#pragma once

#include "Device.h"
#include "Stats.h"
#include <Network/HttpServer.h>
#include <Data/WebConstants.h>
#include <SimpleTimer.h>
#include <Network/UPnP/schemas-upnp-org/ClassGroup.h>

namespace Hue
{
enum class Model {
	LWB004, ///< Dimmable white
	LWB007, ///< Colour
};

/**
 * @brief Information about user
 */
struct User {
	String deviceType;		///< How the user identifies themselves
	uint16_t count{0};		///< Number of requests received from this user
	bool authorized{false}; ///< Only authorized users may perform actions
};

/**
 * @brief List of users, key is user name
 */
using UserMap = HashMap<String, User>;

class Bridge : public UPnP::schemas_upnp_org::device::Basic1Template<Bridge>
{
public:
	/**
	 *
	 */
	struct Config {
		enum class Type {
			AuthorizeUser,
			RevokeUser,
		};

		Type type;		   ///< Configuration action to perform
		String deviceType; ///< How device identifies itself
		String name;	   ///< Randomly generated key
	};

	/**
	 * @brief Called when a new user key is created
	 *
	 * The application should use this to store new users in persistent memory.
	 * At startup, these should be passed back via the `configure()` method.
	 */
	using ConfigDelegate = Delegate<void(const Config& config)>;

	/**
	 * @brief A global callback may be provided to perform actions when device states change
	 * @param device The device which has been updated
	 * @param attr A set of flags indicating which attributes were changed
	 *
	 * The callback is invoked only when all request actions have been completed.
	 * The current state may be quereied using `device::getAttribute`.
	 *
	 */
	using StateChangeDelegate = Delegate<void(const Hue::Device& device, Hue::Device::Attributes attr)>;

	/**
	 * @brief Constructor
	 * @param devices List of devices to present
	 */
	Bridge(Hue::Device::Enumerator& devices) : Basic1Template(), devices(devices)
	{
	}

	/**
	 * @brief Perform a configuration action
	 * @param config The action to perform
	 */
	void configure(const Config& config);

	/**
	 * @brief Enable creation of new users
	 * @note DO NOT leave this permanently enabled!
	 *
	 * This could be enabled via web page on local Access Point, or physical push-button.
	 * It should also be time limited, so exits pairing mode after maybe 30 seconds.
	 * If a user creation request is received then this is disabled automatically.
	 */
	void enablePairing(bool enable)
	{
		pairingEnabled = enable;
	}

	void onConfigChange(ConfigDelegate delegate)
	{
		configDelegate = delegate;
	}

	void onStateChanged(StateChangeDelegate delegate)
	{
		stateChangeDelegate = delegate;
	}

	void begin();

	/**
	 * @brief Get bridge statistics
	 * @retval const Stats&
	 */
	const Stats& getStats()
	{
		return stats;
	}

	/**
	 * @brief Clear the bridge statistics
	 */
	void resetStats()
	{
		memset(&stats, 0, sizeof(stats));
	}

	/**
	 * @brief Access the list of users
	 * @retval const UserMap&
	 */
	const UserMap& getUsers() const
	{
		return users;
	}

	/**
	 * @brief Get bridge status information in JSON format
	 * @param json Where to write information
	 */
	void getStatusInfo(JsonObject json);

	/**
	 * @brief Devices call this method when their state has been updated
	 * @note Applications should not call this method
	 */
	void deviceStateChanged(const Hue::Device& device, Hue::Device::Attributes changed)
	{
		if(stateChangeDelegate) {
			stateChangeDelegate(device, changed);
		}
	}

	/* UPnP::Device */

	String getField(Field desc) const override;

	bool formatMessage(SSDP::Message& msg, SSDP::MessageSpec& ms) override;
	bool onHttpRequest(HttpServerConnection& connection) override;

private:
	void createUser(JsonObjectConst request, JsonDocument& result, const String& path);
	bool validateUser(const char* userName);
	void handleApiRequest(HttpServerConnection& connection);

private:
	UserMap users;
	bool pairingEnabled = false;
	Hue::Device::Enumerator& devices;
	ConfigDelegate configDelegate;
	StateChangeDelegate stateChangeDelegate;
	Stats stats;
};

} // namespace Hue
