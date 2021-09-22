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

#ifndef POKEMINI_INUI
#define POKEMINI_INUI

#include <stdint.h>

#define UI_STATUS_MENU	1
#define UI_STATUS_GAME	0
#define UI_STATUS_EXIT	-1

// File list cache
typedef struct {
	char name[PMTMPV]; // Filename					| Message
	char stats;	// 0 = Invalid, 1 = Directory, 2 = File		| Unused
	char color;	// 0 = Normal, 1 = Color available, 2 = Package	| 0 = Yellow, 1 = Aqua
} TUIMenu_FileListCache;

// Maximum files/directories per directory
#ifndef UI_MAXCACHE
#define UI_MAXCACHE	512
#endif

// Menu callback reason
enum {
	UIMENU_LOAD,	// Menu was loaded (called from UIMenu_LoadItems)
	UIMENU_CANCEL,	// User pressed B
	UIMENU_OK,	// User pressed A
	UIMENU_LEFT,	// User pressed Left
	UIMENU_RIGHT	// User pressed Right
};

// Menu callback type
typedef int (*TUIMenu_Callback)(int index, int reason);

// Menu item
typedef struct TUIMenu_Item {
	int code;			// Code: 0 = Yellow, 1 = Aqua, 9 = End-of-list
	int index;			// Index of item
	char caption[32];		// Text to display, last entry will be the title
	TUIMenu_Callback callback;	// Callback, last entry will receive UIMENU_LOAD
	struct TUIMenu_Item *prev;	// Must be NULL
} TUIMenu_Item;

// Load items list
void UIMenu_LoadItems(TUIMenu_Item *items, int cursorindex);

// Return to previous menu
void UIMenu_PrevMenu(void);

// Change item on current menu
int UIMenu_ChangeItem(TUIMenu_Item *items, int index, const char *format, ...);

// Message output
void UIMenu_BeginMessage(void);
void UIMenu_SetMessage(char *message, int color);
void UIMenu_EndMessage(int timeout);

// Real-time text output
typedef int (*TUIRealtimeCB)(int line, char *outtext);
void UIMenu_RealTimeMessage(TUIRealtimeCB cb);

// Handle key events
void UIMenu_KeyEvent(int key, int press);

#endif
