/**
 * Device.cpp
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

#include "include/Hue/Device.h"
#include <Platform/Station.h>

namespace Hue
{
// Strings used in this file
#define LOCALSTR(str) DEFINE_FSTR_LOCAL(FS_##str, #str);
LOCALSTR(address);
LOCALSTR(alert);
LOCALSTR(colormode);
LOCALSTR(description);
LOCALSTR(effect);
LOCALSTR(error);
LOCALSTR(homeautomation);
LOCALSTR(LCT007);
LOCALSTR(LWB001);
LOCALSTR(manufacturername);
LOCALSTR(mode);
LOCALSTR(modelid);
LOCALSTR(name);
LOCALSTR(none);
LOCALSTR(Philips);
LOCALSTR(reachable);
LOCALSTR(state);
LOCALSTR(swversion);
LOCALSTR(type);
LOCALSTR(uniqueid);
DEFINE_FSTR_LOCAL(FS_VERSION, "1.0.0");
DEFINE_FSTR_LOCAL(FS_extendedColorLight, "Extended color light");
DEFINE_FSTR_LOCAL(FS_onOffLight, "On/off light");

#define XX(t) #t "\0"
DEFINE_FSTR_LOCAL(fstrAttrTags, HUE_DEVICE_ATTR_MAP(XX));
#undef XX

#define XX(t) #t "\0"
DEFINE_FSTR_LOCAL(fstrColormodeTags, HUE_COLORMODE_MAP(XX));
#undef XX

String getErrorDesc(Error error)
{
	switch(error) {
#define XX(code, tag, desc)                                                                                            \
	case Error::tag:                                                                                                   \
		return F(desc);
		HUE_ERROR_CODE_MAP(XX)
#undef XX
	default:
		return nullptr;
	}
}

static JsonObject createResultObject(JsonDocument& result, const String& name)
{
	if(result.isNull()) {
		result.to<JsonArray>();
	}

	if(result.is<JsonArray>()) {
		auto obj = result.createNestedObject();
		return obj.createNestedObject(name);
	} else {
		return result.createNestedObject(name);
	}
}

JsonObject createSuccess(JsonDocument& result)
{
	return createResultObject(result, _F("success"));
}

JsonObject createError(JsonDocument& result, const String& path, Error error, String description)
{
	auto err = createResultObject(result, FS_error);
	err[FS_type] = int(error);
	err[FS_address] = path;
	if(!description) {
		description = getErrorDesc(error);
	}
	err[FS_description] = description;

	debug_i("[HUE] ERROR: %s", description.c_str());

	return err;
}

/* Device */

String toString(Device::Attribute attr)
{
	return CStringArray(fstrAttrTags)[unsigned(attr)];
}

String toString(Device::Attributes attr)
{
	String s;
	CStringArray csa(fstrAttrTags);
	for(auto it = csa.begin(); it != csa.end(); it++) {
		auto a = Device::Attribute(it.index());
		if(attr[a]) {
			if(s) {
				s += ',';
			}
			s += *it;
		}
	}

	return s;
}

bool fromString(const char* tag, Device::Attribute& attr)
{
	int i = CStringArray(fstrAttrTags).indexOf(tag);
	if(i < 0) {
		return false;
	}
	attr = Device::Attribute(i);
	return true;
}

String toString(Device::ColorMode mode)
{
	return CStringArray(fstrColormodeTags)[unsigned(mode)];
}

String Device::getUniqueId() const
{
	String s;
	s.reserve(26);
	s += WifiStation.getMacAddress().toString();
	// Append lower 24-bits of ID
	auto id = getId();
	s += ':';
	s += hexchar((id >> 20) & 0x0f);
	s += hexchar((id >> 16) & 0x0f);
	s += ':';
	s += hexchar((id >> 12) & 0x0f);
	s += hexchar((id >> 8) & 0x0f);
	s += '-';
	s += hexchar((id >> 4) & 0x0f);
	s += hexchar(id & 0x0f);
	return s;
}

void Device::getInfo(JsonObject json)
{
	bool isColour = false;

	CStringArray attrTags(fstrAttrTags);

	auto getAttr = [&](JsonObject obj, Device::Attribute attr) {
		unsigned value = 0;
		if(!getAttribute(attr, value)) {
			return;
		}

		String tag = attrTags[unsigned(attr)];
		if(attr == Attribute::on) {
			obj[tag] = (value != 0);
		} else {
			obj[tag] = value;
			isColour = true;
		}
	};

	JsonObject state = json.createNestedObject("state");
	getAttr(state, Attribute::on);
	state[FS_alert] = FS_none;
	state[FS_effect] = FS_none;
	state[FS_mode] = FS_homeautomation;

	getAttr(state, Attribute::bri);
	getAttr(state, Attribute::ct);
	getAttr(state, Attribute::hue);
	getAttr(state, Attribute::sat);

	auto colormode = getColorMode();
	if(colormode != ColorMode::none) {
		state[FS_colormode] = toString(colormode);
	}

	state[FS_reachable] = true;
	json[FS_uniqueid] = getUniqueId();
	json[FS_name] = getName();
	json[FS_manufacturername] = FS_Philips;
	if(isColour) {
		json[FS_type] = FS_extendedColorLight;
		json[FS_modelid] = FS_LCT007;
	} else {
		json[FS_type] = FS_onOffLight;
		json[FS_modelid] = FS_LWB001;
	}
	json[FS_swversion] = FS_VERSION;
}

/* DeviceEnumerator */

Device* DeviceEnumerator::find(Device::ID id)
{
	reset();
	Device* device;
	while((device = next()) != nullptr) {
		if(*device == id) {
			return device;
		}
	}

	return nullptr;
}

Device* DeviceEnumerator::find(const String& name)
{
	reset();
	Device* device;
	while((device = next()) != nullptr) {
		if(*device == name) {
			return device;
		}
	}

	return nullptr;
}

} // namespace Hue
