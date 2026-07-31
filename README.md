# PokeMini (libretro)

A libretro port of [PokeMini](http://pokemini.sourceforge.net/), JustBurn's
emulator for the Nintendo Pokémon mini.

Core name: `PokeMini` &nbsp;·&nbsp; Library: `pokemini_libretro` &nbsp;·&nbsp; Version: `v0.60`

## Building

The libretro core is built from `Makefile.libretro`:

```sh
make -f Makefile.libretro
```

Cross-compiling is driven by `platform=`, which selects a fragment from
`build/`:

```sh
make -f Makefile.libretro platform=mingw_x86_64
```

Available platforms: `android_arm64-v8a`, `android_armeabi`,
`android_armeabi-v7a`, `android_mips`, `android_mips64`, `android_x86`,
`android_x86_64`, `linux_x86`, `linux_x86_64`, `linux-portable_x86`,
`linux-portable_x86_64`, `mingw_x86`, `mingw_x86_64`, `wii_ppc`,
`windows_x86`, `windows_x86_64`, `windows_msvc2003_x86`,
`windows_msvc2005_x86`, `windows_msvc2010_x86`, `windows_msvc2010_x64`,
`windows_msvc2015_x64`.

Android can also be built through `jni/Android.mk` with `ndk-build`; it
includes `build/Makefile.common`, so the source list is shared with every
other target.

`STATIC_LINKING=1` omits the bundled libretro-common objects, for frontends
that link the core statically and provide their own.

## Content

Loads `.min` ROM images. Content is passed in memory (`need_fullpath` is
false), so archives are handled by the frontend.

## BIOS

Optional. The core looks for `bios.min` in the frontend's system directory.
When it is absent, the bundled Pokémon mini FreeBIOS is used instead, so the
core runs without any external files.

## Controls

| RetroPad | Pokémon mini |
| --- | --- |
| D-Pad | D-Pad |
| A | A |
| B | B |
| X | Turbo A |
| R | C |
| L | Shake detector |
| Select | Power |

## Core options

| Key | Description |
| --- | --- |
| `pokemini_video_scale` | Internal render scale, `1x`–`7x` (restart required) |
| `pokemini_60hz_mode` | Report 60 Hz instead of the hardware's 72 Hz |
| `pokemini_lcdfilter` | LCD filter: Dot Matrix, Scanlines or None |
| `pokemini_lcdmode` | LCD shading: Analog, 3 Shades or 2 Shades |
| `pokemini_lcdcontrast` | LCD contrast |
| `pokemini_lcdbright` | LCD brightness |
| `pokemini_palette` | Display palette (17 presets) |
| `pokemini_piezofilter` | Piezo speaker filter |
| `pokemini_lowpass_filter` | Audio low-pass filter |
| `pokemini_lowpass_range` | Low-pass filter strength, in percent |
| `pokemini_screen_shake_lv` | Screen shake intensity |
| `pokemini_rumble_lv` | Controller rumble intensity |
| `pokemini_turbo_period` | Turbo button period |

## Technical notes

Base resolution is 96×64, scaled by `pokemini_video_scale`. Pixel format is
RGB565. Native refresh is 72 Hz; `pokemini_60hz_mode` reports exactly 60 Hz
(`5 × 72 / 6`) for displays that cannot follow the hardware rate. Audio is
output at 44100 Hz.

Savestates are supported and fixed-size. `RETRO_MEMORY_SYSTEM_RAM` exposes
the 8 KB CPU RAM. EEPROM is persisted as a `.eep` file in the save
directory.

Two fields inside a savestate are derived from the host clock — the header
timestamp, and the RTC stamp that `PokeMini_SyncHostTime()` writes into the
EEPROM. Two states captured from an identical emulator state will therefore
differ in those bytes. This is by design and predates the libretro port; it
is worth knowing when diffing states or writing determinism tests.

## libretro-common

The core vendors a subset of [libretro-common](https://github.com/libretro/RetroArch/tree/master/libretro-common)
under `libretro/libretro-common/`. Only the files the core actually compiles
are carried, and they are kept byte-identical to upstream RetroArch so that
re-syncing is a straight copy — please do not reformat them or fix
whitespace in place.

Four headers referenced by these sources are deliberately **not** vendored,
because the core never defines the macros that would pull them in:

| Header | Guarded by |
| --- | --- |
| `memmap.h` | `HAVE_MMAP` |
| `rthreads/rthreads.h` | `HAVE_THREADS` |
| `vfs/vfs_implementation_cdrom.h` | `HAVE_CDROM` |
| `vfs/vfs_implementation_saf.h` | `ANDROID` + `HAVE_SAF` |

If a future sync makes the core define any of these, the matching header (and
its implementation, where applicable) has to be added to
`build/Makefile.common` at the same time.

## License

PokeMini is licensed under the GNU General Public License v3. See
[LICENSE](LICENSE).

The bundled libretro-common files carry their own permissive licenses; see
the headers of the individual files.

## Upstream documentation

The original standalone-emulator documentation is preserved alongside this
file and still describes the SDL builds rather than the libretro core:

- `readme.txt` — general usage and keyboard controls
- `readme_portable.txt` — portable/handheld builds
- `readme_source.txt` — building the standalone emulator
- `history.txt` — upstream changelog
