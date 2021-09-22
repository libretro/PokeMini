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

/* Convert slashes to windows type */
static void ConvertSlashes(char *filename, int slashtype)
{
	int i;
	for (i = strlen(filename)-1; i >= 0; i--) {
		if ((filename[i] == '/') || (filename[i] == '\\')) {
			if (slashtype == 0) filename[i] = PATH_SLASH_CHR;
			else if (slashtype == 1) filename[i] = '/';
			else if (slashtype == 2) filename[i] = '\\';
		}
	}
}

#ifndef NO_DIRS

#ifdef _MSC_VER
#include <direct.h>
#else
#include <unistd.h>
#endif

static char PokeMini_ExecDir[PMTMPV];	// Executable directory
static char PokeMini_CurrDir[PMTMPV];	// Current directory

// Get current directory and save on parameter
void PokeMini_GetCustomDir(char *dir, int max)
{
	if (!getcwd(dir, max))
	{
		strcpy(dir, "/");
		log_cb(RETRO_LOG_ERROR, "getcwd() error\n");
	}
	if (!strlen(dir))
		strcpy(dir, "/");
	else
		ConvertSlashes(dir, PATH_SLASH_UNIX);
}

// Go to custom directory
void PokeMini_GotoCustomDir(const char *dir)
{
	char buffer[PMTMPV];
	strcpy(buffer, dir);
	ConvertSlashes(buffer, PATH_SLASH_OS);
	if (chdir(buffer))
		log_cb(RETRO_LOG_ERROR, "abs chdir('%s') error\n", buffer);
}

// Go to launch directory
void PokeMini_GotoExecDir(void)
{
	PokeMini_GotoCustomDir(PokeMini_ExecDir);
}
#else
void PokeMini_GotoExecDir(void) {}
#endif
