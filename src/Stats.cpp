/**
 * Stats.cpp
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

#include "include/Hue/Stats.h"
#include "Strings.h"

namespace Hue
{
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

} // namespace Hue
