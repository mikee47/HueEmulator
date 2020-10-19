/**
 * Strings.h
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
#include <FlashString/String.hpp>

#define HUE_STRING_MAP(XX)                                                                                             \
	XX(req)                                                                                                            \
	XX(count)                                                                                                          \
	XX(root)                                                                                                           \
	XX(getAllDev)                                                                                                      \
	XX(getDev)                                                                                                         \
	XX(setDev)                                                                                                         \
	XX(resp)                                                                                                           \
	XX(size)                                                                                                           \
	XX(err)                                                                                                            \
	XX(res)                                                                                                            \
	XX(meth)                                                                                                           \
	XX(users)                                                                                                          \
	XX(user)                                                                                                           \
	XX(devicetype)                                                                                                     \
	XX(auth)                                                                                                           \
	XX(address)                                                                                                        \
	XX(alert)                                                                                                          \
	XX(colormode)                                                                                                      \
	XX(description)                                                                                                    \
	XX(effect)                                                                                                         \
	XX(error)                                                                                                          \
	XX(homeautomation)                                                                                                 \
	XX(LCT007)                                                                                                         \
	XX(LWB001)                                                                                                         \
	XX(manufacturername)                                                                                               \
	XX(mode)                                                                                                           \
	XX(modelid)                                                                                                        \
	XX(name)                                                                                                           \
	XX(none)                                                                                                           \
	XX(Philips)                                                                                                        \
	XX(reachable)                                                                                                      \
	XX(state)                                                                                                          \
	XX(swversion)                                                                                                      \
	XX(type)                                                                                                           \
	XX(uniqueid)

#define HUE_STRING_MAP2(XX)                                                                                            \
	XX(VERSION, "1.0.0")                                                                                               \
	XX(extendedColorLight, "Extended color light")                                                                     \
	XX(onOffLight, "On/off light")

namespace Hue
{
#define XX(s) DECLARE_FSTR(FS_##s)
HUE_STRING_MAP(XX)
#undef XX

#define XX(n, s) DECLARE_FSTR(FS_##n)
HUE_STRING_MAP2(XX)
#undef XX

} // namespace Hue
