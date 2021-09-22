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
#include <ctype.h>

#include <compat/posix_string.h>

// Return true if the string is valid and non-empty
int StringIsSet(char *str)
{
	if (!str) return 0;
	if (strlen(str) == 0) return 0;
	return 1;
}

// Get multiple of 2
int GetMultiple2(int input)
{
	if (!input) return 0;
	input--;
	input |= (input >> 1);
	input |= (input >> 2);
	input |= (input >> 4);
	input |= (input >> 8);
	input |= (input >> 16);
	return input+1;
}

// Get multiple of 2 (Mask)
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
	FILE *fi;

	// Test open file
	fi = fopen(filename, "rb");
	if (fi == NULL) return 0;
	fclose(fi);

	return 1;
}

// Get filename
char *GetFilename(const char *filename)
{
	int i;
	for (i = strlen(filename)-1; i >= 0; i--) {
		if ((filename[i] == '/') || (filename[i] == '\\')) {
			return (char *)&filename[i+1];
		}
	}
	return (char *)filename;
}

// Get extension
char *GetExtension(const char *filename)
{
	int i;
	for (i = strlen(filename)-1; i >= 0; i--) {
		if (filename[i] == '.') {
			return (char *)&filename[i];
		}
		if ((filename[i] == '/') || (filename[i] == '\\')) break;
	}
	return (char *)&filename[strlen(filename)-1];
}

// Extract path
char *ExtractPath(char *filename, int slash)
{
	int i;
	for (i = strlen(filename)-1; i >= 0; i--) {
		if ((filename[i] == '/') || (filename[i] == '\\')) {
			if (slash) filename[i+1] = 0;
			else filename[i] = 0;
			return filename;
		}
	}
	if (slash && !strlen(filename)) {
		filename[0] = '.';
		filename[1] = '/';
		filename[2] = 0;
	}
	return filename;
}

// Remove extension
char *RemoveExtension(char *filename)
{
	int i;
	for (i = strlen(filename)-1; i >= 0; i--) {
		if (filename[i] == '.') {
			filename[i] = 0;
			return filename;
		}
		if ((filename[i] == '/') || (filename[i] == '\\')) {
			return filename;
		}
	}
	return filename;
}

// Check if filename has certain extension
int ExtensionCheck(const char *filename, const char *ext)
{
	int i;
	for (i = strlen(filename)-1; i >= 0; i--) {
		if (filename[i] == '.') {
			if (strcasecmp(&filename[i], ext) == 0) {
				return 1;
			}
		}
		if ((filename[i] == '/') || (filename[i] == '\\')) {
			return 0;
		}
	}
	return 0;
}

// true if the path ends with a slash
int HasLastSlash(char *path)
{
	int len = strlen(path);
	if (!len) return 0;
	return (path[len-1] == '/') || (path[len-1] == '\\');
}

// Convert slashes to windows type
void ConvertSlashes(char *filename, int slashtype)
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

void PokeDPrint(int pokemsg, char *format, ...)
{
	char buffer[PMTMPV];
	va_list args;
	va_start(args, format);
	vsprintf(buffer, format, args);
	switch (pokemsg) {
	case POKEMSG_OUT:
		printf("%s", buffer);
		break;
	case POKEMSG_ERR:
		fprintf(stderr, "%s", buffer);
		break;
	}
	va_end(args);
}

#ifndef NO_DIRS

#ifdef _MSC_VER
#include <direct.h>
#else
#include <unistd.h>
#endif

char PokeMini_ExecDir[PMTMPV];	// Executable directory
char PokeMini_CurrDir[PMTMPV];	// Current directory

// Get current directory and save on parameter
void PokeMini_GetCustomDir(char *dir, int max)
{
	if (!getcwd(dir, max)) {
		strcpy(dir, "/");
		PokeDPrint(POKEMSG_ERR, "getcwd() error\n");
	}
	if (!strlen(dir)) strcpy(dir, "/");
	else ConvertSlashes(dir, PATH_SLASH_UNIX);
}

// Go to custom directory
void PokeMini_GotoCustomDir(const char *dir)
{
	char buffer[PMTMPV];
	strcpy(buffer, dir);
	ConvertSlashes(buffer, PATH_SLASH_OS);
	if (chdir(buffer)) {
		PokeDPrint(POKEMSG_ERR, "abs chdir('%s') error\n", buffer);
	}
}

// Get current directory
void PokeMini_GetCurrentDir(void)
{
	PokeMini_GetCustomDir(PokeMini_CurrDir, PMTMPV);
}

// Set current directory
void PokeMini_SetCurrentDir(const char *dir)
{
	PokeMini_GotoCustomDir(dir);
	PokeMini_GetCurrentDir();
}

// Go to current directory
void PokeMini_GotoCurrentDir(void)
{
	PokeMini_GotoCustomDir(PokeMini_CurrDir);
}

// Go to launch directory
void PokeMini_GotoExecDir(void)
{
	PokeMini_GotoCustomDir(PokeMini_ExecDir);
}

#else

void PokeMini_GetCurrentDir(void) {}
void PokeMini_GotoCurrentDir(void) {}
void PokeMini_GotoExecDir(void) {}

#endif
