/**
 * OnOffDevice.h
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

namespace Hue
{
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

	bool getAttribute(Attribute attr, unsigned& value) const override
	{
		switch(attr) {
		case Attribute::on:
			value = on;
			return true;
		default:
			return false;
		}
	}

	Status setAttribute(Attribute attr, unsigned value, Callback callback) override
	{
		switch(attr) {
		case Attribute::on:
			this->on = value;
			return Status::success;
		default:
			return Status::error;
		}
	}

private:
	ID id;
	String name;
	bool on{false};
};

} // namespace Hue
