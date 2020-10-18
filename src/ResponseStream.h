/**
 * ResponseStream.h
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

#include <Data/Stream/MemoryDataStream.h>
#include <Network/Http/HttpRequest.h>
#include "include/Hue/Bridge.h"
#include <WHashMap.h>

namespace Hue
{
/*
 * Handles a command and generates asynchronous response stream.
 * This is populated only when all IO requests have been completed.
 */
class ResponseStream : public MemoryDataStream
{
public:
	ResponseStream(Bridge& bridge, Device& device, HttpServerConnection& connection, String& path)
		: bridge(bridge), device(device), connection(connection), path(path)
	{
	}

	void handleRequest(JsonDocument& request);

	int available() override
	{
		// Wait until requests have been handled before sending response headers
		return (outstandingRequests == 0) ? MemoryDataStream::available() : 0;
	}

	bool isFinished() override
	{
		return (outstandingRequests == 0) && MemoryDataStream::isFinished();
	}

	uint16_t readMemoryBlock(char* data, int bufSize) override;

private:
	void generateResponse();

	Bridge& bridge;
	Device& device;
	HttpServerConnection& connection;
	String path;
	HashMap<String, String> results;
	uint8_t outstandingRequests{0};
	Device::Attributes changed;
};

} // namespace Hue
