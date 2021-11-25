/*
  PokeMini - Pokémon-Mini Emulator
  Copyright (C) 2009-2012  JustBurn

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

#ifndef POKEMINI_VIDEO_X7
#define POKEMINI_VIDEO_X7

#include <stdint.h>

// Video specs
extern const TPokeMini_VideoSpec PokeMini_Video7x7;

// Return the best blitter
TPokeMini_DrawVideo32 PokeMini_GetVideo7x7_32(int filter, int lcdmode);
TPokeMini_DrawVideo16 PokeMini_GetVideo7x7_16(int filter, int lcdmode);

// Render to 672x448, analog + scanline
void PokeMini_VideoAScanLine7x7_32(uint32_t *screen, int pitchW);
void PokeMini_VideoAScanLine7x7_16(uint16_t *screen, int pitchW);

// Render to 672x448, 3-colors + scanline
void PokeMini_Video3ScanLine7x7_32(uint32_t *screen, int pitchW);
void PokeMini_Video3ScanLine7x7_16(uint16_t *screen, int pitchW);

// Render to 672x448, 2-colors + scanline
void PokeMini_Video2ScanLine7x7_32(uint32_t *screen, int pitchW);
void PokeMini_Video2ScanLine7x7_16(uint16_t *screen, int pitchW);

// Render to 672x448, analog + dot matrix
void PokeMini_VideoAMatrix7x7_32(uint32_t *screen, int pitchW);
void PokeMini_VideoAMatrix7x7_16(uint16_t *screen, int pitchW);

// Render to 672x448, 3-colors + dot matrix
void PokeMini_Video3Matrix7x7_32(uint32_t *screen, int pitchW);
void PokeMini_Video3Matrix7x7_16(uint16_t *screen, int pitchW);

// Render to 672x448, 2-colors + dot matrix
void PokeMini_Video2Matrix7x7_32(uint32_t *screen, int pitchW);
void PokeMini_Video2Matrix7x7_16(uint16_t *screen, int pitchW);

// Render to 672x448, analog
void PokeMini_VideoANone7x7_32(uint32_t *screen, int pitchW);
void PokeMini_VideoANone7x7_16(uint16_t *screen, int pitchW);

// Render to 672x448, 3-colors
void PokeMini_Video3None7x7_32(uint32_t *screen, int pitchW);
void PokeMini_Video3None7x7_16(uint16_t *screen, int pitchW);

// Render to 672x448, 2-colors
void PokeMini_Video2None7x7_32(uint32_t *screen, int pitchW);
void PokeMini_Video2None7x7_16(uint16_t *screen, int pitchW);

// Render to 672x448, unofficial colors
void PokeMini_VideoColor7x7_32(uint32_t *screen, int pitchW);
void PokeMini_VideoColor7x7_16(uint16_t *screen, int pitchW);
void PokeMini_VideoColorL7x7_32(uint32_t *screen, int pitchW);
void PokeMini_VideoColorL7x7_16(uint16_t *screen, int pitchW);
void PokeMini_VideoColorH7x7_32(uint32_t *screen, int pitchW);
void PokeMini_VideoColorH7x7_16(uint16_t *screen, int pitchW);

#endif
