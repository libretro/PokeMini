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
/*
 bare functions to use
 int PokeMini_EmulateFrame(void);
 int PokeMini_Create(int flags, int soundfifo);
 void PokeMini_Destroy();
 int PokeMini_NewMIN(uint32_t size);
 int PokeMini_LoadMINFile(const char *filename);
 */


//pokemini headers
#include "PokeMini.h"
#include "Hardware.h"
#include "Joystick.h"
#include "UI.h"
//#include "Video_x4.h"
#include "Video_x3.h"
#include "PokeMini_BG3.h"

#define MAKEBTNMAP(btn,pkebtn) JoystickButtonsEvent((pkebtn), input_cb(0/*port*/, RETRO_DEVICE_JOYPAD, 0, (btn)) != 0)

// Sound buffer size
#define SOUNDBUFFER	2048
#define PMSOUNDBUFF	(SOUNDBUFFER*2)

uint16_t screenbuff [320*240];
int PixPitch = 320;//screen->pitch / 2;
int ScOffP = (24 * 320/*PixPitch*/) + 16;

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
   if (reason == UIMENU_OK) reason = UIMENU_RIGHT;
   if (reason == UIMENU_CANCEL) UIMenu_PrevMenu();
   if (reason == UIMENU_RIGHT) {
      if (index == 9) JoystickEnterMenu();
   }
   return 1;
}

// Load MIN ROM
int PokeMini_LoadMINFileXPLATFORM(size_t size, uint8_t* buffer)
{

   // Check if size is valid
   if ((size <= 0x2100) || (size > 0x200000)) {
      return 0;
   }
   
   // Free existing color information
   PokeMini_FreeColorInfo();
   
   // Allocate ROM and set cartridge size
   if (!PokeMini_NewMIN(size)) {
      return 0;
   }
   
   // Read content
   memcpy(PM_ROM,buffer,size);
   
   NewMulticart();
   
   return 1;
}

static retro_log_printf_t log_cb = NULL;
static retro_video_refresh_t video_cb = NULL;
static retro_input_poll_t poll_cb = NULL;
static retro_input_state_t input_cb = NULL;
static retro_audio_sample_t audio_cb = NULL;
//static retro_audio_sample_batch_t audio_batch_cb = NULL;
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

void retro_set_video_refresh(retro_video_refresh_t cb)
{
   video_cb = cb;
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{
   audio_cb = cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
   //audio_batch_cb = cb;
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
   struct retro_log_callback logging;
   
   environ_cb = cb;
   
   if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging))
      log_cb = logging.log;
   else
      log_cb = NULL;
}

void retro_get_system_info(struct retro_system_info *info)
{
   memset(info, 0, sizeof(*info));
   info->need_fullpath    = false;
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
   info->geometry.aspect_ratio = 4 / 3;
   info->timing.fps = 72;
   info->timing.sample_rate = 44100;
}

void retro_init (void)
{
   enum retro_pixel_format rgb565;
   

   rgb565 = RETRO_PIXEL_FORMAT_RGB565;
   if(environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &rgb565) && log_cb)
         log_cb(RETRO_LOG_INFO, "Frontend supports RGB565 - will use that instead of XRGB1555.\n");

}

void retro_deinit(void)
{
   PokeMini_VideoPalette_Free();
   PokeMini_Destroy();
}

void retro_reset (void)
{
   PokeMini_Reset(1 /*hardreset*/);
}

void retro_run (void)
{
   
   poll_cb();
   handlekeyevents();
   
   PokeMini_EmulateFrame();
   
   /*
   uint16_t audiosamples;
   static int16_t audiobuffer[44100 + 100];
   audiosamples = MinxAudio_SamplesInBuffer();
   MinxAudio_GetSamplesS16(audiobuffer, audiosamples);
   audio_batch_cb(audiobuffer, audiosamples);
   */
   
   static int16_t audiobuffer[612];
   uint16_t audiosamples = 612;// MinxAudio_SamplesInBuffer();
   MinxAudio_GetSamplesS16(audiobuffer, audiosamples);
   int i;
   for(i=0;i<612;i++)
      audio_cb(audiobuffer[i],audiobuffer[i]);
   
   if (PokeMini_Rumbling) {
      PokeMini_VideoBlit((uint16_t *)screenbuff + ScOffP + PokeMini_GenRumbleOffset(PixPitch), PixPitch);
   } else {
      PokeMini_VideoBlit((uint16_t *)screenbuff + ScOffP, PixPitch);
   }
   
   video_cb(screenbuff, 320, 240, 320*2/*Pitch*/);
}

size_t retro_serialize_size (void)
{
   return 0;
}

bool retro_serialize(void *data, size_t size)
{
   return false;
}

bool retro_unserialize(const void * data, size_t size)
{
   return false;
}

void retro_cheat_reset(void)
{
   //no cheats on this core
} 

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
   //no cheats on this core
} 

bool retro_load_game(const struct retro_game_info *game)
{
   int passed;
   int userdefinedindex = 0;//make user defined later
   
   CommandLineInit();
   CommandLine.joyenabled = 1;
   
   //add LCDMODE_COLORS option
   // Set video spec and check if is supported
   if (!PokeMini_SetVideo((TPokeMini_VideoSpec *)&PokeMini_Video3x3, 16, 0/*lcdfilter*//*0=none*/, LCDMODE_ANALOG/*lcdmode*/)) {
      printf("Couldn't set video spec\n");
      abort();
   }
   
   passed = PokeMini_Create(0/*flags*/, PMSOUNDBUFF);//returns 1 on completion,0 on error
   if(!passed)abort();
   
   PokeMini_VideoPalette_Init(PokeMini_RGB16, 1/*enablehighcolor*/);
   PokeMini_VideoPalette_Index(userdefinedindex, NULL /*CustomMonoPal*/, 0/*int contrastboost*/, 0/*int brightoffset*/);
   PokeMini_ApplyChanges();
   
   PokeMini_UseDefaultCallbacks();
   
   MinxAudio_ChangeEngine(MINX_AUDIO_GENERATED);//enable sound
   
   passed = PokeMini_LoadMINFileXPLATFORM(game->size,(uint8_t*)game->data);//returns 1 on completion,0 on error
   if(!passed)abort();
   
   PokeMini_Reset(1 /*hardreset*/);
   return true;
}

bool retro_load_game_special(
  unsigned game_type,
  const struct retro_game_info *info, size_t num_info
)
{
   return false;
}

void retro_unload_game (void){}


//useless callbacks

unsigned retro_api_version(void)
{
   return RETRO_API_VERSION;
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
   (void)port;
   (void)device;
}

unsigned retro_get_region (void)
{ 
   return RETRO_REGION_NTSC;
}

