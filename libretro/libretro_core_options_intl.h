#ifndef LIBRETRO_CORE_OPTIONS_INTL_H__
#define LIBRETRO_CORE_OPTIONS_INTL_H__

#if defined(_MSC_VER) && (_MSC_VER >= 1500 && _MSC_VER < 1900)
/* https://support.microsoft.com/en-us/kb/980263 */
#pragma execution_character_set("utf-8")
#pragma warning(disable:4566)
#endif

#include <libretro.h>

/*
 ********************************
 * VERSION: 1.3
 ********************************
 *
 * - 1.3: Move translations to libretro_core_options_intl.h
 *        - libretro_core_options_intl.h includes BOM and utf-8
 *          fix for MSVC 2010-2013
 *        - Added HAVE_NO_LANGEXTRA flag to disable translations
 *          on platforms/compilers without BOM support
 * - 1.2: Use core options v1 interface when
 *        RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION is >= 1
 *        (previously required RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION == 1)
 * - 1.1: Support generation of core options v0 retro_core_option_value
 *        arrays containing options with a single value
 * - 1.0: First commit
*/

#ifdef __cplusplus
extern "C" {
#endif

/*
 ********************************
 * Core Option Definitions
 ********************************
*/

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

#ifdef __cplusplus
}
#endif

#endif
