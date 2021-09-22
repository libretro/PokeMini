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

#include "PokeMini.h"
#include "CommandLine.h"

#include "Joystick.h"

// Process joystick buttons packed in bits
void JoystickBitsEvent(uint32_t pressbits)
{
	static uint32_t lastpressbits;
	uint32_t maskbit, togglebits = pressbits ^ lastpressbits;
	int index, joybutton;

	for (index=0; index<10; index++) {
		joybutton = CommandLine.joybutton[index];
		if (joybutton >= 0) {
			maskbit = (1 << joybutton);
			if (togglebits & maskbit) {
				if (index) {
					UIMenu_KeyEvent(index, (pressbits & maskbit) ? 1 : 0);
				} else {
					if (pressbits & maskbit) UI_Status = !UI_Status;
				}
			}
		}
	}

	lastpressbits = pressbits;
}

// Process joystick buttons
void JoystickButtonsEvent(int button, int pressed)
{
	int index;

	for (index=0; index<10; index++) {
		if (CommandLine.joybutton[index] == button) {
			if (index) {
				UIMenu_KeyEvent(index, pressed);
			} else {
				if (pressed) UI_Status = !UI_Status;
			}
		}
	}
}
