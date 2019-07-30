#ifndef LIBRETRO_CORE_OPTIONS_H__
#define LIBRETRO_CORE_OPTIONS_H__

#include <stdlib.h>
#include <string.h>

#include <libretro.h>
#include <retro_inline.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 ********************************
 * Core Option Definitions
 ********************************
*/

/* RETRO_LANGUAGE_ENGLISH */

/* Default language:
 * - All other languages must include the same keys and values
 * - Will be used as a fallback in the event that frontend language
 *   is not available
 * - Will be used as a fallback for any missing entries in
 *   frontend language definition */

struct retro_core_option_definition option_defs_us[] = {
   {
      "pokemini_video_scale",
      "Video Scale (Restart)",
      "Set internal video scale factor. Increasing the scale factor improves the appearance of the internal 'Dot Matrix' LCD filter.",
      {
         { "1x", NULL },
         { "2x", NULL },
         { "3x", NULL },
#ifndef _3DS
         { "4x", NULL },
         { "5x", NULL },
         { "6x", NULL },
#endif
         { NULL, NULL },
      },
#ifdef _3DS
      "1x"
#else
      "4x"
#endif
   },
   {
      "pokemini_lcdfilter",
      "LCD Filter",
      "Select internal screen filter. 'Dot Matrix' produces an LCD effect that mimics real hardware. LCD filters are disabled when 'Video Scale' is set to '1x'.",
      {
         { "dotmatrix", "Dot Matrix" },
         { "scanline",  "Scanlines" },
         { "none",      "None" },
         { NULL, NULL },
      },
      "dotmatrix"
   },
   {
      "pokemini_lcdmode",
      "LCD Mode",
      "Specify the greyscale 'color' reproduction characteristics of the emulated liquid crystal display. 'Analog' mimics real hardware. '2 Shades' removes ghosting, but causes flickering in most games.",
      {
         { "analog",  "Analog" },
         { "3shades", "3 Shades" },
         { "2shades", "2 Shades" },
         { NULL, NULL },
      },
      "analog"
   },
   {
      "pokemini_lcdcontrast",
      "LCD Contrast",
      "Set contrast level of emulated liquid crystal display.",
      {
         { "0",  NULL },
         { "16", NULL },
         { "32", NULL },
         { "48", NULL },
         { "64", NULL },
         { "80", NULL },
         { "96", NULL },
         { NULL, NULL },
      },
      "64"
   },
   {
      "pokemini_lcdbright",
      "LCD Brightness",
      "Set brightness offset of emulated liquid crystal display.",
      {
         { "-80", NULL },
         { "-60", NULL },
         { "-40", NULL },
         { "-20", NULL },
         { "0",   NULL },
         { "20",  NULL },
         { "40",  NULL },
         { "60",  NULL },
         { "80",  NULL },
         { NULL, NULL },
      },
      "0"
   },
   {
      "pokemini_palette",
      "Palette",
      "Specify palette used to 'colorize' the emulated liquid crystal display. 'Default' mimics real hardware.",
      {
         { "Default",           "Default" },
         { "Old",               "Old" },
         { "Monochrome",        "Black & White" },
         { "Green",             "Green" },
         { "Green Vector",      "Inverted Green" },
         { "Red",               "Red" },
         { "Red Vector",        "Inverted Red" },
         { "Blue LCD",          "Blue LCD" },
         { "LEDBacklight",      "LED Backlight" },
         { "Girl Power",        "Girl Power" },
         { "Blue",              "Blue" },
         { "Blue Vector",       "Inverted Blue" },
         { "Sepia",             "Sepia" },
         { "Monochrome Vector", "Inverted Black & White" },
         { NULL, NULL },
      },
      "Default"
   },
   {
      "pokemini_piezofilter",
      "Piezo Filter",
      "Use an audio filter to simulate the characteristics of the Pokemon Mini's piezoelectric speaker.",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled"
   },
   {
      "pokemini_rumblelvl",
      "Rumble Level (Screen + Controller)",
      "Specify the magnitude of the force feedback effect, both virtual and physical.",
      {
         { "0", NULL },
         { "1", NULL },
         { "2", NULL },
         { "3", NULL },
         { NULL, NULL },
      },
      "3"
   },
   {
      "pokemini_controller_rumble",
      "Controller Rumble",
      "Enable physical force feedback effect via controller rumble.",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled"
   },
   {
      "pokemini_screen_shake",
      "Screen Shake",
      "Enable virtual force feedback effect by 'shaking' the screen.",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled"
   },
   { NULL, NULL, NULL, {{0}}, NULL },
};

/* RETRO_LANGUAGE_JAPANESE */

/* RETRO_LANGUAGE_FRENCH */

struct retro_core_option_definition option_defs_fr[] = {
   {
      "pokemini_video_scale",
      "Échelle vidéo (redémarrer)",
      "Définir le facteur d'échelle vidéo interne. L'augmentation du facteur d'échelle améliore l'apparence du filtre LCD interne «Matrice de Points».",
      {
         { NULL, NULL }, /* Scale factors do not require translation */
      },
      NULL
   },
   {
      "pokemini_lcdfilter",
      "Filtre LCD",
      "Sélectionnez le filtre d'écran interne. «Matrice de Points» produit un effet LCD qui imite le matériel réel. Les filtres LCD sont désactivés lorsque «Échelle vidéo» est défini sur «1x».",
      {
         { "dotmatrix", "Matrice de Points" },
         { "scanline",  "Lignes de Balayage" },
         { "none",      "Aucun" },
         { NULL, NULL },
      },
      NULL
   },
   {
      "pokemini_lcdmode",
      "Mode LCD",
      "Spécifiez les caractéristiques de reproduction 'couleur' en niveaux de gris de l'affichage à cristaux liquides émulé. «Analogique» imite le matériel réel. «2 Nuances» supprime les images fantômes, mais provoque un scintillement dans la plupart des jeux.",
      {
         { "analog",  "Analogique" },
         { "3shades", "3 Nuances" },
         { "2shades", "2 Nuances" },
         { NULL, NULL },
      },
      NULL
   },
   {
      "pokemini_lcdcontrast",
      "Contraste LCD",
      "Réglez le niveau de contraste de l'écran à cristaux liquides émulé.",
      {
         { NULL, NULL }, /* Numbers do not require translation */
      },
      NULL
   },
   {
      "pokemini_lcdbright",
      "Luminosité de l'écran LCD",
      "Définissez le décalage de luminosité de l'affichage à cristaux liquides émulé.",
      {
         { NULL, NULL }, /* Numbers do not require translation */
      },
      NULL
   },
   {
      "pokemini_palette",
      "Palette",
      "Spécifiez la palette utilisée pour 'coloriser' l'affichage à cristaux liquides émulé. «Défaut» imite le matériel réel.",
      {
         { "Default",           "Défaut" },
         { "Old",               "Vieux" },
         { "Monochrome",        "Noir et Blanc" },
         { "Green",             "Vert" },
         { "Green Vector",      "Vert Inversé" },
         { "Red",               "Rouge" },
         { "Red Vector",        "Rouge Inversé" },
         { "Blue LCD",          "LCD Bleu" },
         { "LEDBacklight",      "Rétro-éclairage LED" },
         { "Girl Power",        "Pouvoir des Filles" },
         { "Blue",              "Bleu" },
         { "Blue Vector",       "Bleu Inversé" },
         { "Sepia",             "Sépia" },
         { "Monochrome Vector", "Noir et Blanc Inversé" },
         { NULL, NULL },
      },
      NULL
   },
   {
      "pokemini_piezofilter",
      "Filtre Piézo",
      "Utilisez un filtre audio pour simuler les caractéristiques du haut-parleur piézoélectrique du Pokemon Mini.",
      {
         { NULL, NULL }, /* enabled/disabled strings do not require translation */
      },
      NULL
   },
   {
      "pokemini_rumblelvl",
      "Niveau de Rumble (écran + contrôleur)",
      "Spécifiez l'ampleur de l'effet de retour de force, à la fois virtuel et physique.",
      {
         { NULL, NULL }, /* Numbers do not require translation */
      },
      NULL
   },
   {
      "pokemini_controller_rumble",
      "Contrôleur Rumble",
      "Activer l'effet de retour de force physique via le roulement du contrôleur.",
      {
         { NULL, NULL }, /* enabled/disabled strings do not require translation */
      },
      NULL
   },
   {
      "pokemini_screen_shake",
      "Secousse de l'écran",
      "Activez l'effet de retour de force virtuel en 'secouant' l'écran.",
      {
         { NULL, NULL }, /* enabled/disabled strings do not require translation */
      },
      NULL
   },
   { NULL, NULL, NULL, {{0}}, NULL },
};

/* RETRO_LANGUAGE_SPANISH */

/* RETRO_LANGUAGE_GERMAN */

/* RETRO_LANGUAGE_ITALIAN */

/* RETRO_LANGUAGE_DUTCH */

/* RETRO_LANGUAGE_PORTUGUESE_BRAZIL */

/* RETRO_LANGUAGE_PORTUGUESE_PORTUGAL */

/* RETRO_LANGUAGE_RUSSIAN */

/* RETRO_LANGUAGE_KOREAN */

/* RETRO_LANGUAGE_CHINESE_TRADITIONAL */

/* RETRO_LANGUAGE_CHINESE_SIMPLIFIED */

/* RETRO_LANGUAGE_ESPERANTO */

/* RETRO_LANGUAGE_POLISH */

/* RETRO_LANGUAGE_VIETNAMESE */

/* RETRO_LANGUAGE_ARABIC */

/* RETRO_LANGUAGE_GREEK */

/* RETRO_LANGUAGE_TURKISH */

struct retro_core_option_definition option_defs_tr[] = {
   {
      "pokemini_video_scale",
      "Görüntü Ölçekleme (Yeniden Başlatma Gerektirir)",
      "Dahili video ölçeği faktörünü ayarlayın. Ölçek faktörünün arttırılması, dahili 'Dot Matrix' LCD filtresinin görünümünü iyileştirir.",
      {
         { NULL, NULL },
      },
      NULL
   },
   {
      "pokemini_lcdfilter",
      "LCD Filtresi",
      "Dahili ekran filtresini seçin. 'Dot Matrix', gerçek donanımı taklit eden bir LCD efekti üretir. 'Video Ölçeği' '1x' olarak ayarlandığında LCD filtreleri devre dışı bırakılır.",
      {
         { "dotmatrix", "Dot Matrix" },
         { "scanline",  "Scanlines" },
         { "none",      "Hiçbiri" },
         { NULL, NULL },
      },
      "dotmatrix"
   },
   {
      "pokemini_lcdmode",
      "LCD Modu",
      "Öykünülmüş sıvı kristal ekranın gri tonlamalı 'renk' üreme özelliklerini belirtin. 'Analog' gerçek donanımı taklit eder. '2 Shades' gölgelenmeyi giderir, ancak çoğu oyunda titremeye neden olur.",
      {
         { NULL, NULL }, /* Scale factors do not require translation */
      },
      NULL
   },
   {
      "pokemini_lcdcontrast",
      "LCD Kontrast",
      "Öykünülmüş sıvı kristal ekranın kontrast seviyesini ayarlayın.",
      {
         { NULL, NULL },
      },
      NULL
   },
   {
      "pokemini_lcdbright",
      "LCD Parlaklığı",
      "Öykünülmüş sıvı kristal ekranın parlaklık ofsetini ayarlayın.",
      {
         { NULL, NULL },
      },
      NULL
   },
   {
      "pokemini_palette",
      "Palette",
      "Öykünülmüş sıvı kristal ekranı 'renklendirmek' için kullanılan paleti belirtin. 'Varsayılan' gerçek donanımı taklit eder.",
      {
         { "Default",           "Varsayılan" },
         { "Old",               "Eski" },
         { "Monochrome",        "Siyah & Beyaz" },
         { "Green",             "Yeşil" },
         { "Green Vector",      "Ters Green" },
         { "Red",               "Kırmızı" },
         { "Red Vector",        "Ters Red" },
         { "Blue LCD",          "Mavi LCD" },
         { "LEDBacklight",      "LED Arka ışığı" },
         { "Girl Power",        "Kız Gücü" },
         { "Blue",              "Mavi" },
         { "Blue Vector",       "Ters Mavi" },
         { "Sepia",             "Sepya" },
         { "Monochrome Vector", "Ters Siyah & beyaz" },
         { NULL, NULL },
      },
      "Default"
   },
   {
      "pokemini_piezofilter",
      "Piezo Filtresi",
      "Pokemon Mini'nin piezoelektrik hoparlörünün özelliklerini taklit etmek için bir ses filtresi kullanın.",
      {
         { NULL, NULL },
      },
      NULL
   },
   {
      "pokemini_rumblelvl",
      "Rumble Level (Ekran + Kontrolör)",
      "Hem sanal hem de fiziksel olarak hareketli geri bildirim etkisinin kuvvetini belirtin.",
      {
         { NULL, NULL },
      },
      NULL
   },
   {
      "pokemini_controller_rumble",
      "Controller Rumble",
      "Kontrolör ile fiziksel kuvvet geri bildirim efektini etkinleştirin.",
      {
         { NULL, NULL },
      },
      NULL
   },
   {
      "pokemini_screen_shake",
      "Ekran Sarsıntısı",
      "Ekranı 'sallayarak' sanal güç geribildirim efektini etkinleştirin.",
      {
         { NULL, NULL },
      },
      NULL
   },
   { NULL, NULL, NULL, {{0}}, NULL },
};


/*
 ********************************
 * Language Mapping
 ********************************
*/

struct retro_core_option_definition *option_defs_intl[RETRO_LANGUAGE_LAST] = {
   option_defs_us, /* RETRO_LANGUAGE_ENGLISH */
   NULL,           /* RETRO_LANGUAGE_JAPANESE */
   option_defs_fr, /* RETRO_LANGUAGE_FRENCH */
   NULL,           /* RETRO_LANGUAGE_SPANISH */
   NULL,           /* RETRO_LANGUAGE_GERMAN */
   NULL,           /* RETRO_LANGUAGE_ITALIAN */
   NULL,           /* RETRO_LANGUAGE_DUTCH */
   NULL,           /* RETRO_LANGUAGE_PORTUGUESE_BRAZIL */
   NULL,           /* RETRO_LANGUAGE_PORTUGUESE_PORTUGAL */
   NULL,           /* RETRO_LANGUAGE_RUSSIAN */
   NULL,           /* RETRO_LANGUAGE_KOREAN */
   NULL,           /* RETRO_LANGUAGE_CHINESE_TRADITIONAL */
   NULL,           /* RETRO_LANGUAGE_CHINESE_SIMPLIFIED */
   NULL,           /* RETRO_LANGUAGE_ESPERANTO */
   NULL,           /* RETRO_LANGUAGE_POLISH */
   NULL,           /* RETRO_LANGUAGE_VIETNAMESE */
   NULL,           /* RETRO_LANGUAGE_ARABIC */
   NULL,           /* RETRO_LANGUAGE_GREEK */
   option_defs_tr, /* RETRO_LANGUAGE_TURKISH */
};

/*
 ********************************
 * Functions
 ********************************
*/

/* Handles configuration/setting of core options.
 * Should only be called inside retro_set_environment().
 * > We place the function body in the header to avoid the
 *   necessity of adding more .c files (i.e. want this to
 *   be as painless as possible for core devs)
 */

static INLINE void libretro_set_core_options(retro_environment_t environ_cb)
{
   unsigned version = 0;

   if (!environ_cb)
      return;

   if (environ_cb(RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION, &version) && (version == 1))
   {
      struct retro_core_options_intl core_options_intl;
      unsigned language = 0;

      core_options_intl.us    = option_defs_us;
      core_options_intl.local = NULL;

      if (environ_cb(RETRO_ENVIRONMENT_GET_LANGUAGE, &language) &&
          (language < RETRO_LANGUAGE_LAST) && (language != RETRO_LANGUAGE_ENGLISH))
         core_options_intl.local = option_defs_intl[language];

      environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL, &core_options_intl);
   }
   else
   {
      size_t i;
      size_t num_options               = 0;
      struct retro_variable *variables = NULL;
      char **values_buf                = NULL;

      /* Determine number of options */
      while (true)
      {
         if (option_defs_us[num_options].key)
            num_options++;
         else
            break;
      }

      /* Allocate arrays */
      variables  = (struct retro_variable *)calloc(num_options + 1, sizeof(struct retro_variable));
      values_buf = (char **)calloc(num_options, sizeof(char *));

      if (!variables || !values_buf)
         goto error;

      /* Copy parameters from option_defs_us array */
      for (i = 0; i < num_options; i++)
      {
         const char *key                        = option_defs_us[i].key;
         const char *desc                       = option_defs_us[i].desc;
         const char *default_value              = option_defs_us[i].default_value;
         struct retro_core_option_value *values = option_defs_us[i].values;
         size_t buf_len                         = 3;
         size_t default_index                   = 0;

         values_buf[i] = NULL;

         if (desc)
         {
            size_t num_values = 0;

            /* Determine number of values */
            while (true)
            {
               if (values[num_values].value)
               {
                  /* Check if this is the default value */
                  if (default_value)
                     if (strcmp(values[num_values].value, default_value) == 0)
                        default_index = num_values;

                  buf_len += strlen(values[num_values].value);
                  num_values++;
               }
               else
                  break;
            }

            /* Build values string */
            if (num_values > 1)
            {
               size_t j;

               buf_len += num_values - 1;
               buf_len += strlen(desc);

               values_buf[i] = (char *)calloc(buf_len, sizeof(char));
               if (!values_buf[i])
                  goto error;

               strcpy(values_buf[i], desc);
               strcat(values_buf[i], "; ");

               /* Default value goes first */
               strcat(values_buf[i], values[default_index].value);

               /* Add remaining values */
               for (j = 0; j < num_values; j++)
               {
                  if (j != default_index)
                  {
                     strcat(values_buf[i], "|");
                     strcat(values_buf[i], values[j].value);
                  }
               }
            }
         }

         variables[i].key   = key;
         variables[i].value = values_buf[i];
      }
      
      /* Set variables */
      environ_cb(RETRO_ENVIRONMENT_SET_VARIABLES, variables);

error:

      /* Clean up */
      if (values_buf)
      {
         for (i = 0; i < num_options; i++)
         {
            if (values_buf[i])
            {
               free(values_buf[i]);
               values_buf[i] = NULL;
            }
         }

         free(values_buf);
         values_buf = NULL;
      }

      if (variables)
      {
         free(variables);
         variables = NULL;
      }
   }
}

#ifdef __cplusplus
}
#endif

#endif
