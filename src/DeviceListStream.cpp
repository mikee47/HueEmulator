/**
 * DeviceListStream.cpp - Support for streaming Hue device information in JSON format
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

#include "DeviceListStream.h"

namespace Hue
{
bool DeviceListStream::getContent()
{
	if(state == 0) {
		devices->reset();
		content = nullptr;
	} else {
		content = ',';
	}

	auto device = devices->next();
	if(device == nullptr) {
		return false;
	}

	StaticJsonDocument<2048> doc;
	device->getInfo(doc.to<JsonObject>());
	content += '"';
	content += device->getId();
	content += "\":";
	content += Json::serialize(doc);
	return true;
}

uint16_t DeviceListStream::readMemoryBlock(char* data, int bufSize)
{
	if(bufSize <= 0) {
		return 0;
	}

	switch(state) {
	case 0: {
		*data = '{';
		return 1;
	}

	case 1: {
		auto len = std::min(size_t(bufSize), content.length() - readPos);
		memcpy(data, &content[readPos], len);
		return len;
	}

	case 2: {
		*data = '}';
		return 1;
	}

	default:
		return 0;
	}
}

bool DeviceListStream::seek(int len)
{
	if(len <= 0) {
		return false;
	}

	switch(state) {
	case 0:
		if(len != 1) {
			return false;
		}
		if(getContent()) {
			state = 1;
		} else {
			// No devices, finish now
			state = 2;
		}
		return true;

	case 1: {
		auto newPos = readPos + len;
		if(newPos > content.length()) {
			debug_e("[HUE] seek(%d) out of range, max %u", len, content.length());
			return false;
		}
		if(newPos < content.length()) {
			readPos = newPos;
			return true;
		}

		content = nullptr;
		readPos = 0;

		if(!getContent()) {
			++state;
		}
		return true;
	}

	case 2:
		if(len != 1) {
			return false;
		}
		++state;
		return true;

	default:
		return false;
	}
}

String DeviceListStream::getName() const
{
	return _F("devices.json");
}

} // namespace Hue
