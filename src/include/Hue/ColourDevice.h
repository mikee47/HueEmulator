/**
 * ColourDevice.h
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

#include "DimmableDevice.h"

namespace Hue
{
class ColourDevice : public DimmableDevice
{
public:
	ColourDevice(ID id, const String& name) : DimmableDevice(id, name)
	{
	}

	bool getAttribute(Attribute attr, unsigned& value) const override
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

	Status setAttribute(Attribute attr, unsigned value, Callback callback) override
	{
		switch(attr) {
		case Attribute::sat:
			sat = value;
			return Status::success;
		case Attribute::hue:
			hue = value;
			return Status::success;
		case Attribute::ct:
			ct = value;
			return Status::success;
		default:
			return DimmableDevice::setAttribute(attr, value, callback);
		}
	}

private:
	uint8_t sat = 0;
	uint16_t hue = 0;
	uint16_t ct = 234;
};

} // namespace Hue
