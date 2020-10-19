/**
 * Strings.cpp
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

#include "Strings.h"

namespace Hue
{
#define XX(s) DEFINE_FSTR(FS_##s, #s)
HUE_STRING_MAP(XX)
#undef XX

#define XX(n, s) DEFINE_FSTR(FS_##n, #s)
HUE_STRING_MAP2(XX)
#undef XX

} // namespace Hue