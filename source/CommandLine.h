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

#ifndef COMMAND_LINE
#define COMMAND_LINE

#include <stdio.h>
#include <stdint.h>

typedef struct {
	int forcefreebios;
	char min_file[PMTMPV];
	char bios_file[PMTMPV];
	char eeprom_file[PMTMPV];
	char state_file[PMTMPV];
	char rom_dir[PMTMPV];
	int updatertc;
	int eeprom_share;
	int sound;
	int piezofilter;
	int lcdfilter;
	int lcdmode;
	int low_battery;
	int palette;
	int rumblelvl;
	int joyenabled;
	int joyid;
	int joyaxis_dpad;
	int joyhats_dpad;
	char joyplatform[32];
	int joybutton[10];
	int multicart;
	int synccycles;
	int keyb_a[10];
	int keyb_b[10];
	uint32_t custompal[4];
	int lcdcontrast;
	int lcdbright;
} TCommandLine;

// Extern command line structure
extern TCommandLine CommandLine;

// Process arguments parsing into the command line structure
void CommandLineInit(void);

#endif
