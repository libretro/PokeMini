#include <stdio.h>
#include <stdint.h>
#ifndef _MSC_VER
#include <stdbool.h>
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "libretro.h"
#include "retro_miscellaneous.h"

//pokemini headers
#include "PokeMini.h"
#include "Hardware.h"
#include "Joystick.h"
#include "UI.h"
#include "Video_x3.h"
#include "PokeMini_BG3.h"

#define MAKEBTNMAP(btn,pkebtn) JoystickButtonsEvent((pkebtn), input_cb(0/*port*/, RETRO_DEVICE_JOYPAD, 0, (btn)) != 0)

// Sound buffer size
#define SOUNDBUFFER	2048
#define PMSOUNDBUFF	(SOUNDBUFFER*2)

uint16_t screenbuff [320*240*2];
int PixPitch = 320;//screen->pitch / 2;
int ScOffP = (24 * 320/*PixPitch*/) + 16;

// Platform menu (REQUIRED >= 0.4.4)
int UIItems_PlatformC(int index, int reason);
TUIMenu_Item UIItems_Platform[] = {
   PLATFORMDEF_GOBACK,
   //{ 0,  9, "Define Joystick...", UIItems_PlatformC },
   PLATFORMDEF_SAVEOPTIONS,
   PLATFORMDEF_END(UIItems_PlatformC)
};
int UIItems_PlatformC(int index, int reason)
{
   if (reason == UIMENU_OK) reason = UIMENU_RIGHT;
   if (reason == UIMENU_CANCEL) UIMenu_PrevMenu();
   if (reason == UIMENU_RIGHT) {
      if (index == 9) JoystickEnterMenu();
   }
   return 1;
}

// Enable / Disable sound
void enablesound(int sound)
{
   MinxAudio_ChangeEngine(sound);
}

static retro_log_printf_t log_cb = NULL;
static retro_video_refresh_t video_cb = NULL;
static retro_input_poll_t poll_cb = NULL;
static retro_input_state_t input_cb = NULL;
static retro_audio_sample_batch_t audio_batch_cb = NULL;
static retro_environment_t environ_cb = NULL;

void handlekeyevents(){
   MAKEBTNMAP(RETRO_DEVICE_ID_JOYPAD_SELECT,0);
   MAKEBTNMAP(RETRO_DEVICE_ID_JOYPAD_START,1);
   MAKEBTNMAP(RETRO_DEVICE_ID_JOYPAD_UP,2);
   MAKEBTNMAP(RETRO_DEVICE_ID_JOYPAD_DOWN,3);
   MAKEBTNMAP(RETRO_DEVICE_ID_JOYPAD_LEFT,4);
   MAKEBTNMAP(RETRO_DEVICE_ID_JOYPAD_RIGHT,5);
   MAKEBTNMAP(RETRO_DEVICE_ID_JOYPAD_A,6);
   MAKEBTNMAP(RETRO_DEVICE_ID_JOYPAD_B,7);
   MAKEBTNMAP(RETRO_DEVICE_ID_JOYPAD_X,8);
   MAKEBTNMAP(RETRO_DEVICE_ID_JOYPAD_Y,9);
   MAKEBTNMAP(RETRO_DEVICE_ID_JOYPAD_L,10);
   MAKEBTNMAP(RETRO_DEVICE_ID_JOYPAD_R,11);
}

void *retro_get_memory_data(unsigned type)
{
   return NULL;
}

size_t retro_get_memory_size(unsigned type)
{
   return 0;
}

unsigned retro_api_version(void)
{
   return RETRO_API_VERSION;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
   video_cb = cb;
}

void retro_set_audio_sample(retro_audio_sample_t cb){}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
   audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb)
{
   poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb)
{
   input_cb = cb;
}

void retro_set_environment(retro_environment_t cb)
{
   environ_cb = cb;
   
   struct retro_log_callback logging;
   if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging))
      log_cb = logging.log;
   else
      log_cb = NULL;
   
   bool no_rom = true;
   environ_cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_rom);
}

void retro_get_system_info(struct retro_system_info *info)
{
   info->need_fullpath    = true;
   info->valid_extensions = "min";
   info->library_version  = "v0.60";
   info->library_name     = "PokeMini";
   info->block_extract    = false;
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
   info->geometry.base_width = 320;
   info->geometry.base_height = 240;
   info->geometry.max_width = 320;
   info->geometry.max_height = 240;
   info->geometry.aspect_ratio = 4.0 / 3.0;
   info->timing.fps = 60;
   info->timing.sample_rate = 44100;
}

void retro_init (void)
{
   struct retro_log_callback log;
   enum retro_pixel_format rgb565;

   rgb565 = RETRO_PIXEL_FORMAT_RGB565;
   if(environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &rgb565) && log_cb)
         log_cb(RETRO_LOG_INFO, "Frontend supports RGB565 - will use that instead of XRGB1555.\n");
   
   
   PokeMini_InitDirs(NULL/*argv[0]*/, NULL);
   CommandLineInit();
   CommandLineConfFile("pokemini.cfg", NULL, NULL);
   
   if (!PokeMini_SetVideo((TPokeMini_VideoSpec *)&PokeMini_Video3x3, 16, CommandLine.lcdfilter, CommandLine.lcdmode)) {
      log_cb(RETRO_LOG_INFO, "Couldn't set video spec\n");
      exit(1);
   }
   
   UIMenu_SetDisplay(288, 192, PokeMini_RGB16, (uint8_t *)PokeMini_BG3, (uint16_t *)PokeMini_BG3_PalRGB16, (uint32_t *)PokeMini_BG3_PalRGB32);
   
   PokeMini_VideoPalette_Init(PokeMini_RGB16, 1);
   PokeMini_VideoPalette_Index(CommandLine.palette, CommandLine.custompal, CommandLine.lcdcontrast, CommandLine.lcdbright);
   PokeMini_ApplyChanges();
   
   PokeMini_UseDefaultCallbacks();
   
   UIMenu_Init();
   enablesound(1);

}

void retro_deinit(void)
{
   enablesound(0);
   UIMenu_Destroy();
   
   // Save Stuff
   PokeMini_SaveFromCommandLines(1);
   
   PokeMini_VideoPalette_Free();
   PokeMini_Destroy();
}

void retro_reset (void)
{

}

void retro_run (void)
{
   static int16_t audiobuffer[44100 + 100];
   
   //bool updated = false;
   //if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
   //check_variables();
   
   poll_cb();
   handlekeyevents();
   
   MinxAudio_GetSamplesS16(audiobuffer, 44100);
   audio_batch_cb(audiobuffer, 44100);
   PokeMini_EmulateFrame();
   // Sleep a little in the hope to free a few samples
   //while (MinxAudio_SyncWithAudio()) retro_sleep(1);

   
   if (PokeMini_Rumbling) {
      PokeMini_VideoBlit((uint16_t *)screenbuff + ScOffP + PokeMini_GenRumbleOffset(PixPitch), PixPitch);
   } else {
      PokeMini_VideoBlit((uint16_t *)screenbuff + ScOffP, PixPitch);
   }
   
}

size_t retro_serialize_size (void)
{

}

bool retro_serialize(void *data, size_t size)
{

}

bool retro_unserialize(const void * data, size_t size)
{

}

void retro_cheat_reset(void)
{

} 

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{

} 

bool retro_load_game(const struct retro_game_info *game)
{

}

bool retro_load_game_special(
  unsigned game_type,
  const struct retro_game_info *info, size_t num_info
)
{
   return false;
}

void retro_unload_game (void)
{ }

unsigned retro_get_region (void)
{ 
   return RETRO_REGION_NTSC;
}

