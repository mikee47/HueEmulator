/**
 * Bridge.cpp
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

#include "include/Hue/Bridge.h"
#include "DeviceListStream.h"
#include <Platform/Station.h>
#include "ResponseStream.h"
#include <ArduinoJson.h>
#include <Data/CStringArray.h>
#include <Data/HexString.h>

#define LOCALSTR(s) DEFINE_FSTR_LOCAL(FS_##s, #s)
LOCALSTR(req);
LOCALSTR(count);
LOCALSTR(root);
LOCALSTR(getAllDev);
LOCALSTR(getDev);
LOCALSTR(setDev);
LOCALSTR(resp);
LOCALSTR(size);
LOCALSTR(err);
LOCALSTR(res);
LOCALSTR(meth);
LOCALSTR(users);
LOCALSTR(user);
LOCALSTR(devicetype);
LOCALSTR(auth);

namespace Hue
{
/* Stats */

void Stats::serialize(JsonObject json) const
{
	auto req = json.createNestedObject(FS_req);
	req[FS_count] = request.count;
	req[FS_root] = request.root;
	req[FS_getAllDev] = request.getAllDeviceInfo;
	req[FS_getDev] = request.getDeviceInfo;
	req[FS_setDev] = request.setDeviceInfo;
	auto resp = json.createNestedObject(FS_resp);
	resp[FS_count] = response.count;
	resp[FS_size] = response.size;
	auto err = json.createNestedObject(FS_err);
	err[FS_count] = error.count;
	err[FS_res] = error.resourceNotAvailable;
	err[FS_meth] = error.methodNotAvailable;
	err[FS_user] = error.unauthorizedUser;
}

/* Bridge */

void Bridge::getStatusInfo(JsonObject json)
{
	stats.serialize(json);
	auto jusers = json.createNestedObject(FS_users);
	for(unsigned i = 0; i < users.count(); ++i) {
		auto& username = users.keyAt(i);
		auto& info = users.valueAt(i);
		auto juser = jusers.createNestedObject(username);
		juser[FS_devicetype] = info.deviceType;
		juser[FS_auth] = info.authorized;
		juser[FS_count] = info.count;
	}
}

String Bridge::getField(Field desc)
{
	switch(desc) {
	case Field::friendlyName: {
		String s = F("Philips hue (");
		s += WifiStation.getIP().toString();
		s += ')';
		return s;
	}
	case Field::manufacturer:
		return F("Royal Philips Electronics");
	case Field::manufacturerURL:
		return F("http://www.philips.com");
	case Field::modelDescription:
		return F("Sming Hue Emulator");
	case Field::modelName:
		return F("Philips hue bridge 2012");
	case Field::modelNumber:
		return F("929000226503");
	case Field::modelURL:
		return F("http://www.meethue.com");
	case Field::serialNumber:
		return WifiStation.getMAC();
	case Field::UDN:
		return F("uuid:2f402f80-da50-11e1-9b23-") + getField(Field::serialNumber);
	case Field::presentationURL:
		return F("index.html");
	case Field::serverId:
		return RootDevice::getField(desc) + _F(" IpBridge/1.17.0");
	default:
		return RootDevice::getField(desc);
	}
}

bool Bridge::formatMessage(Message& msg, MessageSpec& ms)
{
	msg[F("hue-bridgeid")] = WifiStation.getMAC();
	return RootDevice::formatMessage(msg, ms);
}

void Bridge::configure(const Config& config)
{
	switch(config.type) {
	case Config::Type::AuthorizeUser: {
		auto& user = users[config.name];
		user.deviceType = config.deviceType;
		user.authorized = true;
		debug_i("[HUE] Created user, devicetype = '%s', name = '%s'", user.deviceType.c_str(), config.name.c_str());
		break;
	}

	case Config::Type::RevokeUser: {
		auto& user = users[config.name];
		user.authorized = false;
		debug_i("[HUE] Revoke user, devicetype = '%s', name = '%s'", user.deviceType.c_str(), config.name.c_str());
		break;
	}
	}
}

void Bridge::createUser(JsonObjectConst request, JsonDocument& result, const String& requestPath)
{
	if(!pairingEnabled) {
		createError(result, requestPath, Error::LinkButtonNotPressed, nullptr);
		return;
	}

	Config cfg;
	cfg.type = Config::Type::AuthorizeUser;
	cfg.deviceType = request[_F("devicetype")].as<const char*>();

	uint8_t buf[16];
	os_get_random(buf, sizeof(buf));
	cfg.name = makeHexString(buf, sizeof(buf));

	configure(cfg);

	if(configDelegate) {
		configDelegate(cfg);
	} else {
		debug_e("[HUE] Config delegate not set!");
	}

	auto obj = createSuccess(result);
	obj[_F("username")] = cfg.name;
}

bool Bridge::validateUser(const char* userName)
{
	// If user doesn't exist, will create a default un-authorized entry
	auto& user = users[userName];
	++user.count;

	if(user.authorized) {
		return true;
	}

	if(!pairingEnabled) {
		return false;
	}

	debug_i("In pairing mode, storing provided username '%s'", userName);
	Config config;
	config.type = Config::Type::AuthorizeUser;
	config.deviceType = F("Default");
	config.name = userName;
	configure(config);
	if(configDelegate) {
		configDelegate(config);
	}

	return true;
}

/*
 * GET /api/<username>/lights
 * 	Get all lights
 *
 * GET /api/<username>/lights/new
 * 	Get new lights
 *
 * POST /api/<username>/lights
 * 	Search for new lights
 *
 * GET /api/<username>/lights/<id>
 * 	Get light attributes and state
 *
 * PUT /api/<username>/lights/<id>
 * 	Set light attributes (rename)
 *
 * POST /api/<username>/lights/<id>/state
 * 	Set light state
 *
 * DELETE /api/<username>/lights/<id>
 * 	Delete lights
 *
 * POST /api
 * 	Create user.
 * 	TODO: Discovery mode must be enabled via some other technique, e.g. button press
 * 	Generate unique user ID and store to flash.
 * 	DO NOT leave enabled permanently.
 *
 * GET /api/<username>/config
 *  Get configuration
 *
 * PUT /api/<username>/config
 *  Modify configuration
 *
 * DELETE /api/<application_key>/config/whitelist/<element>
 * 	Delete user from whitelist
 *
 * GET /api/<username>
 * 	Get full state (datastore)
 *
 *
 */
bool Bridge::onHttpRequest(HttpServerConnection& connection)
{
	++stats.request.count;

	if(RootDevice::onHttpRequest(connection)) {
		++stats.request.root;
		return true;
	}

	auto request = connection.getRequest();
	if(!request->uri.Path.startsWith("/api")) {
		++stats.request.ignored;
		return false;
	}

	handleApiRequest(connection);

	return true;
}

void Bridge::handleApiRequest(HttpServerConnection& connection)
{
	auto& request = *connection.getRequest();
	String requestPath = request.uri.getRelativePath();
	debug_i("[HUE] Request: %s %s", http_method_str(request.method), requestPath.c_str());

	auto badRequest = [&]() -> void {
		connection.getResponse()->code = HTTP_STATUS_BAD_REQUEST;
		++stats.error.count;
	};

	auto sendStream = [&](IDataSourceStream* stream, size_t len) -> void {
		connection.getResponse()->sendDataStream(stream, MIME_JSON);
		++stats.response.count;
		stats.response.size += len;
	};

	auto body = request.getBodyStream();
	int avail = 0;
	if(body != nullptr) {
		avail = body->available();
		if(avail < 0 || body->getStreamType() != eSST_Memory) {
			debug_e("[HUE] Invalid request body");
			return badRequest();
		}
	}

	requestPath.replace('/', '\0');
	CStringArray segments(requestPath);

	assert(F("api") == segments[0]);

	// Reassemble path suitable for including in error responses
	requestPath = '/';
	for(unsigned i = 2; i < segments.count(); ++i) {
		if(i > 2) {
			requestPath += '/';
		}
		requestPath += segments[i];
	}

	StaticJsonDocument<128> requestDoc;
	if(avail > 0) {
		debug_d("[HUE] Body: %d bytes", avail);
		auto mem = reinterpret_cast<MemoryDataStream*>(body);
		auto data = const_cast<char*>(mem->getStreamPointer());
#if DEBUG_VERBOSE_LEVEL == INFO
		m_nputs(data, avail);
		m_putc('\n');
#endif
		Json::deserialize(requestDoc, data, avail);
	}

	StaticJsonDocument<2048> resultDoc;

	auto sendResult = [&]() {
		auto stream = new MemoryDataStream;
		auto len = Json::serialize(resultDoc, stream);
		return sendStream(stream, len);
	};

	auto resourceNotAvailable = [&]() {
		++stats.error.resourceNotAvailable;
		String s = getErrorDesc(Error::ResourceNotAvailable);
		s.replace(F("<resource>"), requestPath);
		createError(resultDoc, requestPath, Error::ResourceNotAvailable, s);
		return sendResult();
	};

	auto methodNotAvailable = [&]() {
		++stats.error.methodNotAvailable;
		String s = getErrorDesc(Error::MethodNotAvailable);
		s.replace(F("<method_name>"), http_method_str(request.method));
		s.replace(F("<resource>"), requestPath);
		createError(resultDoc, requestPath, Error::MethodNotAvailable, s);
		return sendResult();
	};

	if(segments.count() == 1) {
		if(request.method != HTTP_POST) {
			return methodNotAvailable();
		}

		createUser(requestDoc.as<JsonObject>(), resultDoc, requestPath);
		return sendResult();
	}

	const char* userName = segments[1];
	if(!validateUser(userName)) {
		++stats.error.unauthorizedUser;
		createError(resultDoc, requestPath, Error::UnauthorizedUser, nullptr);
		return sendResult();
	}

	const char* apiName = segments[2];
	if(F("lights") != apiName) {
		return resourceNotAvailable();
	}

	int id = String(segments[3]).toInt();
	if(request.method == HTTP_GET) {
		// "/api/<username>/lights/<id>"
		debug_i("[HUE] Get lights (%d)", id);
		if(id <= 0) {
			// All devices
			++stats.request.getAllDeviceInfo;
			auto stream = new DeviceListStream(devices.clone());
			return sendStream(stream, 0);
		}

		// Client is requesting a single device
		auto json = resultDoc.to<JsonObject>();
		auto device = devices.find(id);
		if(device == nullptr) {
			return resourceNotAvailable();
		}

		++stats.request.getDeviceInfo;
		device->getInfo(json);
		return sendResult();
	}

	if(request.method != HTTP_POST) {
		return methodNotAvailable();
	}

	if(segments.count() < 4) {
		/*
		 * Search for new lights
		 *
		 * "/api/<username>/lights"
		 */
		debug_i("[HUE] Search for new lights");
		auto obj = createSuccess(resultDoc);
		obj["lights"] = _F("Searching for new devices");
		return sendResult();
	}

	if(F("state") == segments[4]) {
		/*
		 * Set device state
		 *
		 * "/api/<username>/lights/<id>/state"
		 */
		debug_i("[HUE] Set device state (%d)", id);
		auto device = devices.find(id);
		if(device == nullptr) {
			debug_e("[HUE] Invalid device ID: %u", id);
			return resourceNotAvailable();
		}

		++stats.request.setDeviceInfo;
		auto stream = new ResponseStream(connection, requestPath);
		stream->handleRequest(requestDoc, *device);
		return sendStream(stream, 0);
	}

	// No idea
	badRequest();
}

} // namespace Hue
