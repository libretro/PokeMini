/*
  PokeMini - Pokémon-Mini Emulator
  Copyright (C) 2009-2015  JustBurn

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "PMCommon.h"
#include "PokeMini.h"
#include <ctype.h>

#include <string/stdstring.h>
#include <file/file_path.h>

/* Return true if the string is valid and non-empty */
int StringIsSet(char *str)
{
	return string_is_empty(str) ? 0 : 1;
}

/* Get multiple of 2 (Mask) */
int GetMultiple2Mask(int input)
{
	if (input) input--;
	input |= (input >> 1);
	input |= (input >> 2);
	input |= (input >> 4);
	input |= (input >> 8);
	input |= (input >> 16);
	return input;
}

/* Check if file exists */
int FileExist(const char *filename)
{
	return (!string_is_empty(filename) && path_is_valid(filename)) ? 1 : 0;
}
