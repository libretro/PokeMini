/*
  PokeMini - Pok�mon-Mini Emulator
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
#include "Endianess.h"
#include <time.h>

// Dependencies
#ifndef NO_ZIP
#include "unzip.h"
#endif

// Include Free BIOS
#include "freebios.h"
int PokeMini_FreeBIOS = 0;	// Using freebios?

int PokeMini_Flags = 0;		// Configuration flags
uint8_t PM_BIOS[4096];		// Pokemon-Mini BIOS ($000000 to $000FFF, 4096)
uint8_t PM_RAM[8192];		// Pokemon-Mini RAM  ($001000 to $002100, 4096 + 256)
uint8_t *PM_ROM = NULL;		// Pokemon-Mini ROM  ($002100 to $1FFFFF, Up to 2MB)
int PM_ROM_Alloc = 0;		// Pokemon-Mini ROM Allocated on memory?
int PM_ROM_Size = 0;		// Pokemon-Mini ROM Size
int PM_ROM_Mask = 0;		// Pokemon-Mini ROM Mask
int PokeMini_LCDMode = 0;	// LCD Mode
int PokeMini_ColorFormat = 0;	// Color Format (0 = 8x8, 1 = 4x4)
int PokeMini_HostBattStatus = 0;// Host battery status
int PokeMini_RumbleAnim = 0;	// Rumble animation


int PokeMini_RumbleAmount[4] = { -2,  1, -1,  2 };
const int PokeMini_RumbleAmountTable[16] = {
	 0,  0,  0,  0,
	-1,  0,  1,  0,
	-1,  1, -1,  1,
	-2,  1, -1,  2,
};

const uint8_t PM_IO_INIT[256] = {
	0x7F, 0x20, 0x5C, 0xff, 0xff, 0xff, 0xff, 0xff, // $00~$07 System Control
	0x01, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, // $08~$0F Second Counter
	0x08, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // $10~$17 Battery Sensor
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, // $18~$1F Timers Controls
	0x00, 0x30, 0x02, 0x00, 0x02, 0x00, 0x00, 0x40, // $20~$27 IRQ
	0x00, 0xC0, 0x40, 0xff, 0xff, 0xff, 0xff, 0xff, // $28~$2F IRQ
	0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, // $30~$37 Timer 1
	0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, // $38~$3F Timer 2
	0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, // $40~$47 256Hz Counter + ???
	0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, // $48~$4F Timer 3
	0xFF, 0x00, 0xFF, 0x00, 0x01, 0x01, 0xff, 0xff, // $50~$57 Keypad + ???
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // $58~$5F Unused
	0x32, 0x64, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, // $60~$67 I/O
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // $68~$6F Unused
	0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // $70~$77 Audio + ???
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // $78~$7F Unused
	0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // $80~$87 PRC
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // $88~$8F PRC
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // $90~$97 Unused
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // $98~$9F Unused
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // $A0~$A7 Unused
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // $A8~$AF Unused
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // $B0~$B7 Unused
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // $B8~$BF Unused
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // $C0~$C7 Unused
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // $C8~$CF Unused
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // $D0~$D7 Unused
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // $D8~$DF Unused
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // $E0~$E7 Unused
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // $E8~$EF Unused
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // $F0~$F7 ???
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x40, 0xFF  // $F8~$FF LCD I/O
};

// Callbacks
void (*PokeMini_OnAllocMIN)(int newsize, int success) = NULL;
void (*PokeMini_OnUnzipError)(const char *zipfile, const char *reason) = NULL;
void (*PokeMini_OnLoadBIOSFile)(const char *filename, int success) = NULL;
void (*PokeMini_OnLoadMINFile)(const char *filename, int success) = NULL;
void (*PokeMini_OnLoadColorFile)(const char *filename, int success) = NULL;
void (*PokeMini_OnLoadEEPROMFile)(const char *filename, int success) = NULL;
void (*PokeMini_OnSaveEEPROMFile)(const char *filename, int success) = NULL;
void (*PokeMini_OnReset)(int hardreset) = NULL;

int (*PokeMini_CustomLoadEEPROM)(const char *filename) = NULL;
int (*PokeMini_CustomSaveEEPROM)(const char *filename) = NULL;

// Number of cycles to process on hardware
int PokeHWCycles = 0;

// Create emulator and all interfaces
int PokeMini_Create(int flags, int soundfifo)
{
	// Clear BIOS & RAM
	memset(PM_BIOS, 0xFF, 4096);
	memset(PM_RAM, 0xFF, 8192);	// RAM + IO

	// Dummy ROM (In case LoadROM fail)
	PM_ROM_Alloc = 0;
	PM_ROM_Mask = 0x1FFF;
	PM_ROM_Size = 0x2000;
	PM_ROM = (unsigned char *)PM_RAM;

	// Clear & Init State
	memcpy(PM_IO, PM_IO_INIT, 256);
	if (!MinxCPU_Create()) return 0;
	if (!MinxTimers_Create()) return 0;
	if (!MinxIO_Create()) return 0;
	if (!MinxIRQ_Create()) return 0;
	if (!MinxPRC_Create()) return 0;
	if (!MinxColorPRC_Create()) return 0;
	if (!MinxLCD_Create()) return 0;
	if (!MinxAudio_Create(soundfifo, soundfifo)) return 0;

	// Initialize variables
	PokeMini_Flags = flags;

	// Load FreeBIOS
	PokeMini_LoadFreeBIOS();

#ifndef PERFORMANCE
	// Initialize multicart
	NewMulticart();
	SetMulticart(CommandLine.multicart);
#endif

	return 1;
}

// Destroy emulator and all interfaces
void PokeMini_Destroy()
{
	// Destroy all interfaces
	MinxAudio_Destroy();
	MinxLCD_Destroy();
	MinxPRC_Destroy();
	MinxColorPRC_Destroy();
	MinxIRQ_Destroy();
	MinxIO_Destroy();
	MinxTimers_Destroy();
	MinxCPU_Destroy();

	// Free ROM if was allocated
	if (PM_ROM_Alloc) {
		free(PM_ROM);
		PM_ROM = NULL;
		PM_ROM_Alloc = 0;
	}

	// Free color info
	PokeMini_FreeColorInfo();
}

// Apply changes from command lines
void PokeMini_ApplyChanges()
{
	int i;
	if (CommandLine.low_battery != 2) MinxIO_BatteryLow(CommandLine.low_battery);
	PokeMini_SetLCDMode(CommandLine.lcdmode);
	PokeMini_SetVideo(PokeMini_VideoCurrent, PokeMini_VideoDepth, CommandLine.lcdfilter, CommandLine.lcdmode);
	MinxAudio_ChangeFilter(CommandLine.piezofilter);
	for (i=0; i<4; i++) PokeMini_RumbleAmount[i] = PokeMini_RumbleAmountTable[((CommandLine.rumblelvl & 3) << 2) | i];
}

// User press or release a Pokemon-Mini key
void PokeMini_KeypadEvent(uint8_t key, int pressed)
{
	MinxIO_Keypad(key, pressed);
}

// Low power battery emulation
void PokeMini_LowPower(int enable)
{
	if (CommandLine.low_battery == 2) {
		PokeMini_HostBattStatus = enable;
		if (PokeMini_BatteryStatus != enable) MinxIO_BatteryLow(enable);
	}
}

// Set LCD mode
void PokeMini_SetLCDMode(int mode)
{
	PokeMini_LCDMode = mode;
	MinxPRC_Render = MinxPRC_Render_Mono;
	if (mode == LCDMODE_COLORS) {
		if (PokeMini_ColorFormat == 1) MinxPRC_Render = MinxPRC_Render_Color4;
		else MinxPRC_Render = MinxPRC_Render_Color8;
	}
}

// Generate rumble offset
int PokeMini_GenRumbleOffset(int pitch)
{
	return PokeMini_RumbleAmount[PokeMini_RumbleAnim++ & 3] * pitch;
}

// Load BIOS from file
int PokeMini_LoadBIOSFile(const char *filename)
{
	FILE *fbios;
	int readbytes;

	// Open file
	fbios = fopen(filename, "rb");
	if (fbios == NULL) {
		if (PokeMini_OnLoadBIOSFile) PokeMini_OnLoadBIOSFile(filename, -1);
		return 0;
	}

	// Read content
	readbytes = fread(PM_BIOS, 1, 4096, fbios);
	PokeMini_FreeBIOS = 0;

	// Close file
	fclose(fbios);

	// Callback
	if (PokeMini_OnLoadBIOSFile) PokeMini_OnLoadBIOSFile(filename, (readbytes == 4096) ? 1 : 0);

	return (readbytes == 4096);
}

// Save BIOS from file
int PokeMini_SaveBIOSFile(const char *filename)
{
	FILE *fbios;
	int writebytes;

	// Open file
	fbios = fopen(filename, "wb");
	if (fbios == NULL) {
		return 0;
	}

	// Read content
	writebytes = fwrite(PM_BIOS, 1, 4096, fbios);

	// Close file
	fclose(fbios);

	return (writebytes == 4096);
}

// Load FreeBIOS
int PokeMini_LoadFreeBIOS()
{
	PokeMini_FreeBIOS = 1;
	memcpy(PM_BIOS, FreeBIOS, 4096);
	return 1;
}

// New MIN ROM
int PokeMini_NewMIN(uint32_t size)
{
	// Allocate ROM and set cartridge size
	if (PM_ROM_Alloc) {
		free(PM_ROM);
		PM_ROM = NULL;
		PM_ROM_Alloc = 0;
	}
	PM_ROM_Mask = GetMultiple2Mask(size);
	PM_ROM_Size = PM_ROM_Mask + 1;
	PM_ROM = (uint8_t *)malloc(PM_ROM_Size);
	if (!PM_ROM) {
		if (PokeMini_OnAllocMIN) PokeMini_OnAllocMIN(PM_ROM_Size, 0);
		return 0;
	}
	memset(PM_ROM, 0xFF, PM_ROM_Size);
	PM_ROM_Alloc = 1;

	if (PokeMini_OnAllocMIN) PokeMini_OnAllocMIN(PM_ROM_Size, 1);
	return 1;
}

// Load MIN ROM
int PokeMini_LoadMINFile(const char *filename)
{
	FILE *fi;
	int size, readbytes;

	// Open file
	fi = fopen(filename, "rb");
	if (fi == NULL) {
		if (PokeMini_OnLoadMINFile) PokeMini_OnLoadMINFile(filename, -1);
		return 0;
	}

	// Check filesize
	fseek(fi, 0, SEEK_END);
	size = ftell(fi);

	// Check if size is valid
	if ((size <= 0x2100) || (size > 0x200000)) {
		fclose(fi);
		if (PokeMini_OnLoadMINFile) PokeMini_OnLoadMINFile(filename, -2);
		return 0;
	}

	// Free existing color information
	PokeMini_FreeColorInfo();

	// Allocate ROM and set cartridge size
	if (!PokeMini_NewMIN(size)) {
		fclose(fi);
		return 0;
	}

	// Read content
	fseek(fi, 0, SEEK_SET);
	readbytes = fread(PM_ROM, 1, size, fi);
	fclose(fi);

	// Callback
	if (PokeMini_OnLoadMINFile) PokeMini_OnLoadMINFile(filename, (readbytes == size) ? 1 : 0);
	NewMulticart();

	return (readbytes == size);
}

// Save MIN ROM
int PokeMini_SaveMINFile(const char *filename)
{
	FILE *fi;
	int writebytes;

	// Open file
	fi = fopen(filename, "wb");
	if (fi == NULL) {
		return 0;
	}

	// Write content
	writebytes = fwrite(PM_ROM, 1, PM_ROM_Size, fi);
	fclose(fi);

	return (writebytes == PM_ROM_Size);
}

// Set MIN from memory
int PokeMini_SetMINMem(uint8_t *mem, int size)
{
	if (PM_ROM_Alloc) { free(PM_ROM); PM_ROM = NULL; PM_ROM_Alloc = 0; }
	PM_ROM_Mask = GetMultiple2Mask(size);
	PM_ROM_Size = PM_ROM_Mask + 1;
	PM_ROM = mem;
	PM_ROM_Alloc = 0;
	NewMulticart();
	return 1;
}

// Free color information
void PokeMini_FreeColorInfo()
{
	if (PRCColorMap) {
		free(PRCColorMap);
		PRCColorMap = NULL;
	}
}

// Syncronize host time
int PokeMini_SyncHostTime()
{
#ifndef NO_RTC
	// Modify EEPROM for host time
	if (CommandLine.updatertc == 2) {
		time_t tim = time(NULL);
		struct tm *now = localtime(&tim);
		MinxIO_SetTimeStamp(now->tm_year % 100, now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
		PMR_SEC_CTRL = 0x01;
		MinxTimers.SecTimerCnt = 0;
		return 1;
	}
#endif
	return 0;
}

#define DATAREADFROMFILE(var, size) {\
	if (stream(var, size, stream_ptr) != size) {\
		return 0;\
	}\
}

const uint8_t RemapMINC10_11[16] = {
	 0, 1, 2, 4, 5, 6, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 
};

// Load color information from stream, note that PokeMini_OnLoadColorFile callback isn't called!
int PokeMini_LoadColorStream(TPokeMini_StreamIO stream, void *stream_ptr)
{
	uint8_t hdr[4], vcod[4], reserved[16];
	uint32_t maptiles, mapoffset, bytespertile;
	int i, readbytes;

	// Check header
	DATAREADFROMFILE(hdr, 4);		// ID
	if ((hdr[0] != 'M') || (hdr[1] != 'I') || (hdr[2] != 'N') || (hdr[3] != 'c')) return 0;
	DATAREADFROMFILE(vcod, 4);		// VCode
	if (vcod[0] != 0x01) return 0;		// Only version 1 supported
	if (vcod[1] > 0x01) return 0;		// Only color type 0 and 1 are valid
	PRCColorFlags = vcod[2];
	PokeMini_ColorFormat = vcod[1];
	DATAREADFROMFILE(&maptiles, 4);		// Number of map tiles
	maptiles = Endian32(maptiles);
	DATAREADFROMFILE(&mapoffset, 4);	// Map offset in tiles
	mapoffset = Endian32(mapoffset);
	DATAREADFROMFILE(reserved, 16);		// Reserved area

	// PM ROM Max is 2MB, that's 256K Map Tiles
	if (maptiles > 262144) return 0;
	if (mapoffset > 262144) return 0;

	if (PokeMini_ColorFormat == 1) bytespertile = 8;
	else bytespertile = 2;

	// Free existing color information
	PokeMini_FreeColorInfo();
	PRCColorOffset = 0;
	PRCColorTop = (uint8_t *)0;

	// Create and load map
	PRCColorMap = (uint8_t *)malloc(maptiles * bytespertile);
	for (i=0; i<maptiles * bytespertile; i++) PRCColorMap[i] = 0x00;
	readbytes = stream(PRCColorMap, maptiles * bytespertile, stream_ptr);

	// Setup offset and top
	PRCColorOffset = mapoffset * bytespertile;
	PRCColorTop = (uint8_t *)PRCColorMap + (maptiles * bytespertile);

	// Version 0.4.5 have old color palette, remap colors to the new one
	if (!(PRCColorFlags & 1)) {
		for (i=0; i<maptiles * bytespertile; i++) {
			PRCColorMap[i] = RemapMINC10_11[PRCColorMap[i] & 15] | (PRCColorMap[i] & 0xF0);
		}
	}

	return (readbytes > 0);
}

// Internal: Stream from file
int PokeMini_StreamFromFile(void *data, int size, void *ptr)
{
	return fread(data, 1, size, (FILE *)ptr);
}

// Load color information from file, MIN must be loaded first
int PokeMini_LoadColorFile(const char *filename)
{
	FILE *fi;
	int res;

	// Open file
	fi = fopen(filename, "rb");
	if (fi == NULL) return 0;		// Silently exit

	// Read color information
	res = PokeMini_LoadColorStream(PokeMini_StreamFromFile, (void *)fi);

	// Done
	fclose(fi);

	// Callback
	if (PokeMini_OnLoadColorFile) PokeMini_OnLoadColorFile(filename, res);

	return res;
}

// Load EEPROM
int PokeMini_LoadEEPROMFile(const char *filename)
{
	FILE *fi;
	int readbytes, success;

	// Custom EEPROM load
	if (PokeMini_CustomLoadEEPROM) {
		success = PokeMini_CustomLoadEEPROM(filename);
		PokeMini_OnLoadEEPROMFile(filename, success);
		return success;
	}

	// Open file
	fi = fopen(filename, "rb");
	if (fi == NULL) {
		if (PokeMini_OnLoadEEPROMFile) PokeMini_OnLoadEEPROMFile(filename, -1);
		return 0;
	}

	// Read content
	readbytes = fread(EEPROM, 1, 8192, fi);
	fclose(fi);

	// Callback
	if (PokeMini_OnLoadEEPROMFile) PokeMini_OnLoadEEPROMFile(filename, (readbytes == 8192) ? 1 : 0);

	return (readbytes == 8192);
}

// Save EEPROM
int PokeMini_SaveEEPROMFile(const char *filename)
{
	FILE *fo;
	int writebytes, success;

	// Custom EEPROM save
	if (PokeMini_CustomSaveEEPROM) {
		success = PokeMini_CustomSaveEEPROM(filename);
		PokeMini_OnSaveEEPROMFile(filename, success);
		return success;
	}

	// Open file
	fo = fopen(filename, "wb");
	if (fo == NULL) {
		if (PokeMini_OnSaveEEPROMFile) PokeMini_OnSaveEEPROMFile(filename, -1);
		return 0;
	}

	// Read content
	writebytes = fwrite(EEPROM, 1, 8192, fo);
	fclose(fo);

	// Callback
	if (PokeMini_OnSaveEEPROMFile) PokeMini_OnSaveEEPROMFile(filename, (writebytes == 8192) ? 1 : 0);

	return (writebytes == 8192);
}

// Load emulator state from memory stream
int PokeMini_LoadSSStream(uint8_t *buffer, uint64_t size)
{
	memstream_t *stream;
	int readbytes;
	char PMiniStr[PMTMPV];
	uint32_t PMiniID, StatTime, BSize;

	// Open memory stream
	memstream_set_buffer(buffer, size);
	stream = memstream_open(0);
	if (stream == NULL) {
		return 0;
	}

	// Read content
	PMiniStr[12] = 0;
	readbytes = memstream_read(stream, PMiniStr, 12);	// Read File ID
	if ((readbytes != 12) || strcmp(PMiniStr, "PokeMiniStat")) {
		memstream_close(stream);
		memstream_set_buffer(NULL, 0);
		return 0;
	}
	readbytes = memstream_read(stream, &PMiniID, 4);	// Read State ID
	if ((readbytes != 4) || (PMiniID != PokeMini_ID)) {
		memstream_close(stream);
		memstream_set_buffer(NULL, 0);
		return 0;
	}
	readbytes = memstream_read(stream, &StatTime, 4);	// Read Time
	if (readbytes != 4) {
		memstream_close(stream);
		memstream_set_buffer(NULL, 0);
		return 0;
	}

	// Read State Structure
	PMiniStr[4] = 0;
	while (memstream_pos(stream) < size) {
		readbytes = memstream_read(stream, PMiniStr, 4);
		if (readbytes != 4) {
			memstream_close(stream);
			memstream_set_buffer(NULL, 0);
			return 0;
		}
		readbytes = memstream_read(stream, &BSize, 4);
		if (readbytes != 4) {
			memstream_close(stream);
			memstream_set_buffer(NULL, 0);
			return 0;
		}
		if (!strcmp(PMiniStr, "RAM-")) {		// RAM
			readbytes = memstream_read(stream, PM_RAM, 0x1000);
			if ((BSize != 0x1000) || (readbytes != 0x1000)) {
				memstream_close(stream);
				memstream_set_buffer(NULL, 0);
				return 0;
			}
		} else if (!strcmp(PMiniStr, "REG-")) {		// Register I/O
			readbytes = memstream_read(stream, PM_IO, 256);
			if ((BSize != 256) || (readbytes != 256)) {
				memstream_close(stream);
				memstream_set_buffer(NULL, 0);
				return 0;
			}
		} else if (!strcmp(PMiniStr, "CPU-")) {		// CPU
			if (!MinxCPU_LoadStateStream(stream, BSize)) {
				memstream_close(stream);
				memstream_set_buffer(NULL, 0);
				return 0;
			}
		} else if (!strcmp(PMiniStr, "IRQ-")) {		// IRQ
			if (!MinxIRQ_LoadStateStream(stream, BSize)) {
				memstream_close(stream);
				memstream_set_buffer(NULL, 0);
				return 0;
			}
		} else if (!strcmp(PMiniStr, "TMR-")) {		// Timers
			if (!MinxTimers_LoadStateStream(stream, BSize)) {
				memstream_close(stream);
				memstream_set_buffer(NULL, 0);
				return 0;
			}
		} else if (!strcmp(PMiniStr, "PIO-")) {		// Parallel IO
			if (!MinxIO_LoadStateStream(stream, BSize)) {
				memstream_close(stream);
				memstream_set_buffer(NULL, 0);
				return 0;
			}
		} else if (!strcmp(PMiniStr, "PRC-")) {		// PRC
			if (!MinxPRC_LoadStateStream(stream, BSize)) {
				memstream_close(stream);
				memstream_set_buffer(NULL, 0);
				return 0;
			}
		} else if (!strcmp(PMiniStr, "CPM-")) {		// Color PRC
			if (!MinxColorPRC_LoadStateStream(stream, BSize)) {
				memstream_close(stream);
				memstream_set_buffer(NULL, 0);
				return 0;
			}
		} else if (!strcmp(PMiniStr, "LCD-")) {		// LCD
			if (!MinxLCD_LoadStateStream(stream, BSize)) {
				memstream_close(stream);
				memstream_set_buffer(NULL, 0);
				return 0;
			}
		} else if (!strcmp(PMiniStr, "LCD-")) {		// Audio
			if (!MinxAudio_LoadStateStream(stream, BSize)) {
				memstream_close(stream);
				memstream_set_buffer(NULL, 0);
				return 0;
			}
		} else if (!strcmp(PMiniStr, "END-")) {
			break;
		}
	}
	memstream_close(stream);
	memstream_set_buffer(NULL, 0);

	// Update RTC if requested
	if (CommandLine.updatertc == 1) {
		MinxTimers.SecTimerCnt += (uint32_t)time(NULL) - StatTime;
	}

	// Syncronize with host time
	PokeMini_SyncHostTime();

	return 1;
}

// Save emulator state to memory stream
int PokeMini_SaveSSStream(uint8_t *buffer, uint64_t size)
{
	// TODO: Error check bytes written
	
	memstream_t *stream;
	uint32_t PMiniID, StatTime, BSize;

	// Open memory stream
	memstream_set_buffer(buffer, size);
	stream = memstream_open(1);
	if (stream == NULL) {
		return 0;
	}

	// Write content
	memstream_write(stream, (void *)"PokeMiniStat", 12);	// Write File ID
	PMiniID = PokeMini_ID;
	memstream_write(stream, &PMiniID, 4);	// Write State ID
	StatTime = (uint32_t)time(NULL);
	memstream_write(stream, &StatTime, 4);	// Write Time

	// Read State Structure
	// - RAM
	memstream_write(stream, (void *)"RAM-", 4);
	BSize = 0x1000;
	memstream_write(stream, &BSize, 4);
	memstream_write(stream, PM_RAM, 0x1000);
	// - Registers I/O
	memstream_write(stream, (void *)"REG-", 4);
	BSize = 256;
	memstream_write(stream, &BSize, 4);
	memstream_write(stream, PM_IO, 256);
	// - CPU Interface
	memstream_write(stream, (void *)"CPU-", 4);
	MinxCPU_SaveStateStream(stream);
	// - IRQ Interface
	memstream_write(stream, (void *)"IRQ-", 4);
	MinxIRQ_SaveStateStream(stream);
	// - Timers Interface
	memstream_write(stream, (void *)"TMR-", 4);
	MinxTimers_SaveStateStream(stream);
	// - Parallel IO Interface
	memstream_write(stream, (void *)"PIO-", 4);
	MinxIO_SaveStateStream(stream);
	// - PRC Interface
	memstream_write(stream, (void *)"PRC-", 4);
	MinxPRC_SaveStateStream(stream);
	// - Color PRC Interface
	memstream_write(stream, (void *)"CPM-", 4);
	MinxColorPRC_SaveStateStream(stream);
	// - LCD Interface
	memstream_write(stream, (void *)"LCD-", 4);
	MinxLCD_SaveStateStream(stream);
	// - Audio Interface
	memstream_write(stream, (void *)"AUD-", 4);
	MinxAudio_SaveStateStream(stream);
	// - EOF
	memstream_write(stream, (void *)"END-", 4);
	BSize = 0;
	memstream_write(stream, &BSize, 4);
	memstream_close(stream);
	memstream_set_buffer(NULL, 0);

	return 1;
}

#ifndef NO_ZIP
// Internal: Stream from ZIP
static int PokeMini_StreamFromZIP(void *data, int size, void *ptr)
{
	return unzReadCurrentFile((unzFile)ptr, data, size);
}

// Load compressed MIN ROM
static int PokeMini_iLoadROMZip(const char *zipfile, int *colorloaded)
{
	unzFile uf = NULL;
	unz_global_info64 gi;
	unz_file_info64 file_inf;
	char filein[PMTMPV];
	void *new_data;
	int i, size, loaded = 0, cloaded = 0;

	if (colorloaded) *colorloaded = 0;

	// Open ZIP
	uf = unzOpen(zipfile);
	if (!uf) {
		if (PokeMini_OnUnzipError) PokeMini_OnUnzipError(zipfile, "Opening ZIP error");
		return 0;
	}
	if (unzGetGlobalInfo64(uf, &gi) != UNZ_OK) {
		if (PokeMini_OnUnzipError) PokeMini_OnUnzipError(zipfile, "Getting global info");
		unzClose(uf);
		return 0;
	}

	// Find and load MIN
	for (i=0; i<gi.number_entry; i++) {
		if (unzGetCurrentFileInfo64(uf, &file_inf, filein, PMTMPV, NULL, 0, NULL, 0) != UNZ_OK) {
			if (PokeMini_OnUnzipError) PokeMini_OnUnzipError(zipfile, "Current file info");
			unzClose(uf);
			return 0;
		}
		if (ExtensionCheck(filein, ".min") && (!loaded)) {
			if (unzLocateFile(uf, filein, 0) != UNZ_OK) {
				if (PokeMini_OnUnzipError) PokeMini_OnUnzipError(zipfile, "LocateFile failed");
				unzClose(uf);
				return 0;
			}
			if (unzOpenCurrentFile(uf) != UNZ_OK) {
				if (PokeMini_OnUnzipError) PokeMini_OnUnzipError(zipfile, "Opening file error");
				unzClose(uf);
				return 0;
			}
			size = GetMultiple2(file_inf.uncompressed_size);
			new_data = (void *)malloc(size);
			memset(new_data, 0xFF, size);
			if (!new_data) {
				if (PokeMini_OnUnzipError) PokeMini_OnUnzipError(zipfile, "Not enough memory");
				unzCloseCurrentFile(uf);
				unzClose(uf);
				return 0;
			}
			size = unzReadCurrentFile(uf, new_data, file_inf.uncompressed_size);
			if (size < 0) {
				if (PokeMini_OnUnzipError) PokeMini_OnUnzipError(zipfile, "Reading file error");
				unzCloseCurrentFile(uf);
				unzClose(uf);
				return 0;
			}
			PokeMini_FreeColorInfo();	// Free existing color information
			PokeMini_SetMINMem((uint8_t *)new_data, file_inf.uncompressed_size);
			PM_ROM_Alloc = 1;
			if (unzCloseCurrentFile(uf) != UNZ_OK) {
				if (PokeMini_OnUnzipError) PokeMini_OnUnzipError(zipfile, "Closing file error");
				unzClose(uf);
				return 0;
			}
			if (PokeMini_OnAllocMIN) PokeMini_OnAllocMIN(PM_ROM_Size, 1);
			if (PokeMini_OnLoadMINFile) PokeMini_OnLoadMINFile(zipfile, 1);
			loaded = 1;
			break;
		}
		if ((i+1) < gi.number_entry) {
			if (unzGoToNextFile(uf) != UNZ_OK) {
				if (PokeMini_OnUnzipError) PokeMini_OnUnzipError(zipfile, "No next file");
				unzClose(uf);
				return 0;
			}
		}
	}

	// Check if there's color information file
	strcat(filein, "c");
	if (!loaded) {
		if (PokeMini_OnUnzipError) PokeMini_OnUnzipError(zipfile, "No ROM in ZIP");
		unzClose(uf);
		return 0;
	} else {
		if (unzGoToFirstFile(uf) != UNZ_OK) {
			if (PokeMini_OnUnzipError) PokeMini_OnUnzipError(zipfile, "No first file");
			unzClose(uf);
			return 0;
		}
		if (unzLocateFile(uf, filein, 0) == UNZ_OK) {
			if (unzOpenCurrentFile(uf) != UNZ_OK) {
				if (PokeMini_OnUnzipError) PokeMini_OnUnzipError(zipfile, "Opening file error");
				unzClose(uf);
				return 0;
			}
			if (!PokeMini_LoadColorStream(PokeMini_StreamFromZIP, uf)) {
				if (PokeMini_OnUnzipError) PokeMini_OnUnzipError(zipfile, "Reading file error");
				unzCloseCurrentFile(uf);
				unzClose(uf);
				return 0;
			}
			if (unzCloseCurrentFile(uf) != UNZ_OK) {
				if (PokeMini_OnUnzipError) PokeMini_OnUnzipError(zipfile, "Closing file error");
				unzClose(uf);
				return 0;
			}
			cloaded = 1;
			if (colorloaded) *colorloaded = 1;
			if (PokeMini_OnLoadColorFile) PokeMini_OnLoadColorFile(zipfile, 1);
		}
	}

	// Close ZIP
	unzClose(uf);

	// Try to load color information from a file outside if wasn't found in the zip
	if (!cloaded) {
		// Filename based of the ROM inside the zip
		RemoveExtension(filein);
		strcat(filein, ".minc");
		if (FileExist(filein)) {
			if (PokeMini_LoadColorFile(filein)) {
				if (colorloaded) *colorloaded = 1;
				if (PokeMini_OnLoadColorFile) PokeMini_OnLoadColorFile(zipfile, 1);
				return 1;
			}
		}
		// Filename based of the zip
		strcpy(filein, zipfile);
		RemoveExtension(filein);
		strcat(filein, ".minc");
		if (FileExist(filein)) {
			if (PokeMini_LoadColorFile(filein)) {
				if (colorloaded) *colorloaded = 1;
				if (PokeMini_OnLoadColorFile) PokeMini_OnLoadColorFile(zipfile, 1);
				return 1;
			}
		}
	}

	return 1;
}
#endif

// Use default callbacks messages
void PokeMini_OnUnzipError_Def(const char *zipfile, const char *reason)
{
	PokeDPrint(POKEMSG_ERR, "Error decompressing %s: %s\n", zipfile, reason);
}
void PokeMini_OnLoadBIOSFile_Def(const char *filename, int success)
{
	if (success == 1) PokeDPrint(POKEMSG_OUT, "BIOS '%s' loaded\n", filename);
	else if (success == -1) PokeDPrint(POKEMSG_ERR, "Error loading BIOS '%s': file not found\nUsing FreeBIOS\n", filename);
	else PokeDPrint(POKEMSG_ERR, "Error loading BIOS '%s': read error\nUsing FreeBIOS\n", filename);
}
void PokeMini_OnLoadMINFile_Def(const char *filename, int success)
{
	if (success == 1) PokeDPrint(POKEMSG_OUT, "ROM '%s' loaded\n", filename);
	else if (success == -1) PokeDPrint(POKEMSG_ERR, "Error loading ROM '%s': file not found\n", filename);
	else if (success == -2) PokeDPrint(POKEMSG_ERR, "Error loading ROM '%s': invalid size\n", filename);
	else PokeDPrint(POKEMSG_ERR, "Error loading ROM '%s', read error\n", filename);
}
void PokeMini_OnLoadColorFile_Def(const char *filename, int success)
{
	if (success == 1) PokeDPrint(POKEMSG_OUT, "Color info '%s' loaded\n", filename);
	else if (success == -1) PokeDPrint(POKEMSG_ERR, "Error loading color info '%s': file not found\n", filename);
	else PokeDPrint(POKEMSG_ERR, "Error loading color info '%s': read error\n", filename);
}
void PokeMini_OnLoadEEPROMFile_Def(const char *filename, int success)
{
	if (success == 1) PokeDPrint(POKEMSG_OUT, "EEPROM '%s' loaded\n", filename);
	else if (success == -1) PokeDPrint(POKEMSG_ERR, "Error loading EEPROM '%s': file not found\n", filename);
	else if (success == -2) PokeDPrint(POKEMSG_ERR, "Error saving EEPROM '%s': device not found\n", filename);
	else PokeDPrint(POKEMSG_ERR, "Error loading EEPROM '%s': read error\n", filename);
}
void PokeMini_OnSaveEEPROMFile_Def(const char *filename, int success)
{
	if (success == 1) PokeDPrint(POKEMSG_OUT, "EEPROM '%s' saved\n", filename);
	else if (success == -1) PokeDPrint(POKEMSG_ERR, "Error saving EEPROM '%s': filename invalid\n", filename);
	else if (success == -2) PokeDPrint(POKEMSG_ERR, "Error saving EEPROM '%s': device not found\n", filename);
	else PokeDPrint(POKEMSG_ERR, "Error saving EEPROM '%s': write error\n", filename);
}
void PokeMini_OnReset_Def(int hardreset)
{
}
void PokeMini_UseDefaultCallbacks()
{
	PokeMini_OnAllocMIN = NULL;
	PokeMini_OnUnzipError = PokeMini_OnUnzipError_Def;
	PokeMini_OnLoadBIOSFile = PokeMini_OnLoadBIOSFile_Def;
	PokeMini_OnLoadMINFile = PokeMini_OnLoadMINFile_Def;
	PokeMini_OnLoadColorFile = PokeMini_OnLoadColorFile_Def;
	PokeMini_OnLoadEEPROMFile = PokeMini_OnLoadEEPROMFile_Def;
	PokeMini_OnSaveEEPROMFile = PokeMini_OnSaveEEPROMFile_Def;
	PokeMini_OnReset = PokeMini_OnReset_Def;
}

// Reset emulation
void PokeMini_Reset(int hardreset)
{
	char tmp[PMTMPV];

	// Reset IO
	if (hardreset) {
		memset(PM_RAM, 0xFF, 8192);
		memcpy(PM_IO, PM_IO_INIT, 256);
	}

	// Reset all components
	MinxTimers_Reset(hardreset);
	MinxIRQ_Reset(hardreset);
	MinxIO_Reset(hardreset);
	MinxPRC_Reset(hardreset);
	MinxColorPRC_Reset(hardreset);
	MinxLCD_Reset(hardreset);
	MinxAudio_Reset(hardreset);
	MinxCPU_Reset(hardreset);

	// Change BIOS
	if (!PokeMini_FreeBIOS && CommandLine.forcefreebios) {
		PokeMini_LoadFreeBIOS();
	}
	if (PokeMini_FreeBIOS && !CommandLine.forcefreebios) {
		PokeMini_LoadFreeBIOS();
		if (StringIsSet(CommandLine.bios_file)) {
			if (FileExist(CommandLine.bios_file)) PokeMini_LoadBIOSFile(CommandLine.bios_file);
			else {
				PokeMini_GetCustomDir(tmp, PMTMPV);
				PokeMini_GotoExecDir();
				if (FileExist(CommandLine.bios_file)) PokeMini_LoadBIOSFile(CommandLine.bios_file);
				PokeMini_GotoCustomDir(tmp);
			}
		}
	}

	// Syncronize with host time
	PokeMini_SyncHostTime();

#ifndef PERFORMANCE
	// Set multicart type
	SetMulticart(CommandLine.multicart);
#endif

	// Callback
	if (PokeMini_OnReset) PokeMini_OnReset(hardreset);
}
