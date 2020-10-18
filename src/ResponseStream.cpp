/**
 * ResponseStream.cpp
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

#include "ResponseStream.h"

namespace Hue
{
void ResponseStream::handleRequest(JsonDocument& request)
{
	for(JsonPair pair : request.as<JsonObject>()) {
		Device::Attribute attr;
		const char* tag = pair.key().c_str();
		if(fromString(tag, attr)) {
			unsigned value;
			if(attr == Device::Attribute::on) {
				value = pair.value().as<bool>();
			} else {
				value = pair.value();
			}

			debug_i("[HUE] Set '%s' = %u", tag, value);

			auto callback = [this, attr](int errorCode) {
				--outstandingRequests;
				debug_i("ResponseStream::requestComplete, errorCode = %d, outstanding = %u", errorCode,
						outstandingRequests);

				if(errorCode < 0) {
					auto tag = toString(attr);
					results[tag] = nullptr;
				} else {
					changed[attr] = true;
				}

				if(outstandingRequests == 0) {
					generateResponse();
					connection.send();
				}
			};

			results[tag] = pair.value().as<String>();
			auto status = device.setAttribute(attr, value, callback);
			if(status == Status::pending) {
				++outstandingRequests;
				continue;
			}

			if(status == Status::success) {
				changed[attr] = true;
				continue;
			}
		}

		results[tag] = nullptr;
	}

	if(outstandingRequests == 0) {
		generateResponse();
	}
}

void ResponseStream::generateResponse()
{
	bridge.deviceStateChanged(device, changed);

	StaticJsonDocument<2048> doc;
	doc.to<JsonArray>();
	for(unsigned i = 0; i < results.count(); ++i) {
		auto tag = results.keyAt(i);
		auto value = results.valueAt(i);
		if(value) {
			auto obj = createSuccess(doc);
			obj[path + "/" + tag] = value;
		} else {
			String s = getErrorDesc(Error::InternalError);
			s.replace(F("<error_code>"), "-1");
			createError(doc, path, Error::InternalError, s);
		}
	}
	size_t len = Json::serialize(doc, *this);

	//		stats.response.size += len;
	debug_i("Serialized %d bytes", len);
}

uint16_t ResponseStream::readMemoryBlock(char* data, int bufSize)
{
	auto count = MemoryDataStream::readMemoryBlock(data, bufSize);
	debug_i("ResponseStream::read: %u", count);
	return count;
}

} // namespace Hue
