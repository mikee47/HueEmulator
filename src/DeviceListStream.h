/****
 * DeviceListStream.h - Support for streaming Hue device information in JSON format
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

#include <Data/Stream/DataSourceStream.h>
#include "include/Hue/Device.h"

namespace Hue
{
/**
 * @brief A forward-only stream for listing contents of a tune file
 * @note Tune files can be large so we only output one tune title at a time
 */
class DeviceListStream : public IDataSourceStream
{
public:
	DeviceListStream(Device::Enumerator* devices) : devices(devices)
	{
	}

	~DeviceListStream()
	{
		delete devices;
	}

	bool isValid() const override
	{
		return true;
	}

	uint16_t readMemoryBlock(char* data, int bufSize) override;

	bool seek(int len) override;

	bool isFinished() override
	{
		return state >= 3;
	}

	String getName() const override;

protected:
	bool getContent();
	void begin();

private:
	Device::Enumerator* devices;
	String content;
	uint8_t state = 0;
	unsigned readPos = 0;
};

} // namespace Hue
