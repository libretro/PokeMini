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

// Process joystick buttons
void JoystickButtonsEvent(int button, int pressed)
{
	int index;

        if (pressed)
        {
		for (index=0; index<10; index++) {
			if (CommandLine.joybutton[index] == button) {
				if (index)
					MinxIO_Keypad(index, 1);
			}
		}
        }
        else
	{
		for (index=0; index<10; index++) {
			if (CommandLine.joybutton[index] == button) {
				if (index)
					MinxIO_Keypad(index, 0);
			}
		}
	}
}
