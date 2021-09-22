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

#include <compat/posix_string.h>

#include "PokeMini.h"
#include "Keyboard.h"

TCommandLine CommandLine;

void CommandLineInit(void)
{
	// Clear structure
	memset((void *)&CommandLine, 0, sizeof(TCommandLine));

	// Default strings
	CommandLine.min_file[0] = 0; 
	strcpy(CommandLine.bios_file, "bios.min");
	strcpy(CommandLine.eeprom_file, "PokeMini.eep");
	CommandLine.state_file[0] = 0;
	strcpy(CommandLine.joyplatform, "default");
	CommandLine.rom_dir[0] = 0;

	// Default booleans / integers
	CommandLine.forcefreebios = 0;	// Force FreeBIOS
	CommandLine.updatertc = 2;	// Update RTC (0=Off, 1=State, 2=Host)
	CommandLine.eeprom_share = 0;	// EEPROM Share
#ifdef PERFORMANCE
	CommandLine.sound = MINX_AUDIO_GENERATED;
	CommandLine.piezofilter = 0;	// Piezo Filter
#else
	CommandLine.sound = MINX_AUDIO_DIRECTPWM;
	CommandLine.piezofilter = 1;	// Piezo Filter
#endif
	CommandLine.lcdfilter = 1;	// LCD Filter
	CommandLine.lcdmode = 0;	// LCD Mode
	CommandLine.low_battery = 0;	// Low Battery
	CommandLine.palette = 0;	// Palette Index
	CommandLine.rumblelvl = 3;	// Rumble level
	CommandLine.joyid = 0;		// Joystick ID
	CommandLine.joyaxis_dpad = 1;	// Joystick Axis as DPad
	CommandLine.joyhats_dpad = 1;	// Joystick Hats as DPad
	// Joystick mapping
	CommandLine.joybutton[0] = 8;	// Menu:  Button 8
	CommandLine.joybutton[1] = 1;	// A:     Button 1
	CommandLine.joybutton[2] = 2;	// B:     Button 2
	CommandLine.joybutton[3] = 7;	// C:     Button 7
	CommandLine.joybutton[4] = 10;	// Up:    Button 10
	CommandLine.joybutton[5] = 11;	// Down:  Button 11
	CommandLine.joybutton[6] = 4;	// Left:  Button 4
	CommandLine.joybutton[7] = 5;	// Right: Button 5
	CommandLine.joybutton[8] = 9;	// Power: Button 9
	CommandLine.joybutton[9] = 6;	// Shake: Button 6
	// Keyboard mapping (Magic numbers!)
	CommandLine.keyb_a[0] = PMKEYB_ESCAPE;	// Menu:  ESCAPE
	CommandLine.keyb_a[1] = PMKEYB_X;	// A:     X
	CommandLine.keyb_a[2] = PMKEYB_Z;	// B:     Z
	CommandLine.keyb_a[3] = PMKEYB_C;	// C:     C
	CommandLine.keyb_a[4] = PMKEYB_UP;	// Up:    UP
	CommandLine.keyb_a[5] = PMKEYB_DOWN;	// Down:  DOWN
	CommandLine.keyb_a[6] = PMKEYB_LEFT;	// Left:  LEFT
	CommandLine.keyb_a[7] = PMKEYB_RIGHT;	// Right: RIGHT
	CommandLine.keyb_a[8] = PMKEYB_E;	// Power: E
	CommandLine.keyb_a[9] = PMKEYB_A;	// Shake: A
	// Keyboard alternative mapping (Magic numbers!)
	CommandLine.keyb_b[0] = PMKEYB_Q;	// Menu:  Q
	CommandLine.keyb_b[1] = PMKEYB_NONE;	// A:     NONE
	CommandLine.keyb_b[2] = PMKEYB_NONE;	// B:     NONE
	CommandLine.keyb_b[3] = PMKEYB_D;	// C:     D
	CommandLine.keyb_b[4] = PMKEYB_KP_8;	// Up:    KP_8
	CommandLine.keyb_b[5] = PMKEYB_KP_2;	// Down:  KP_2
	CommandLine.keyb_b[6] = PMKEYB_KP_4;	// Left:  KP_4
	CommandLine.keyb_b[7] = PMKEYB_KP_6;	// Right: KP_6
	CommandLine.keyb_b[8] = PMKEYB_P;	// Power: P
	CommandLine.keyb_b[9] = PMKEYB_S;	// Shake: S
	CommandLine.custompal[0] = 0xFFFFFF;	// Custom Palette 1 Light
	CommandLine.custompal[1] = 0x000000;	// Custom Palette 1 Dark
	CommandLine.custompal[2] = 0xFFFFFF;	// Custom Palette 2 Light
	CommandLine.custompal[3] = 0x000000;	// Custom Palette 2 Dark
	CommandLine.lcdcontrast = 64;		// LCD contrast
	CommandLine.lcdbright = 0;		// LCD bright offset
	CommandLine.multicart = 0;	// Multicart support
#ifdef PERFORMANCE
	CommandLine.synccycles = 64;	// Sync cycles to 64 (Performance)
#else
	CommandLine.synccycles = 8;	// Sync cycles to 8 (Accurant)
#endif
}
