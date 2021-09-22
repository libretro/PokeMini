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

int UI_FirstLoad = 1;
int UI_Enabled = 0;
int UI_Status = UI_STATUS_GAME;
int UI_PreviewDist = 12;
uint32_t *UI_Font1_Pal32 = NULL;
uint16_t *UI_Font1_Pal16 = NULL;
uint32_t *UI_Font2_Pal32 = NULL;
uint16_t *UI_Font2_Pal16 = NULL;
uint32_t *UI_Icons_Pal32 = NULL;
uint16_t *UI_Icons_Pal16 = NULL;

#define UIMenu_FilesLines	(UIMenu_Lines-1)

int UIMenu_Width = 0;	// Menu width
int UIMenu_Height = 0;	// Menu height
int UIMenu_PixelLayout = 0;	// Menu pixel layout
int UIMenu_Lines = 0;	// Menu visible lines

uint8_t *UIMenu_BGImage = NULL;
uint32_t *UIMenu_BGPal32 = NULL;
uint16_t *UIMenu_BGPal16 = NULL;

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

int UIItems_OptionsC(int index, int reason);
TUIMenu_Item UIItems_Options[] = {
	{ 0,  0, "Go Back...",         UIItems_OptionsC, NULL },
	{ 0,  1, "Palette: %s",        UIItems_OptionsC, NULL },
	{ 0,  2, "LCD Mode: %s",       UIItems_OptionsC, NULL },
	{ 0,  3, "LCD Filter: %s",     UIItems_OptionsC, NULL },
	{ 0, 10, "Contrast: %i%%",     UIItems_OptionsC, NULL },
	{ 0, 11, "Bright: %i%%",       UIItems_OptionsC, NULL },
	{ 0,  4, "Sound: %s",          UIItems_OptionsC, NULL },
	{ 0,  5, "Piezo Filter: %s",   UIItems_OptionsC, NULL },
	{ 0,  6, "PM Battery: %s",     UIItems_OptionsC, NULL },
	{ 0,  7, "RTC: %s",            UIItems_OptionsC, NULL },
	{ 0,  8, "Shared EEP.: %s",    UIItems_OptionsC, NULL },
	{ 0,  9, "Force FreeBIOS: %s", UIItems_OptionsC, NULL },
#ifndef PERFORMANCE
	{ 0, 20, "Multicart: %s",      UIItems_OptionsC, NULL },
#endif
	{ 0, 50, "Sync Cycles: %d",      UIItems_OptionsC, NULL },
	{ 0, 60, "Reload Color Info...", UIItems_OptionsC, NULL },
	{ 0, 99, "Save Configs...",      UIItems_OptionsC, NULL },
	{ 9,  0, "Options",              UIItems_OptionsC, NULL }
};

int UIItems_PalEditC(int index, int reason);
TUIMenu_Item UIItems_PalEdit[] = {
	{ 0,  0, "Go Back...",        UIItems_PalEditC, NULL },
	{ 0,  1, "1-Light   Red: %d", UIItems_PalEditC, NULL },
	{ 0,  2, "1-Light Green: %d", UIItems_PalEditC, NULL },
	{ 0,  3, "1-Light  Blue: %d", UIItems_PalEditC, NULL },
	{ 0,  4, "1-Dark    Red: %d", UIItems_PalEditC, NULL },
	{ 0,  5, "1-Dark  Green: %d", UIItems_PalEditC, NULL },
	{ 0,  6, "1-Dark   Blue: %d", UIItems_PalEditC, NULL },
	{ 0,  7, "1-Light   Red: %d", UIItems_PalEditC, NULL },
	{ 0,  8, "1-Light Green: %d", UIItems_PalEditC, NULL },
	{ 0,  9, "1-Light  Blue: %d", UIItems_PalEditC, NULL },
	{ 0, 10, "1-Dark    Red: %d", UIItems_PalEditC, NULL },
	{ 0, 11, "1-Dark  Green: %d", UIItems_PalEditC, NULL },
	{ 0, 12, "1-Dark   Blue: %d", UIItems_PalEditC, NULL },
	{ 9,  0, "Palette Edit",      UIItems_PalEditC, NULL }
};

void UIMenu_SwapEntries(int a, int b)
{
	TUIMenu_FileListCache tmp;
	memcpy(&tmp, &UIMenu_FileListCache[a], sizeof(TUIMenu_FileListCache));
	memcpy(&UIMenu_FileListCache[a], &UIMenu_FileListCache[b], sizeof(TUIMenu_FileListCache));
	memcpy(&UIMenu_FileListCache[b], &tmp, sizeof(TUIMenu_FileListCache));
}

#ifndef NO_SCANDIRS

int UIMenu_ReadDir(char *dirname)
{
	int i, j, cmp, hasslash, isdir, items = 0;
	char file[PMTMPV];

	// Clear all cache
	for (i=0; i<UI_MAXCACHE; i++) {
		UIMenu_FileListCache[i].name[0] = 0;
		UIMenu_FileListCache[i].stats = 0;
		UIMenu_FileListCache[i].color = 0;
	}

	// Read directories and files
	hasslash = HasLastSlash(dirname);
#ifdef FS_DC
	file_t d = fs_open(dirname, O_RDONLY | O_DIR);
	dirent_t *de;
	if (strlen(dirname) > 1) {
		strcpy(UIMenu_FileListCache[0].name, "..");
		UIMenu_FileListCache[0].stats = 1;
		items = 1;
	}
	while ( (de = fs_readdir(d)) ) {
		if (de->name[0] == 0) break;
		if (de->name[0] == '.') continue;
		UIMenu_FileListCache[items].stats = 1;
		strcpy(UIMenu_FileListCache[items].name, de->name);
		if (hasslash) sprintf(file, "%s%s", dirname, de->name);
		else sprintf(file, "%s/%s", dirname, de->name);
		isdir = (de->size < 0);
		if (isdir) {
			// Directory
			UIMenu_FileListCache[items++].stats = 1;
			if (items >= UI_MAXCACHE) break;
		} else {
			// File
			if (ExtensionCheck(de->name, ".min")) {
				UIMenu_FileListCache[items].stats = 2;
				sprintf(file, "%sc", de->name);
				if (FileExist(file)) UIMenu_FileListCache[items].color = 1;
				items++;
#ifndef NO_ZIP
			} else if (ExtensionCheck(de->name, ".zip")) {
				UIMenu_FileListCache[items].stats = 2;
				UIMenu_FileListCache[items].color = 2;
				items++;
#endif
			}
			if (items >= UI_MAXCACHE) break;
		}
	}
	fs_close(d);
#else
   {
      RDIR *dir = retro_opendir(dirname);
      if (dir == NULL) {
         PokeDPrint(POKEMSG_ERR, "retro_opendir('%s') error\n", dirname);
         return 0;
      }
      while((retro_readdir(dir)))
      {
         if (!retro_dirent_get_name(dir))
            break;
         UIMenu_FileListCache[items].stats = 0;
         strcpy(UIMenu_FileListCache[items].name, retro_dirent_get_name(dir));
         // Current directory
         if (strcmp(retro_dirent_get_name(dir), ".") == 0)
            continue;
         else
         {
            // Current directory, file or directory
            if (hasslash) sprintf(file, "%s%s", dirname, retro_dirent_get_name(dir));
            else sprintf(file, "%s/%s", dirname, retro_dirent_get_name(dir));
            if (!path_is_valid(file))
            {
               PokeDPrint(POKEMSG_ERR, "stat('%s') error\n", file);
               continue;
            }
            else
               isdir = path_is_directory(file);
         }
         if (isdir)
         {
            // Directory
            UIMenu_FileListCache[items++].stats = 1;
            if (items >= UI_MAXCACHE) break;
         } else
         {
            // File
            if (ExtensionCheck(retro_dirent_get_name(dir), ".min")) {
               UIMenu_FileListCache[items].stats = 2;
               sprintf(file, "%sc", retro_dirent_get_name(dir));
               if (FileExist(file)) UIMenu_FileListCache[items].color = 1;
               items++;
#ifndef NO_ZIP
            } else if (ExtensionCheck(retro_dirent_get_name(dir), ".zip")) {
               UIMenu_FileListCache[items].stats = 2;
               UIMenu_FileListCache[items].color = 2;
               items++;
#endif
            }
         }
      }
      retro_closedir(dir);
   }
#endif

	// Sort the list
	for (i=0; i<items; i++) {
		for (j=i; j<items; j++) {
			cmp = strcasecmp(UIMenu_FileListCache[j].name, UIMenu_FileListCache[i].name);
			if (UIMenu_FileListCache[j].stats < UIMenu_FileListCache[i].stats) {
				UIMenu_SwapEntries(i, j);
			} else if (cmp < 0) {
				UIMenu_SwapEntries(i, j);
			}
		}
	}

	return items;
}

void UIMenu_GotoRelativeDir(char *newdir)
{
	char file[PMTMPV];
	int hasslash;

	hasslash = HasLastSlash(PokeMini_CurrDir);

	if (newdir) {
		if (hasslash)
			sprintf(file, "%s%s", PokeMini_CurrDir, newdir);
		else
			sprintf(file, "%s/%s", PokeMini_CurrDir, newdir);
#ifdef FS_DC
		chdir(file);
#else
      if (path_is_valid(file))
			PokeDPrint(POKEMSG_ERR, "stat('%s') error\n", file);
		else {
			if (path_is_directory(file))
         {
            if (chdir(file))
               PokeDPrint(POKEMSG_ERR, "rel chdir('%s') error\n", file);
         }
		}
#endif
	} else {
		PokeMini_GotoCurrentDir();
	}
	PokeMini_GetCurrentDir();
}

#else

int UIMenu_ReadDir(char *dirname) { return 0; }
void UIMenu_GotoRelativeDir(char *newdir) {}

#endif

void UIMenu_KeyEvent(int key, int press)
{
	if (key == MINX_KEY_C) UIMenu_CKeyMod = press;
	if (press) {
		if (UI_Enabled && UI_Status) {
			UIMenu_InKey = key;
		} else {
			PokeMini_KeypadEvent(key, 1);
		}
	} else {
		PokeMini_KeypadEvent(key, 0);
	}
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

char *UIMenuTxt_Palette[16] = {
	"Default", "Old", "Black & White", "Green Palette",
	"Green Vector",	"Red Palette", "Red Vector", "Blue LCD",
	"LEDBacklight", "Girl Power", "Blue Palette", "Blue Vector",
	"Sepia", "Inv. B&W", "Custom 1...", "Custom 2..."
};

char *UIMenuTxt_LCDMode[4] = {
	"Analog", "3-Shades", "2-Shades", "Colors"
};

char *UIMenuTxt_LCDFilter[3] = {
	"None", "Matrix/Hi", "Scanline"
};

char *UIMenuTxt_Sound[6] = {
	"Disabled", "Generated", "Direct", "Emulated", "Direct PWM"
};

char *UIMenuTxt_Battery[3] = {
	"Full", "Low", "Auto"
};

char *UIMenuTxt_RTC[3] = {
	"Off", "State time diff.", "From Host"
};

char *UIMenuTxt_Multicart[3] = {
	"Disabled", "Flash 512K", "Lupin 512K"
};

char *UIMenuTxt_Enabled[2] = {
	"Disabled", "Enabled"
};

int UIItems_OptionsC(int index, int reason)
{
	char tmp[PMTMPV];

	// Options
	if (reason == UIMENU_OK) {
		switch (index) {
			case 0: // Go back...
				reason = UIMENU_CANCEL;
				break;
			case 1: // Palette
				UIMenu_LoadItems(UIItems_PalEdit, 0);
				break;
			case 2: case 3: case 4: case 5:
			case 6: case 7: case 8: case 9:
				reason = UIMENU_RIGHT;
				break;
			case 20: // Multicart
				reason = UIMENU_RIGHT;
				break;
			case 60: // Reload Color Info...
				UIMenu_BeginMessage();
				UIMenu_SetMessage("Reload Color Info...", 1);
				UIMenu_SetMessage("", 1);
				sprintf(tmp, "%sc", CommandLine.min_file);
				if (!FileExist(tmp) || !PokeMini_LoadColorFile(tmp)) {
					if (CommandLine.lcdmode == 3) CommandLine.lcdmode = 0;
					UIMenu_SetMessage("Reload failed!", 0);
					UIMenu_EndMessage(240);
				} else {
					CommandLine.lcdmode = 3;
					UIMenu_SetMessage("Reload complete!", 0);
					UIMenu_EndMessage(60);
				}
				break;
			case 99: // Save configs...
				UIMenu_BeginMessage();
				UIMenu_SetMessage("Save Configs...", 1);
				UIMenu_SetMessage("", 1);
				PokeMini_GotoExecDir();
				UIMenu_EndMessage(240);
				break;
		}
	}
	if (reason == UIMENU_CANCEL) {
		UIMenu_PrevMenu();
		return 1;
	}
	if (reason == UIMENU_LEFT) {
		switch (index) {
			case 1: CommandLine.palette = (CommandLine.palette - 1) & 15;
				PokeMini_VideoPalette_Index(CommandLine.palette, CommandLine.custompal, CommandLine.lcdcontrast, CommandLine.lcdbright);
				break;
			case 2: CommandLine.lcdmode--;
				if (CommandLine.lcdmode < 0) CommandLine.lcdmode = PRCColorMap ? 3 : 2;
				break;
			case 3: CommandLine.lcdfilter--;
				if (CommandLine.lcdfilter < 0) CommandLine.lcdfilter = 2;
				break;
			case 10: CommandLine.lcdcontrast -= 2;
				if (CommandLine.lcdcontrast < 0) CommandLine.lcdcontrast = 100;
				PokeMini_VideoPalette_Index(CommandLine.palette, CommandLine.custompal, CommandLine.lcdcontrast, CommandLine.lcdbright);
				break;
			case 11: CommandLine.lcdbright -= 2;
				if (CommandLine.lcdbright < -100) CommandLine.lcdbright = 100;
				PokeMini_VideoPalette_Index(CommandLine.palette, CommandLine.custompal, CommandLine.lcdcontrast, CommandLine.lcdbright);
				break;
			case 4: if (PokeMini_Flags & POKEMINI_GENSOUND) {
					CommandLine.sound = !CommandLine.sound;
				} else {
					CommandLine.sound--;
					if (CommandLine.sound < 0) CommandLine.sound = 4;
				}
				break;
			case 5: CommandLine.piezofilter = !CommandLine.piezofilter;
				break;
			case 6: if (PokeMini_Flags & POKEMINI_AUTOBATT)
					CommandLine.low_battery--;
				else
					CommandLine.low_battery = !CommandLine.low_battery;
				if (CommandLine.low_battery < 0) CommandLine.low_battery = 2;
				break;
			case 7: CommandLine.updatertc--;
				if (CommandLine.updatertc < 0) CommandLine.updatertc = 2;
				break;
			case 8: CommandLine.eeprom_share = !CommandLine.eeprom_share;
				break;
			case 9: CommandLine.forcefreebios = !CommandLine.forcefreebios;
				break;
			case 20: CommandLine.multicart--;
				if (CommandLine.multicart < 0) CommandLine.multicart = 2;
				break;
			case 50: CommandLine.synccycles >>= 1;
				if (CommandLine.synccycles < 8) CommandLine.synccycles = 8;
				break;
		}
	}
	if (reason == UIMENU_RIGHT) {
		switch (index) {
			case 1: CommandLine.palette = (CommandLine.palette + 1) & 15;
				PokeMini_VideoPalette_Index(CommandLine.palette, CommandLine.custompal, CommandLine.lcdcontrast, CommandLine.lcdbright);
				break;
			case 2: CommandLine.lcdmode++;
				if (CommandLine.lcdmode > (PRCColorMap ? 3 : 2)) CommandLine.lcdmode = 0;
				break;
			case 3: CommandLine.lcdfilter++;
				if (CommandLine.lcdfilter > 2) CommandLine.lcdfilter = 0;
				break;
			case 10: CommandLine.lcdcontrast += 2;
				if (CommandLine.lcdcontrast > 100) CommandLine.lcdcontrast = 0;
				PokeMini_VideoPalette_Index(CommandLine.palette, CommandLine.custompal, CommandLine.lcdcontrast, CommandLine.lcdbright);
				break;
			case 11: CommandLine.lcdbright += 2;
				if (CommandLine.lcdbright > 100) CommandLine.lcdbright = -100;
				PokeMini_VideoPalette_Index(CommandLine.palette, CommandLine.custompal, CommandLine.lcdcontrast, CommandLine.lcdbright);
				break;
			case 4: if (PokeMini_Flags & POKEMINI_GENSOUND) {
					CommandLine.sound = !CommandLine.sound;
				} else {
					CommandLine.sound++;
					if (CommandLine.sound > 4) CommandLine.sound = 0;
				}
				break;
			case 5: CommandLine.piezofilter = !CommandLine.piezofilter;
				break;
			case 6: if (PokeMini_Flags & POKEMINI_AUTOBATT)
					CommandLine.low_battery++;
				else
					CommandLine.low_battery = !CommandLine.low_battery;
				if (CommandLine.low_battery > 2) CommandLine.low_battery = 0;
				break;
			case 7: CommandLine.updatertc++;
				if (CommandLine.updatertc > 2) CommandLine.updatertc = 0;
				break;
			case 8: CommandLine.eeprom_share = !CommandLine.eeprom_share;
				break;
			case 9: CommandLine.forcefreebios = !CommandLine.forcefreebios;
				break;
			case 20: CommandLine.multicart++;
				if (CommandLine.multicart > 2) CommandLine.multicart = 0;
				break;
			case 50: CommandLine.synccycles <<= 1;
#ifdef PERFORMANCE
				if (CommandLine.synccycles > 512) CommandLine.synccycles = 512;
#else
				if (CommandLine.synccycles > 64) CommandLine.synccycles = 64;
#endif
				break;
		}
	}

	// Update items
	UIMenu_ChangeItem(UIItems_Options,  1, "Palette: %s", UIMenuTxt_Palette[CommandLine.palette]);
	UIMenu_ChangeItem(UIItems_Options,  2, "LCD Mode: %s", UIMenuTxt_LCDMode[CommandLine.lcdmode]);
	UIMenu_ChangeItem(UIItems_Options,  3, "LCD Filter: %s", UIMenuTxt_LCDFilter[CommandLine.lcdfilter]);
	UIMenu_ChangeItem(UIItems_Options, 10, "Contrast: %i%%", CommandLine.lcdcontrast);
	UIMenu_ChangeItem(UIItems_Options, 11, "Bright: %i%%", CommandLine.lcdbright);
	if (PokeMini_Flags & POKEMINI_NOSOUND) {
		CommandLine.sound = 0;
		UIMenu_ChangeItem(UIItems_Options,  4, "Sound: Disabled");
	} else if (PokeMini_Flags & POKEMINI_GENSOUND) {
		CommandLine.sound = CommandLine.sound ? 1 : 0;
		UIMenu_ChangeItem(UIItems_Options,  4, "Sound: %s", UIMenuTxt_Enabled[CommandLine.sound]);
	} else {
		UIMenu_ChangeItem(UIItems_Options,  4, "Sound: %s", UIMenuTxt_Sound[CommandLine.sound]);
	}
	UIMenu_ChangeItem(UIItems_Options,  5, "Piezo Filter: %s", CommandLine.piezofilter ? "Yes" : "No");
	if (PokeMini_Flags & POKEMINI_AUTOBATT) {
		UIMenu_ChangeItem(UIItems_Options,  6, "PM Batt.: %s (%s)", UIMenuTxt_Battery[CommandLine.low_battery], UIMenuTxt_Battery[PokeMini_HostBattStatus]);
	} else {
		UIMenu_ChangeItem(UIItems_Options,  6, "PM Battery: %s", UIMenuTxt_Battery[CommandLine.low_battery]);
	}
	UIMenu_ChangeItem(UIItems_Options,  7, "RTC: %s", UIMenuTxt_RTC[CommandLine.updatertc]);
	UIMenu_ChangeItem(UIItems_Options,  8, "Shared EEP.: %s", CommandLine.eeprom_share ? "Yes" : "No");
	UIMenu_ChangeItem(UIItems_Options,  9, "Force FreeBIOS: %s", CommandLine.forcefreebios ? "Yes" : "No");
	UIMenu_ChangeItem(UIItems_Options, 20, "Multicart: %s", UIMenuTxt_Multicart[CommandLine.multicart]);
	UIMenu_ChangeItem(UIItems_Options, 50, "Sync Cycles: %d", CommandLine.synccycles);

	return 1;
}

int UIItems_PalEditC(int index, int reason)
{
	uint8_t r, g, b, ic, ix;
	const int deco[] = {0, 0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14};

	// Palette Editor
	if (reason == UIMENU_OK) {
		switch (index) {
			case 0: // Go back...
				reason = UIMENU_CANCEL;
				break;
		}
	}
	if (reason == UIMENU_CANCEL) {
		UIMenu_PrevMenu();
		return 1;
	}
	if (reason == UIMENU_LEFT) {
		if (index > 0) {
			ic = deco[index] & 3;
			ix = (deco[index] & 12) >> 2;
			r = GetValH24(CommandLine.custompal[ix]);
			g = GetValM24(CommandLine.custompal[ix]);
			b = GetValL24(CommandLine.custompal[ix]);
			if (UIMenu_CKeyMod) {
				if (ic == 0) r-=16;
				if (ic == 1) g-=16;
				if (ic == 2) b-=16;
			} else {
				if (ic == 0) r--;
				if (ic == 1) g--;
				if (ic == 2) b--;
			}
			CommandLine.custompal[ix] = RGB24(b, g, r);
			PokeMini_VideoPalette_Index(CommandLine.palette, CommandLine.custompal, CommandLine.lcdcontrast, CommandLine.lcdbright);
		}
	}
	if (reason == UIMENU_RIGHT) {
		if (index > 0) {
			ic = deco[index] & 3;
			ix = (deco[index] & 12) >> 2;
			r = GetValH24(CommandLine.custompal[ix]);
			g = GetValM24(CommandLine.custompal[ix]);
			b = GetValL24(CommandLine.custompal[ix]);
			if (UIMenu_CKeyMod) {
				if (ic == 0) r+=16;
				if (ic == 1) g+=16;
				if (ic == 2) b+=16;
			} else {
				if (ic == 0) r++;
				if (ic == 1) g++;
				if (ic == 2) b++;
			}
			CommandLine.custompal[ix] = RGB24(b, g, r);
			PokeMini_VideoPalette_Index(CommandLine.palette, CommandLine.custompal, CommandLine.lcdcontrast, CommandLine.lcdbright);
		}
	}

	// Update items
	UIMenu_ChangeItem(UIItems_PalEdit,  1, "1-Light   Red: %d", GetValH24(CommandLine.custompal[0]));
	UIMenu_ChangeItem(UIItems_PalEdit,  2, "1-Light Green: %d", GetValM24(CommandLine.custompal[0]));
	UIMenu_ChangeItem(UIItems_PalEdit,  3, "1-Light  Blue: %d", GetValL24(CommandLine.custompal[0]));
	UIMenu_ChangeItem(UIItems_PalEdit,  4, "1-Dark    Red: %d", GetValH24(CommandLine.custompal[1]));
	UIMenu_ChangeItem(UIItems_PalEdit,  5, "1-Dark  Green: %d", GetValM24(CommandLine.custompal[1]));
	UIMenu_ChangeItem(UIItems_PalEdit,  6, "1-Dark   Blue: %d", GetValL24(CommandLine.custompal[1]));
	UIMenu_ChangeItem(UIItems_PalEdit,  7, "2-Light   Red: %d", GetValH24(CommandLine.custompal[2]));
	UIMenu_ChangeItem(UIItems_PalEdit,  8, "2-Light Green: %d", GetValM24(CommandLine.custompal[2]));
	UIMenu_ChangeItem(UIItems_PalEdit,  9, "2-Light  Blue: %d", GetValL24(CommandLine.custompal[2]));
	UIMenu_ChangeItem(UIItems_PalEdit, 10, "2-Dark    Red: %d", GetValH24(CommandLine.custompal[3]));
	UIMenu_ChangeItem(UIItems_PalEdit, 11, "2-Dark  Green: %d", GetValM24(CommandLine.custompal[3]));
	UIMenu_ChangeItem(UIItems_PalEdit, 12, "2-Dark   Blue: %d", GetValL24(CommandLine.custompal[3]));

	return 1;
}

int UIItems_PlatformDefC(int index, int reason)
{
	if (reason == UIMENU_OK) {
		if (index == 99) { // Save configs...
			UIMenu_BeginMessage();
			UIMenu_SetMessage("Save Configs...", 1);
			UIMenu_SetMessage("", 1);
			PokeMini_GotoExecDir();
			UIMenu_EndMessage(240);
		} else reason = UIMENU_CANCEL;
	}
	if (reason == UIMENU_CANCEL) {
		UIMenu_PrevMenu();
		return 1;
	}
	return 1;
}
