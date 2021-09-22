/*
  PokeMini - Pokémon-Mini Emulator
  Copyright (C) 2009-2014  JustBurn

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

#include <sys/types.h>
#include <ctype.h>

#ifndef NO_SCANDIRS
#include <sys/stat.h>
#ifdef _MSC_VER
#include <direct.h>
#else
#include <unistd.h>
#endif
#endif

#include <file/file_path.h>
#include <retro_dirent.h>

#include "PokeMini.h"
#include "Video.h"
#include "UI.h"
#include "CommandLine.h"
#include "PokeMini_Font12.h"
#include "PokeMini_Icons12.h"
#include "Missing.h"

#define UIMenu_FilesLines	(UIMenu_Lines-1)

int UIMenu_Width = 0;	// Menu width
int UIMenu_Height = 0;	// Menu height
int UIMenu_PixelLayout = 0;	// Menu pixel layout
int UIMenu_Lines = 0;	// Menu visible lines

int UIMenu_Page = 0;	// Mode
int UIMenu_MMax = 0;	// Menu max lines
int UIMenu_MOff = 0;	// Menu offset
int UIMenu_Cur = 0;	// Cursor offset
int UIMenu_Ani = 0;	// For animation effect
int UIMenu_InKey = 0;	// Key input
int UIMenu_CKeyMod  = 0;	// C Key modifier
int UIMenu_HardReset = 0;	// Hard reset
int UIMenu_Savestate = 0;	// Savestate offset

int UIMenu_CurrentItemsNum = 0;			// Number of current items
TUIMenu_Item *UIMenu_CurrentItems = NULL;	// Current items list
TUIMenu_FileListCache *UIMenu_FileListCache = NULL;	// Files list cache
int UIMenu_ListOffs = 0;	// Offset on list cache
int UIMenu_ListFiles = 0;	// Number of files in files list cache

#define UIMenu_MsgCountReset1	200
#define UIMenu_MsgCountReset2	80
int UIMenu_MsgOffset = 0;	// Message line offset (if lines exceed the screen)
int UIMenu_MsgCountDw = 0;	// Message count down (to move the offset)
int UIMenu_MsgTimer = 0;	// Message timer (to close the message)
int UIMenu_MsgLines = 0;	// Message lines (total number of lines)

static TUIRealtimeCB UIRealtimeCB = NULL;

enum {
	UIPAGE_MENUITEMS,
	UIPAGE_LOADROM,
	UIPAGE_MESSAGE,
	UIPAGE_REALTIMETEXT
};

void UIMenu_KeyEvent(int key, int press)
{
	if (key == MINX_KEY_C) UIMenu_CKeyMod = press;
	if (press)
		PokeMini_KeypadEvent(key, 1);
	else
		PokeMini_KeypadEvent(key, 0);
}

void UIMenu_LoadItems(TUIMenu_Item *items, int cursorindex)
{
	int i = 0;
	while (items[i].code < 2) i++;
	items[i].index = UIMenu_Cur;
	items[i].prev = UIMenu_CurrentItems;
	UIMenu_CurrentItems = items;
	UIMenu_CurrentItemsNum = i;
	UIMenu_Cur = cursorindex;
	if (UIMenu_Cur >= UIMenu_MMax) UIMenu_MOff = UIMenu_Cur - UIMenu_MMax + 1;
	else UIMenu_MOff = 0;
	items[i].callback(i, UIMENU_LOAD);
}

void UIMenu_PrevMenu(void)
{
	int i = 0;
	if (!UIMenu_CurrentItems) return;
	UIMenu_Cur = UIMenu_CurrentItems[UIMenu_CurrentItemsNum].index;
	UIMenu_CurrentItems = UIMenu_CurrentItems[UIMenu_CurrentItemsNum].prev;
	while (UIMenu_CurrentItems[i].code < 2) i++;
	UIMenu_CurrentItemsNum = i;
	if (UIMenu_Cur >= UIMenu_MMax) UIMenu_MOff = UIMenu_Cur - UIMenu_MMax + 1;
	else UIMenu_MOff = 0;
	UIMenu_CurrentItems[i].callback(i, UIMENU_LOAD);
}

int UIMenu_ChangeItem(TUIMenu_Item *items, int index, const char *format, ...)
{
	va_list args;
	char buffer[PMTMPV];
	int i = 0;
	if (!items) return 0;
	while (items[i].code < 2) {
		if (items[i].index == index) {
			va_start(args, format);
			vsprintf(buffer, format, args);
			va_end(args);
			buffer[31] = 0;		// Menu only have 32 characters
			strcpy(items[i].caption, buffer);
			return 1;
		}
		i++;
	}
	return 0;
}

void UIMenu_BeginMessage(void)
{
	UIMenu_MsgLines = 0;
}

void UIMenu_SetMessage(char *message, int color)
{
	strcpy(UIMenu_FileListCache[UIMenu_MsgLines].name, message);
	UIMenu_FileListCache[UIMenu_MsgLines].color = color;
	UIMenu_MsgLines++;
}

void UIMenu_EndMessage(int timeout)
{
	UIMenu_Page = UIPAGE_MESSAGE;
	UIMenu_MsgOffset = 0;
	UIMenu_MsgCountDw = UIMenu_MsgCountReset1;
	UIMenu_MsgTimer = timeout;
}

void UIMenu_RealTimeMessage(TUIRealtimeCB cb)
{
	if (cb == NULL) return;
	UIMenu_Page = UIPAGE_REALTIMETEXT;
	UIRealtimeCB = cb;
	UIRealtimeCB(1, NULL);
}
