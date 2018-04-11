#include <stdio.h>
#include <stdint.h>
#include <boolean.h>
#ifdef _MSC_VER
#include <direct.h>
#else
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include <libretro.h>
#include <retro_miscellaneous.h>
/*
 bare functions to use
 int PokeMini_EmulateFrame(void);
 int PokeMini_Create(int flags, int soundfifo);
 void PokeMini_Destroy();
 int PokeMini_NewMIN(uint32_t size);
 int PokeMini_LoadMINFile(const char *filename);
 */

// PokeMini headers
#include "MinxIO.h"
#include "PMCommon.h"
#include "PokeMini.h"
#include "Hardware.h"
#include "Joystick.h"
#include "MinxAudio.h"
#include "UI.h"
#include "Video.h"
#include "Video_x4.h"

#define MAKEBTNMAP(btn,pkebtn) JoystickButtonsEvent((pkebtn), input_cb(0/*port*/, RETRO_DEVICE_JOYPAD, 0, (btn)) != 0)

// Sound buffer size
#define SOUNDBUFFER	2048
#define PMSOUNDBUFF	(SOUNDBUFFER*2)

// Save state size
#define PM_SS_SIZE 44142

// Screen dimension definitions
#define PM_SCEEN_WIDTH  96
#define PM_SCEEN_HEIGHT 64
#define PM_VIDEO_SCALE  4
#define PM_VIDEO_WIDTH  (PM_SCEEN_WIDTH * PM_VIDEO_SCALE)
#define PM_VIDEO_HEIGHT (PM_SCEEN_HEIGHT * PM_VIDEO_SCALE)

uint16_t screenbuff [PM_VIDEO_WIDTH * PM_VIDEO_HEIGHT];
int PixPitch = PM_VIDEO_WIDTH; // screen->pitch / 2;

// File path utils
static char g_basename[256];
static char *g_system_dir;
static char *g_save_dir;

// Platform menu (REQUIRED >= 0.4.4)
int UIItems_PlatformC(int index, int reason);
TUIMenu_Item UIItems_Platform[] = {
   PLATFORMDEF_GOBACK,
   { 0,  9, "Define Joystick...", UIItems_PlatformC },
   PLATFORMDEF_SAVEOPTIONS,
   PLATFORMDEF_END(UIItems_PlatformC)
};
int UIItems_PlatformC(int index, int reason)
{
   if (reason == UIMENU_OK)
      reason = UIMENU_RIGHT;
   if (reason == UIMENU_CANCEL)
      UIMenu_PrevMenu();
   if (reason == UIMENU_RIGHT)
   {
      if (index == 9)
         JoystickEnterMenu();
   }
   return 1;
}

static retro_log_printf_t log_cb = NULL;
static retro_video_refresh_t video_cb = NULL;
static retro_input_poll_t poll_cb = NULL;
static retro_input_state_t input_cb = NULL;
static retro_audio_sample_t audio_cb = NULL;
static retro_audio_sample_batch_t audio_batch_cb = NULL;
static retro_environment_t environ_cb = NULL;

// Force feedback stuff
static struct retro_rumble_interface rumble;
static bool rumble_supported = false;
static uint16_t rumble_strength = 0;

// Utilities
///////////////////////////////////////////////////////////

static void InitialiseInputDescriptors(void)
{
	struct retro_input_descriptor desc[] = {
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,   "D-Pad Left" },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,     "D-Pad Up" },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,   "D-Pad Down" },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT,  "D-Pad Right" },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,      "B" },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,      "A" },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,      "Shake" },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,      "C" },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Power" },
		{ 0 },
	};
	
	environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);
}

///////////////////////////////////////////////////////////

static void InitialiseRumbleInterface(void)
{
	if (environ_cb(RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE, &rumble))
	{
		rumble_supported = true;
		if (log_cb)
			log_cb(RETRO_LOG_INFO, "Rumble environment supported\n");
	}
	else
	{
		rumble_supported = false;
		if (log_cb)
			log_cb(RETRO_LOG_INFO, "Rumble environment not supported\n");
	}
}

///////////////////////////////////////////////////////////

static void extract_basename(char *buf, const char *path, size_t size)
{
	const char *base = strrchr(path, '/');
	if (!base)
		base = strrchr(path, '\\');
	if (!base)
		base = path;
	
	if (*base == '\\' || *base == '/')
		base++;
	
	strncpy(buf, base, size - 1);
	buf[size - 1] = '\0';
	
	char *ext = strrchr(buf, '.');
	if (ext)
		*ext = '\0';
}

///////////////////////////////////////////////////////////

static void SyncCoreOptionsWithCommandLine(void)
{
	struct retro_variable variables = {0};
	
	// pokemini_lcdfilter
	CommandLine.lcdfilter = 1; // LCD Filter (0: nofilter, 1: dotmatrix, 2: scanline)
	variables.key = "pokemini_lcdfilter";
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &variables))
	{
		if (strcmp(variables.value, "scanline") == 0)
		{
			CommandLine.lcdfilter = 2;
		}
		else if (strcmp(variables.value, "none") == 0)
		{
			CommandLine.lcdfilter = 0;
		}
	}
	
	// pokemini_lcdmode
	CommandLine.lcdmode = 0; // LCD Mode (0: analog, 1: 3shades, 2: 2shades)
	variables.key = "pokemini_lcdmode";
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &variables))
	{
		if (strcmp(variables.value, "3shades") == 0)
		{
			CommandLine.lcdmode = 1;
		}
		else if (strcmp(variables.value, "2shades") == 0)
		{
			CommandLine.lcdmode = 2;
		}
	}
	
	// pokemini_lcdcontrast
	CommandLine.lcdcontrast = 64; // LCD contrast
	variables.key = "pokemini_lcdcontrast";
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &variables))
	{
		// atoi() stinks like week-old garbage, but since we have a fixed set of
		// strings it should be okay...
		CommandLine.lcdcontrast = atoi(variables.value);
	}
	
	// pokemini_lcdbright
	CommandLine.lcdbright = 0; // LCD brightness offset
	variables.key = "pokemini_lcdbright";
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &variables))
	{
		CommandLine.lcdbright = atoi(variables.value);
	}
	
	// pokemini_palette
	CommandLine.palette = 0; // Palette Index (0 - 13; 0 == Default)
	variables.key = "pokemini_palette";
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &variables))
	{
		if (strcmp(variables.value, "Old") == 0)
		{
			CommandLine.palette = 1;
		}
		else if (strcmp(variables.value, "Monochrome") == 0)
		{
			CommandLine.palette = 2;
		}
		else if (strcmp(variables.value, "Green") == 0)
		{
			CommandLine.palette = 3;
		}
		else if (strcmp(variables.value, "Green Vector") == 0)
		{
			CommandLine.palette = 4;
		}
		else if (strcmp(variables.value, "Red") == 0)
		{
			CommandLine.palette = 5;
		}
		else if (strcmp(variables.value, "Red Vector") == 0)
		{
			CommandLine.palette = 6;
		}
		else if (strcmp(variables.value, "Blue LCD") == 0)
		{
			CommandLine.palette = 7;
		}
		else if (strcmp(variables.value, "LEDBacklight") == 0)
		{
			CommandLine.palette = 8;
		}
		else if (strcmp(variables.value, "Girl Power") == 0)
		{
			CommandLine.palette = 9;
		}
		else if (strcmp(variables.value, "Blue") == 0)
		{
			CommandLine.palette = 10;
		}
		else if (strcmp(variables.value, "Blue Vector") == 0)
		{
			CommandLine.palette = 11;
		}
		else if (strcmp(variables.value, "Sepia") == 0)
		{
			CommandLine.palette = 12;
		}
		else if (strcmp(variables.value, "Monochrome Vector") == 0)
		{
			CommandLine.palette = 13;
		}
	}
	
	// pokemini_piezofilter
	CommandLine.piezofilter = 1; // ON
	variables.key = "pokemini_piezofilter";
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &variables))
	{
		if (strcmp(variables.value, "disabled") == 0)
		{
			CommandLine.piezofilter = 0;
		}
	}
	
	// pokemini_rumblelvl
	CommandLine.rumblelvl = 3; // (0 - 3; 3 == Default)
	variables.key = "pokemini_rumblelvl";
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &variables))
	{
		CommandLine.rumblelvl = atoi(variables.value);
	}
	
	// pokemini_controller_rumble
	// NB: This is not part of the 'CommandLine' interface, but there is
	// no better place to handle it...
	bool rumble_enabled = true;
	variables.key = "pokemini_controller_rumble";
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &variables))
	{
		if (strcmp(variables.value, "disabled") == 0)
		{
			rumble_enabled = false;
		}
	}
	
	// Determine rumble strength
	if (rumble_enabled)
	{
		rumble_strength = 21845 * CommandLine.rumblelvl;
	}
	else
	{
		rumble_strength = 0;
	}
}

///////////////////////////////////////////////////////////

// Want this code to be 'minimally invasive'
// -> Will make use of existing PokeMini command line interface
//    wherever possible
static void InitialiseCommandLine(const struct retro_game_info *game)
{
	// Mandatory (?)
	CommandLineInit();
	
	// Set fixed overrides (i.e. these values will never change...)
	CommandLine.forcefreebios = 0; // OFF
	CommandLine.eeprom_share = 0;  // OFF (there is no practical benefit to a shared eeprom save
	                               //      - it just gets full and becomes a nuisance...)
	CommandLine.updatertc = 2;	    // Update RTC (0=Off, 1=State, 2=Host)
	CommandLine.sound = MINX_AUDIO_DIRECTPWM;
	CommandLine.joyenabled = 1;    // ON
	
	// Set overrides read from core options
	SyncCoreOptionsWithCommandLine();
	
	// Set file paths
	// > Handle Windows nonsense...
	char slash;
#if defined(_WIN32)
	slash = '\\';
#else
	slash = '/';
#endif
   // > Prep. work
	if (!environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &g_system_dir))
	{
		if (log_cb)
			log_cb(RETRO_LOG_ERROR, "Could not find system directory.\n");
	}
	if (!environ_cb(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY, &g_save_dir))
	{
		if (log_cb)
			log_cb(RETRO_LOG_ERROR, "Could not find save directory.\n");
	}
   // > ROM path
	if (game->path != NULL)
	{
		// >> Set CommandLine.min_file
		sprintf(CommandLine.min_file, "%s", game->path);
		// >> Set CommandLine.eeprom_file
		extract_basename(g_basename, game->path, sizeof(g_basename));
		sprintf(CommandLine.eeprom_file, "%s%c%s.eep", g_save_dir, slash, g_basename);
	}
	// > BIOS path
	// >> Set CommandLine.bios_file
	sprintf(CommandLine.bios_file, "%s%cbios.min", g_system_dir, slash);
}

///////////////////////////////////////////////////////////

// Load MIN ROM
static int PokeMini_LoadMINFileXPLATFORM(size_t size, uint8_t* buffer)
{
	// Check if size is valid
	if ((size <= 0x2100) || (size > 0x200000))
		return 0;
	
	// Free existing color information
	PokeMini_FreeColorInfo();
	
	// Allocate ROM and set cartridge size
	if (!PokeMini_NewMIN(size))
		return 0;
	
	// Read content
	memcpy(PM_ROM,buffer,size);
	
	NewMulticart();
	
	return 1;
}

///////////////////////////////////////////////////////////

void handlekeyevents(void)
{
	MAKEBTNMAP(RETRO_DEVICE_ID_JOYPAD_SELECT,  9);
	MAKEBTNMAP(RETRO_DEVICE_ID_JOYPAD_UP,     10);
	MAKEBTNMAP(RETRO_DEVICE_ID_JOYPAD_DOWN,   11);
	MAKEBTNMAP(RETRO_DEVICE_ID_JOYPAD_LEFT,    4);
	MAKEBTNMAP(RETRO_DEVICE_ID_JOYPAD_RIGHT,   5);
	MAKEBTNMAP(RETRO_DEVICE_ID_JOYPAD_A,       1);
	MAKEBTNMAP(RETRO_DEVICE_ID_JOYPAD_B,       2);
	MAKEBTNMAP(RETRO_DEVICE_ID_JOYPAD_L,       6);
	MAKEBTNMAP(RETRO_DEVICE_ID_JOYPAD_R,       7);
}

///////////////////////////////////////////////////////////

void ActivateControllerRumble(void)
{
	if (rumble_supported)
	{
		rumble.set_rumble_state(0, RETRO_RUMBLE_WEAK, rumble_strength);
		rumble.set_rumble_state(0, RETRO_RUMBLE_STRONG, rumble_strength);
	}
}

///////////////////////////////////////////////////////////

void DeactivateControllerRumble(void)
{
	if (rumble_supported)
	{
		rumble.set_rumble_state(0, RETRO_RUMBLE_WEAK, 0);
		rumble.set_rumble_state(0, RETRO_RUMBLE_STRONG, 0);
	}
}

///////////////////////////////////////////////////////////

static void GetTempFileName(char *name)
{
	char slash;
#if defined(_WIN32)
	slash = '\\';
#else
	slash = '/';
#endif
	
   sprintf(name, "%s%cpokemini_ss.tmp", g_save_dir, slash);
}

// Core functions
///////////////////////////////////////////////////////////

void *retro_get_memory_data(unsigned type)
{
	if (type == RETRO_MEMORY_SYSTEM_RAM)
		return PM_RAM;
	else
		return NULL;
}

///////////////////////////////////////////////////////////

size_t retro_get_memory_size(unsigned type)
{
	if (type == RETRO_MEMORY_SYSTEM_RAM)
		return 0x2000;
	else
		return 0;
}

///////////////////////////////////////////////////////////

void retro_set_video_refresh(retro_video_refresh_t cb)
{
	video_cb = cb;
}

///////////////////////////////////////////////////////////

void retro_set_audio_sample(retro_audio_sample_t cb)
{
	audio_cb = cb;
}

///////////////////////////////////////////////////////////

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
	audio_batch_cb = cb;
}

///////////////////////////////////////////////////////////

void retro_set_input_poll(retro_input_poll_t cb)
{
	poll_cb = cb;
}

///////////////////////////////////////////////////////////

void retro_set_input_state(retro_input_state_t cb)
{
	input_cb = cb;
}

///////////////////////////////////////////////////////////

void retro_set_environment(retro_environment_t cb)
{
	struct retro_log_callback logging;
	
	environ_cb = cb;
	
	if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging))
		log_cb = logging.log;
	else
		log_cb = NULL;
	
	// Core options
	struct retro_variable variables[] = {
		{ "pokemini_lcdfilter", "LCD Filter; dotmatrix|scanline|none" },
		{ "pokemini_lcdmode", "LCD Mode; analog|3shades|2shades" },
		{ "pokemini_lcdcontrast", "LCD Contrast; 64|0|16|32|48|80|96" },
		{ "pokemini_lcdbright", "LCD Brightness; 0|-80|-60|-40|-20|20|40|60|80" },
		{ "pokemini_palette", "Palette; Default|Old|Monochrome|Green|Green Vector|Red|Red Vector|Blue LCD|LEDBacklight|Girl Power|Blue|Blue Vector|Sepia|Monochrome Vector" },
		{ "pokemini_piezofilter", "Piezo Filter; enabled|disabled" },
		{ "pokemini_rumblelvl", "Rumble Level (Screen + Controller); 3|2|1|0" },
		{ "pokemini_controller_rumble", "Controller Rumble; enabled|disabled" },
		{ NULL, NULL },
	};
	
	environ_cb(RETRO_ENVIRONMENT_SET_VARIABLES, variables);
}

///////////////////////////////////////////////////////////

void retro_get_system_info(struct retro_system_info *info)
{
	memset(info, 0, sizeof(*info));
	info->need_fullpath    = false;
	info->valid_extensions = "min";
	info->library_version  = "v0.60";
	info->library_name     = "PokeMini";
	info->block_extract    = false;
}

///////////////////////////////////////////////////////////

void retro_get_system_av_info(struct retro_system_av_info *info)
{
	info->geometry.base_width   = PM_VIDEO_WIDTH;
	info->geometry.base_height  = PM_VIDEO_HEIGHT;
	info->geometry.max_width    = PM_VIDEO_WIDTH;
	info->geometry.max_height   = PM_VIDEO_HEIGHT;
	info->geometry.aspect_ratio = (float)PM_VIDEO_WIDTH / (float)PM_VIDEO_HEIGHT;
	info->timing.fps            = 72.0;
	info->timing.sample_rate    = 44100.0;
}

///////////////////////////////////////////////////////////

void retro_init (void)
{
	enum retro_pixel_format rgb565 = RETRO_PIXEL_FORMAT_RGB565;
	if(environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &rgb565) && log_cb)
		log_cb(RETRO_LOG_INFO, "Frontend supports RGB565 - will use that instead of XRGB1555.\n");
}

///////////////////////////////////////////////////////////

void retro_deinit(void)
{
	PokeMini_VideoPalette_Free();
	PokeMini_Destroy();
}

///////////////////////////////////////////////////////////

void retro_reset (void)
{
	// Soft reset
	PokeMini_Reset(0);
}

///////////////////////////////////////////////////////////

void retro_run (void)
{
	int i;
	int offset = 0;
	static int16_t audiobuffer[612];
	static int16_t audiostretched[612 * 2];
	uint16_t audiosamples = 612;// MinxAudio_SamplesInBuffer();
	
	// Check for core options updates
	bool options_updated = false;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &options_updated) && options_updated)
   {
		SyncCoreOptionsWithCommandLine();
		PokeMini_VideoPalette_Index(CommandLine.palette, NULL, CommandLine.lcdcontrast, CommandLine.lcdbright);
		PokeMini_ApplyChanges();
	}
	
	poll_cb();
	handlekeyevents();
	
	PokeMini_EmulateFrame();
	
	MinxAudio_GetSamplesS16(audiobuffer, audiosamples);
	for(i = 0;i < 612;i++)
	{
		audiostretched[offset]     = audiobuffer[i];
		audiostretched[offset + 1] = audiobuffer[i];
		offset += 2;
	}
	audio_batch_cb(audiostretched, audiosamples);
	
	if (PokeMini_Rumbling)
	{
		PokeMini_VideoBlit((uint16_t *)screenbuff + PokeMini_GenRumbleOffset(PixPitch), PixPitch);
		ActivateControllerRumble();
	}
	else
	{
		PokeMini_VideoBlit((uint16_t *)screenbuff, PixPitch);
		DeactivateControllerRumble();
	}
	
	video_cb(screenbuff, PM_VIDEO_WIDTH, PM_VIDEO_HEIGHT, PM_VIDEO_WIDTH * 2/*Pitch*/);
}

///////////////////////////////////////////////////////////

size_t retro_serialize_size(void)
{
	// Save states have a fixed size of 44142 bytes...
	return PM_SS_SIZE;
}

///////////////////////////////////////////////////////////

bool retro_serialize(void *data, size_t size)
{
	// Okay, this is really bad...
	// I don't know how to portably create a memory stream in C that can be
	// written to like a FILE (open_memstream() is Linux only...), and I don't
	// want to have to re-write everything inside PokeMini_SaveSSFile()...
	// So we're going to do this the crappy way...
	// - Write state to a temporary file
	// - Copy contents of temporary file to *data
	// Maybe someone else can do this properly...
	
	// Get temporary file name
	char temp_file_name[256];
	GetTempFileName(temp_file_name);
	
	// Write state into temporary file
	if (PokeMini_SaveSSFile(temp_file_name, CommandLine.min_file))
	{
		if (log_cb) log_cb(RETRO_LOG_INFO, "Wrote temporary state file %s\n", temp_file_name);
	}
	else
	{
		if (log_cb) log_cb(RETRO_LOG_ERROR, "Failed to write temporary state file.\n");
		remove(temp_file_name); // Just in case...
		return false;
	}
	
	// Read contents of temporary file into *data...
	long int file_length;
	FILE *file = fopen(temp_file_name, "rb");
	if (file)
	{
		fseek(file, 0, SEEK_END);
		file_length = ftell(file);
		fseek(file, 0, SEEK_SET);
		if (size >= file_length)
		{
			fread((char*)data, sizeof(char), file_length, file);
			fclose(file);
			if (log_cb) log_cb(RETRO_LOG_INFO, "Save state created successfully.\n");
		}
		else
		{
			if (log_cb) log_cb(RETRO_LOG_ERROR, "Size mismatch between temporary state file and serialisation buffer...\n");
			fclose(file);
			remove(temp_file_name);
			return false;
		}
	}
	else
	{
		if (log_cb) log_cb(RETRO_LOG_ERROR, "Failed to open temporary state file for reading.\n");
		remove(temp_file_name);
		return false;
	}
	
	// Remove temporary file
	remove(temp_file_name);
	
	return true;
}

///////////////////////////////////////////////////////////

bool retro_unserialize(const void *data, size_t size)
{
	// Same issue here as in retro_serialize()...
	// Maybe someone else can do this properly...
	
	// Get temporary file name
	char temp_file_name[256];
	GetTempFileName(temp_file_name);
	
	// Write contents of *data to temporary file...
	FILE *file = fopen(temp_file_name, "wb");
	if (file)
	{
		size_t write_length = fwrite((char*)data, sizeof(char), size, file);
		if (write_length != size)
		{
			if (log_cb) log_cb(RETRO_LOG_ERROR, "Failed to write temporary state file.\n");
			fclose(file);
			remove(temp_file_name);
			return false;
		}
		fclose(file);
	}
	else
	{
		if (log_cb) log_cb(RETRO_LOG_ERROR, "Failed to open temporary state file.\n");
		return false;
	}
	
	// Read state from temporary file
	if (PokeMini_LoadSSFile(temp_file_name))
	{
		if (log_cb) log_cb(RETRO_LOG_INFO, "Save state loaded successfully.\n");
	}
	else
	{
		if (log_cb) log_cb(RETRO_LOG_ERROR, "Failed to load temporary state file.\n");
		remove(temp_file_name);
		return false;
	}
	
	// Remove temporary file
	remove(temp_file_name);
	
	return true;
}

///////////////////////////////////////////////////////////

void retro_cheat_reset(void)
{
	// no cheats on this core
} 

///////////////////////////////////////////////////////////

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
	// no cheats on this core
} 

///////////////////////////////////////////////////////////

bool retro_load_game(const struct retro_game_info *game)
{
	int passed;
	
	if (!game)
		return false;
	
	InitialiseInputDescriptors();
	InitialiseRumbleInterface();
	InitialiseCommandLine(game);
	
	//add LCDMODE_COLORS option
	// Set video spec and check if is supported
	if (!PokeMini_SetVideo((TPokeMini_VideoSpec *)&PokeMini_Video4x4, 16, CommandLine.lcdfilter, CommandLine.lcdmode))
	{
		if (log_cb)
			log_cb(RETRO_LOG_ERROR, "Couldn't set video spec.\n");
		abort();
	}
	
	passed = PokeMini_Create(0/*flags*/, PMSOUNDBUFF); // returns 1 on completion,0 on error
	if (!passed)
		abort();
	
	PokeMini_VideoPalette_Init(PokeMini_BGR16, 1/*enablehighcolor*/);
	PokeMini_VideoPalette_Index(CommandLine.palette, NULL, CommandLine.lcdcontrast, CommandLine.lcdbright);
	PokeMini_ApplyChanges(); // Note: 'CommandLine.piezofilter' value is also read inside here
	
	PokeMini_UseDefaultCallbacks();
	
	MinxAudio_ChangeEngine(CommandLine.sound); // enable sound
	
	passed = PokeMini_LoadMINFileXPLATFORM(game->size, (uint8_t*)game->data); // returns 1 on completion,0 on error
	if (!passed)
		abort();
	
	// Load EEPROM
	MinxIO_FormatEEPROM();
	if (FileExist(CommandLine.eeprom_file))
	{
		PokeMini_LoadEEPROMFile(CommandLine.eeprom_file);
		if (log_cb)
			log_cb(RETRO_LOG_INFO, "Read EEPROM file: %s\n", CommandLine.eeprom_file);
	}
	
	// Soft reset
	PokeMini_Reset(0);
	
	return true;
}

///////////////////////////////////////////////////////////

bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info)
{
	return false;
}

///////////////////////////////////////////////////////////

void retro_unload_game(void)
{
	// Save EEPROM
	if (PokeMini_EEPROMWritten && StringIsSet(CommandLine.eeprom_file))
	{
		PokeMini_EEPROMWritten = 0;
		PokeMini_SaveEEPROMFile(CommandLine.eeprom_file);
		if (log_cb)
			log_cb(RETRO_LOG_INFO, "Wrote EEPROM file: %s\n", CommandLine.eeprom_file);
	}
}

// Useless (?) callbacks
///////////////////////////////////////////////////////////

unsigned retro_api_version(void)
{
	return RETRO_API_VERSION;
}

///////////////////////////////////////////////////////////

void retro_set_controller_port_device(unsigned port, unsigned device)
{
	(void)port;
	(void)device;
}

///////////////////////////////////////////////////////////

unsigned retro_get_region (void)
{ 
	return RETRO_REGION_NTSC;
}
