/****
 * Stats.h - Maintain useful statistics for actions performed by a Hue Bridge
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

#include <stdint.h>
#include <ArduinoJson6.h>

namespace Hue
{
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

} // namespace Hue
