/**
 * DeviceList.h
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
#include <WVector.h>

namespace Hue
{
using DeviceList = Vector<Device>;

class DeviceListEnumerator : public Device::Enumerator
{
public:
	DeviceListEnumerator(DeviceList& list) : list(list)
	{
	}

	Device::Enumerator* clone() override
	{
		return new DeviceListEnumerator(*this);
	}

	void reset() override
	{
		index = 0;
	}

	Device* current() override
	{
		return (size_t(index) < list.count()) ? &list[index] : nullptr;
	}

	Device* next() override
	{
		return (size_t(index) < list.count()) ? &list[index++] : nullptr;
	}

private:
	DeviceList& list;
	int index{-1};
};

} // namespace Hue
