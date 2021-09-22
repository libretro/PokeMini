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
#include "Missing.h"
#include "PokeMini.h"
#include <ctype.h>

#include <compat/posix_string.h>

/* Return true if the string is valid and non-empty */
int StringIsSet(char *str)
{
	if (!str) return 0;
	if (strlen(str) == 0) return 0;
	return 1;
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

// Check if file exist
int FileExist(const char *filename)
{
	// Test open file
	FILE *fi = fopen(filename, "rb");
	if (fi == NULL) return 0;
	fclose(fi);

	return 1;
}
