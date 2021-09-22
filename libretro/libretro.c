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
#include <libretro_core_options.h>
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
#include "Video_x1.h"
#include "Video_x2.h"
#include "Video_x3.h"
#include "Video_x4.h"
#include "Video_x5.h"
#include "Video_x6.h"

#ifdef _3DS
void* linearMemAlign(size_t size, size_t alignment);
void linearFree(void* mem);
#endif

// Sound buffer size
#define SOUNDBUFFER	2048
#define PMSOUNDBUFF	(SOUNDBUFFER*2)

// Save state size
#define PM_SS_SIZE 44014

// Screen parameters
#define PM_SCEEN_WIDTH  96
#define PM_SCEEN_HEIGHT 64

static uint16_t video_scale = 1;
static uint16_t video_width = PM_SCEEN_WIDTH;
static uint16_t video_height = PM_SCEEN_HEIGHT;

static uint16_t *video_buffer = NULL;

// > In the original standalone code, 'pixel pitch' is defined as
//      (SDL_Surface->pitch / 2)
//   SDL_Surface->pitch is just the length of a row of pixels in bytes,
//   so divide by 2 and we get the screen width in pixels...
static int pix_pitch = PM_SCEEN_WIDTH;

// File path utils
static char g_basename[PMTMPV];
static char *g_system_dir;
static char *g_save_dir;

static retro_log_printf_t log_cb = NULL;
static retro_video_refresh_t video_cb = NULL;
static retro_input_poll_t poll_cb = NULL;
static retro_input_state_t input_cb = NULL;
static retro_audio_sample_t audio_cb = NULL;
static retro_audio_sample_batch_t audio_batch_cb = NULL;
static retro_environment_t environ_cb = NULL;

static bool libretro_supports_bitmasks = false;

// Force feedback parameters
static struct retro_rumble_interface rumble = {0};
static uint16_t rumble_strength      = 0;
static uint16_t rumble_strength_prev = 0;

// Low pass audio filter parameters
static bool low_pass_enabled  = false;
static int32_t low_pass_range = 0;
static int32_t low_pass_prev  = 0; /* Previous sample */

// Turbo button parameters
// > Default turbo pulse train is 9 frames ON, 9 frames OFF
// > At 72 Hz, corresponds to 4 presses per second
#define DEVICE_ID_TURBO_A         RETRO_DEVICE_ID_JOYPAD_X
#define TURBO_PERIOD_MIN          4
#define TURBO_PERIOD_MAX          120
#define TURBO_PERIOD_DEFAULT      18
#define TURBO_PULSE_WIDTH_MIN     2
#define TURBO_PULSE_WIDTH_MAX     18
#define TURBO_PULSE_WIDTH_DEFAULT 9

static uint16_t turbo_period      = TURBO_PERIOD_DEFAULT;
static uint16_t turbo_pulse_width = TURBO_PULSE_WIDTH_DEFAULT;
static uint16_t turbo_counter     = 0;

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
		{ 0, RETRO_DEVICE_JOYPAD, 0, DEVICE_ID_TURBO_A,             "Turbo A" },
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
		if (log_cb)
			log_cb(RETRO_LOG_INFO, "Rumble environment supported\n");
	}
	else if (log_cb)
		log_cb(RETRO_LOG_INFO, "Rumble environment not supported\n");
}

///////////////////////////////////////////////////////////

static void ActivateControllerRumble(void)
{
	if (!rumble.set_rumble_state ||
		 (rumble_strength_prev == rumble_strength))
		return;

	rumble.set_rumble_state(0, RETRO_RUMBLE_WEAK,   rumble_strength);
	rumble.set_rumble_state(0, RETRO_RUMBLE_STRONG, rumble_strength);
	rumble_strength_prev = rumble_strength;
}

///////////////////////////////////////////////////////////

static void DeactivateControllerRumble(void)
{
	if (!rumble.set_rumble_state ||
		 (rumble_strength_prev == 0))
		return;

	rumble.set_rumble_state(0, RETRO_RUMBLE_WEAK,   0);
	rumble.set_rumble_state(0, RETRO_RUMBLE_STRONG, 0);
	rumble_strength_prev = 0;
}

///////////////////////////////////////////////////////////

static void extract_basename(char *buf, const char *path, size_t size)
{
   char *ext        = NULL;
	const char *base = strrchr(path, '/');
	if (!base)
		base = strrchr(path, '\\');
	if (!base)
		base = path;
	
	if (*base == '\\' || *base == '/')
		base++;
	
	strncpy(buf, base, size - 1);
	buf[size - 1] = '\0';
	
	ext = strrchr(buf, '.');
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
	
	// pokemini_screen_shake_lv
	CommandLine.rumblelvl = 3; // (0 - 3; 3 == Default)
	variables.key = "pokemini_screen_shake_lv";
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &variables))
	{
		CommandLine.rumblelvl = atoi(variables.value);
	}
	
	// NB: The following parameters are not part of the 'CommandLine'
	// interface, but there is no better place to handle them...
	
	// pokemini_lowpass_filter
	low_pass_enabled = false;
	variables.key = "pokemini_lowpass_filter";
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &variables))
	{
		if (strcmp(variables.value, "enabled") == 0)
		{
			low_pass_enabled = true;
		}
	}
	
	// pokemini_lowpass_range
	low_pass_range = (60 * 65536) / 100;
	variables.key = "pokemini_lowpass_range";
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &variables))
	{
		low_pass_range = (strtol(variables.value, NULL, 10) * 65536) / 100;
	}
	
	// pokemini_rumble_lv
	rumble_strength = 0;
	variables.key = "pokemini_rumble_lv";
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &variables))
	{
		rumble_strength = atoi(variables.value);
		rumble_strength = (rumble_strength > 10) ? 10 : rumble_strength;
		rumble_strength = (rumble_strength > 0)  ? ((0x1999 * rumble_strength) + 0x5) : 0;
	}
	
	if (rumble_strength == 0)
	{
		DeactivateControllerRumble();
	}

	// pokemini_turbo_period
	turbo_period      = TURBO_PERIOD_DEFAULT;
	turbo_pulse_width = TURBO_PULSE_WIDTH_DEFAULT;
	variables.key     = "pokemini_turbo_period";
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &variables))
	{
		turbo_period = atoi(variables.value);
		turbo_period = (turbo_period < TURBO_PERIOD_MIN) ?
				TURBO_PERIOD_MIN : turbo_period;
		turbo_period = (turbo_period > TURBO_PERIOD_MAX) ?
				TURBO_PERIOD_MAX : turbo_period;

		turbo_pulse_width = turbo_period >> 1;
		turbo_pulse_width = (turbo_pulse_width < TURBO_PULSE_WIDTH_MIN) ?
				TURBO_PULSE_WIDTH_MIN : turbo_pulse_width;
		turbo_pulse_width = (turbo_pulse_width > TURBO_PULSE_WIDTH_MAX) ?
				TURBO_PULSE_WIDTH_MAX : turbo_pulse_width;

		turbo_counter = 0;
	}
}

///////////////////////////////////////////////////////////

// Want this code to be 'minimally invasive'
// -> Will make use of existing PokeMini command line interface
//    wherever possible
static void InitialiseCommandLine(const struct retro_game_info *game)
{
	char slash;
#if defined(_3DS)
	uint8_t device_model = 0xFF;
#endif

	// Mandatory (?)
	CommandLineInit();
	
	// Set fixed overrides (i.e. these values will never change...)
	CommandLine.forcefreebios = 0; // OFF
	CommandLine.eeprom_share = 0;  // OFF (there is no practical benefit to a shared eeprom save
	                               //      - it just gets full and becomes a nuisance...)
	CommandLine.updatertc = 2;	    // Update RTC (0=Off, 1=State, 2=Host)
	CommandLine.joyenabled = 1;    // ON
	
#if defined(_3DS)
	// 3DS has limited performance...
	// > Lower emulation accuracy if required (o3DS/o2DS)
	CFGU_GetSystemModel(&device_model); /* (0 = O3DS, 1 = O3DSXL, 2 = N3DS, 3 = 2DS, 4 = N3DSXL, 5 = N2DSXL) */
	if (device_model == 2 || device_model == 4 || device_model == 5) {
		CommandLine.synccycles = 8;
	} else {
		CommandLine.synccycles = 16;
	}
	// > Reduce sound quality
	CommandLine.sound = MINX_AUDIO_GENERATED;
#elif defined(RS90)
	// RS-90 devices running OpenDingux appear
	// to have similar performance to o3DS
	// > Lower emulation accuracy and reduce
	//   sound quality
	CommandLine.synccycles = 16;
	CommandLine.sound = MINX_AUDIO_GENERATED;
#elif defined(DINGUX)
	// Other OpenDingux platforms appear to have
	// similar performance to n3DS when running
	// this core
	// > Use default 'accurate' emulation,
	//   but reduce sound quality
	CommandLine.synccycles = 8;
	CommandLine.sound = MINX_AUDIO_GENERATED;
#else
	CommandLine.synccycles = 8; // Default 'accurate' setting
	CommandLine.sound = MINX_AUDIO_DIRECTPWM;
#endif
	
	// Set overrides read from core options
	SyncCoreOptionsWithCommandLine();
	
	// Set file paths
	// > Handle Windows nonsense...
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

typedef enum
{
	PM_BUTTON_POWER = 9,
	PM_BUTTON_UP    = 10,
	PM_BUTTON_DOWN  = 11,
	PM_BUTTON_LEFT  = 4,
	PM_BUTTON_RIGHT = 5,
	PM_BUTTON_A     = 1,
	PM_BUTTON_B     = 2,
	PM_BUTTON_SHAKE = 6,
	PM_BUTTON_C     = 7
} pm_button_type;

typedef struct
{
   unsigned retropad;
   pm_button_type pokemini;
} pm_button_map;

static const pm_button_map btn_map[] = {
	{ RETRO_DEVICE_ID_JOYPAD_SELECT, PM_BUTTON_POWER },
	{ RETRO_DEVICE_ID_JOYPAD_UP,     PM_BUTTON_UP    },
	{ RETRO_DEVICE_ID_JOYPAD_DOWN,   PM_BUTTON_DOWN  },
	{ RETRO_DEVICE_ID_JOYPAD_LEFT,   PM_BUTTON_LEFT  },
	{ RETRO_DEVICE_ID_JOYPAD_RIGHT,  PM_BUTTON_RIGHT },
	{ RETRO_DEVICE_ID_JOYPAD_A,      PM_BUTTON_A     },
	{ RETRO_DEVICE_ID_JOYPAD_B,      PM_BUTTON_B     },
	{ RETRO_DEVICE_ID_JOYPAD_L,      PM_BUTTON_SHAKE },
	{ RETRO_DEVICE_ID_JOYPAD_R,      PM_BUTTON_C     }
};

static void handlekeyevents(void)
{
	unsigned i;
	bool a_pressed       = false;
	bool turbo_a_pressed = false;
	bool a_down          = false;

	if (libretro_supports_bitmasks)
	{
		int16_t ret = input_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_MASK);

		for (i = 0; i < sizeof(btn_map) / sizeof(pm_button_map); i++)
		{
			if (btn_map[i].pokemini == PM_BUTTON_A)
				a_pressed = (ret & (1 << btn_map[i].retropad)) != 0;
			else
				JoystickButtonsEvent(
						btn_map[i].pokemini,
						(ret & (1 << btn_map[i].retropad)) != 0);

			turbo_a_pressed = (ret & (1 << DEVICE_ID_TURBO_A)) != 0;
		}
	}
	else
	{
		for (i = 0; i < sizeof(btn_map) / sizeof(pm_button_map); i++)
		{
			if (btn_map[i].pokemini == PM_BUTTON_A)
				a_pressed = input_cb(0, RETRO_DEVICE_JOYPAD, 0, btn_map[i].retropad) != 0;
			else
				JoystickButtonsEvent(
						btn_map[i].pokemini,
						input_cb(0, RETRO_DEVICE_JOYPAD, 0, btn_map[i].retropad) != 0);

			turbo_a_pressed = input_cb(0, RETRO_DEVICE_JOYPAD, 0, DEVICE_ID_TURBO_A) != 0;
		}
	}

	/* Handle A/Turbo A input */
	if (turbo_a_pressed)
	{
		a_down = a_pressed || (turbo_counter < turbo_pulse_width);

		turbo_counter++;
		if (turbo_counter >= turbo_period)
			turbo_counter = 0;
	}
	else
	{
		a_down        = a_pressed;
		turbo_counter = 0;
	}

	JoystickButtonsEvent(PM_BUTTON_A, a_down);
}

///////////////////////////////////////////////////////////

static void InitialiseVideo(void)
{
	struct retro_variable variables = {0};
	TPokeMini_VideoSpec *video_spec = NULL;

	// Get video scale
#if defined(_3DS)
   video_scale = 1; // 3DS cannot handle normal default 4x scale...
                    // (o3DS maxes out at 2x scale + Normal2x CPU filter,
                    //  which actually looks great and is probably the
                    //  best way to play)
#else
   video_scale = 4; // Default value: 4x scale
	                 // - Divides well into 768p and 1080p
	                 // - Gives internal LCD filter a pleasing appearance
#endif
	variables.key = "pokemini_video_scale";
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &variables))
	{
		if (strcmp(variables.value, "1x") == 0)
		{
			video_scale = 1;
		}
		else if (strcmp(variables.value, "2x") == 0)
		{
			video_scale = 2;
		}
		else if (strcmp(variables.value, "3x") == 0)
		{
			video_scale = 3;
		}
#if !defined(_3DS)
		else if (strcmp(variables.value, "5x") == 0)
		{
			video_scale = 5;
		}
		else if (strcmp(variables.value, "6x") == 0)
		{
			video_scale = 6;
		}
#endif
	}

	// If video scale is 1x, LCD filter is disabled
	// > hide pokemini_lcdfilter core option
	if (video_scale == 1)
	{
		struct retro_core_option_display option_display;

		option_display.key     = "pokemini_lcdfilter";
		option_display.visible = false;

		environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY, &option_display);
	}
	
	// Determine video dimensions
	video_width = PM_SCEEN_WIDTH * video_scale;
	video_height = PM_SCEEN_HEIGHT * video_scale;
	pix_pitch = video_width;
	
	// Allocate video buffer
	if (!video_buffer)
	{
#ifdef _3DS
		video_buffer = (uint16_t*)linearMemAlign(sizeof(uint16_t) * video_width * video_height, 128);
#else
		video_buffer = (uint16_t*)calloc(video_width * video_height, sizeof(uint16_t));
#endif
	}
	
	// Determine video spec
	switch (video_scale)
	{
		case 2:
			video_spec = (TPokeMini_VideoSpec *)&PokeMini_Video2x2;
			break;
		case 3:
			video_spec = (TPokeMini_VideoSpec *)&PokeMini_Video3x3;
			break;
		case 4:
			video_spec = (TPokeMini_VideoSpec *)&PokeMini_Video4x4;
			break;
		case 5:
			video_spec = (TPokeMini_VideoSpec *)&PokeMini_Video5x5;
			break;
		case 6:
			video_spec = (TPokeMini_VideoSpec *)&PokeMini_Video6x6;
			break;
		default: // video_scale == 1
			video_spec = (TPokeMini_VideoSpec *)&PokeMini_Video1x1;
			break;
	}
	
	// Set video spec and check if supported
	if (!PokeMini_SetVideo(video_spec, 16, CommandLine.lcdfilter, CommandLine.lcdmode))
	{
		if (log_cb)
			log_cb(RETRO_LOG_ERROR, "Couldn't set video spec.\n");
		abort();
	}
}

///////////////////////////////////////////////////////////

// Set specified area of array to zero
static void ZeroArray(uint16_t array[], int size)
{
	int i;
	for (i = 0; i < size; i++)
		array[i] = 0;
}

///////////////////////////////////////////////////////////

// Classic 'reverse' function, from 'Programming Pearls' by Jon Bentley
static void ReverseArray(uint16_t array[], int size)
{
	int i, j;
	for (i = 0, j = size; i < j; i++, j--)
	{
		uint16_t tmp = array[i];
		array[i] = array[j];
		array[j] = tmp;
	}
}

///////////////////////////////////////////////////////////

// Apply screen shake effect
static void SetPixelOffset(void)
{
	if (CommandLine.rumblelvl)
	{
		int row_offset = PokeMini_GenRumbleOffset(pix_pitch) * video_scale;
		int buffer_size = video_width * video_height;
		
		// Derived from the classic 'rotate array' technique
		// from 'Programming Pearls' by Jon Bentley
		if (row_offset < 0)
		{
			row_offset = buffer_size + row_offset;
			ZeroArray(video_buffer, buffer_size - row_offset - 1);
			ReverseArray(video_buffer + buffer_size - row_offset, row_offset - 1);
		}
		else
		{
			ReverseArray(video_buffer, buffer_size - row_offset - 1);
			ZeroArray(video_buffer + buffer_size - row_offset, row_offset - 1);
		}

		ReverseArray(video_buffer, buffer_size - 1);
	}
}

///////////////////////////////////////////////////////////

// Apply low pass audio filter
static void ApplyLowPassFilter(int16_t *buf, int length)
{
   int samples      = length;
   int16_t *out     = buf;

   /* Restore previous sample */
   int32_t low_pass = low_pass_prev;

   /* Single-pole low-pass filter (6 dB/octave) */
   int32_t factor_a = low_pass_range;
   int32_t factor_b = 0x10000 - factor_a;

   do
   {
      /* Apply low-pass filter */
      low_pass = (low_pass * factor_a) + (*out * factor_b);

      /* 16.16 fixed point */
      low_pass >>= 16;

      /* Update sound buffer
       * > Note: Sound is mono, converted to
       *   stereo by duplicating the left/right
       *   channels */
      *out++   = (int16_t)low_pass;
      *out++   = (int16_t)low_pass;
   }
   while (--samples);

   /* Save last sample for next frame */
   low_pass_prev  = low_pass;
}

// Core functions
///////////////////////////////////////////////////////////

void *retro_get_memory_data(unsigned type)
{
	if (type == RETRO_MEMORY_SYSTEM_RAM)
		return PM_RAM;
   return NULL;
}

///////////////////////////////////////////////////////////

size_t retro_get_memory_size(unsigned type)
{
	if (type == RETRO_MEMORY_SYSTEM_RAM)
		return 0x2000;
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
	
	libretro_set_core_options(environ_cb);
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
	info->geometry.base_width   = video_width;
	info->geometry.base_height  = video_height;
	info->geometry.max_width    = video_width;
	info->geometry.max_height   = video_height;
	info->geometry.aspect_ratio = (float)video_width / (float)video_height;
	info->timing.fps            = 72.0;
	info->timing.sample_rate    = 44100.0;
}

///////////////////////////////////////////////////////////

void retro_init (void)
{
	enum retro_pixel_format rgb565 = RETRO_PIXEL_FORMAT_RGB565;

	if(environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &rgb565) && log_cb)
		log_cb(RETRO_LOG_INFO, "Frontend supports RGB565 - will use that instead of XRGB1555.\n");

	libretro_supports_bitmasks = false;
	if (environ_cb(RETRO_ENVIRONMENT_GET_INPUT_BITMASKS, NULL))
		libretro_supports_bitmasks = true;
}

///////////////////////////////////////////////////////////

void retro_deinit(void)
{
	DeactivateControllerRumble();

	memset(&rumble, 0, sizeof(struct retro_rumble_interface));
	rumble_strength      = 0;
	rumble_strength_prev = 0;

	libretro_supports_bitmasks = false;

	low_pass_enabled = false;
	low_pass_range   = 0;
	low_pass_prev    = 0;

	turbo_period      = TURBO_PERIOD_DEFAULT;
	turbo_pulse_width = TURBO_PULSE_WIDTH_DEFAULT;
	turbo_counter     = 0;
}

///////////////////////////////////////////////////////////

void retro_reset (void)
{
	// Soft reset
	PokeMini_Reset(0);

	low_pass_prev = 0;
}

///////////////////////////////////////////////////////////

void retro_run (void)
{
	static int16_t audiobuffer[612 * 2]; // 2 Channels -> MinxAudio_SamplesInBuffer() * 2
	uint16_t audiosamples = 612;
	
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
	
	MinxAudio_GetSamplesS16Ch(audiobuffer, audiosamples, 2);
	if (low_pass_enabled)
	{
		/* Note: We could be more efficient here by embedding
		 * the low pass filter in MinxAudio_GetSamplesS16Ch().
		 * This would be deeply invasive, however, requiring
		 * many changes to MinxAudio and the 'command line'
		 * interface. Since the performance overheads of the
		 * ApplyLowPassFilter() function are very low, the
		 * additional workload does not seem worthwhile... */
		ApplyLowPassFilter(audiobuffer, audiosamples);
	}
	audio_batch_cb(audiobuffer, audiosamples);
	
	PokeMini_VideoBlit((uint16_t *)video_buffer, pix_pitch);
	
	if (PokeMini_Rumbling)
	{
		SetPixelOffset();
		ActivateControllerRumble();
	}
	else
	{
		DeactivateControllerRumble();
	}
	
	LCDDirty = 0;
	
	video_cb(video_buffer, video_width, video_height, video_width * 2/*Pitch*/);
}

///////////////////////////////////////////////////////////

size_t retro_serialize_size(void)
{
	// Save states have a fixed size...
	return PM_SS_SIZE;
}

///////////////////////////////////////////////////////////

bool retro_serialize(void *data, size_t size)
{
	if (PokeMini_SaveSSStream((uint8_t*)data, size)) {
		if (log_cb) log_cb(RETRO_LOG_INFO, "State saved successfully.\n");
	} else {
		if (log_cb) log_cb(RETRO_LOG_ERROR, "Failed to save state.\n");
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////

bool retro_unserialize(const void *data, size_t size)
{
	if (PokeMini_LoadSSStream((uint8_t*)data, size)) {
		if (log_cb) log_cb(RETRO_LOG_INFO, "Save state loaded successfully.\n");
	} else {
		if (log_cb) log_cb(RETRO_LOG_ERROR, "Failed to load save state.\n");
		return false;
	}
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
	InitialiseVideo();
	
	passed = PokeMini_Create(0/*flags*/, PMSOUNDBUFF); // returns 1 on completion,0 on error
	if (!passed)
		abort();
	
#if (defined(_3DS) || defined(DINGUX))
   PokeMini_VideoPalette_Init(PokeMini_BGR16, 0/* disable high colour*/);
#else
   PokeMini_VideoPalette_Init(PokeMini_BGR16, 1/* enable high colour */);
#endif
	
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

static void SimulatePowerOff(void)
{
	// In order for some games to correctly update
	// their EEPROM data, the virtual PokemonMini
	// console must be 'powered off' before unloading
	// content. This routine simulates a power off event
	// by holding the power button until the CPU enters
	// a 'halt' state. Note that we wait for a maximum of
	// 72 emulated frames (1 virtual second of runtime)
	// to avoid hanging the emulator in the event of an
	// unforeseen error
	const size_t frames_max = 72;
	size_t frame_counter    = 0;

	while ((MinxCPU.Status != MINX_STATUS_HALT) &&
			 (frame_counter < frames_max))
	{
		// Hold power button
		JoystickButtonsEvent(9, 1);

		// Emulate frame
		PokeMini_EmulateFrame();

		frame_counter++;
	}

	// Log an error if halt state was not achieved
	if (MinxCPU.Status != MINX_STATUS_HALT)
		if (log_cb) log_cb(RETRO_LOG_ERROR, "Failed to power off virtual Pokemon Mini console.\n");
}

///////////////////////////////////////////////////////////

void retro_unload_game(void)
{
	// Power off console
	SimulatePowerOff();
	
	// Save EEPROM
	if (PokeMini_EEPROMWritten && StringIsSet(CommandLine.eeprom_file))
	{
		PokeMini_EEPROMWritten = 0;
		PokeMini_SaveEEPROMFile(CommandLine.eeprom_file);
		if (log_cb)
			log_cb(RETRO_LOG_INFO, "Wrote EEPROM file: %s\n", CommandLine.eeprom_file);
	}
	
	// Terminate emulator
	PokeMini_VideoPalette_Free();
	PokeMini_Destroy();
	
	// Deallocate video buffer
	if (video_buffer)
	{
#ifdef _3DS
		linearFree(video_buffer);
#else
		free(video_buffer);
#endif
	}
	video_buffer = NULL;
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
