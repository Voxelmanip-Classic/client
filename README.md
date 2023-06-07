# Voxelmanip Classic

This is the client for Voxelmanip Classic, which is forked from the Minetest engine.

Table of Contents
------------------

2. [Default Controls](#default-controls)
3. [Paths](#paths)
4. [Configuration File](#configuration-file)
5. [Command-line Options](#command-line-options)
6. [Compiling](#compiling)

Default controls
----------------
All controls are re-bindable using settings.
Some can be changed in the key config dialog in the settings tab.

| Button                        | Action                                                         |
|-------------------------------|----------------------------------------------------------------|
| Move mouse                    | Look around                                                    |
| W, A, S, D                    | Move                                                           |
| Space                         | Jump/move up                                                   |
| Shift                         | Sneak/move down                                                |
| Q                             | Drop itemstack                                                 |
| Shift + Q                     | Drop single item                                               |
| Left mouse button             | Dig/punch/use                                                  |
| Right mouse button            | Place/use                                                      |
| Shift + right mouse button    | Build (without using)                                          |
| I                             | Inventory menu                                                 |
| Mouse wheel                   | Select item                                                    |
| 0-9                           | Select item                                                    |
| Z                             | Zoom (needs zoom privilege)                                    |
| T                             | Chat                                                           |
| /                             | Command                                                        |
| Esc                           | Pause menu/abort/exit (pauses only singleplayer game)          |
| +                             | Increase view range                                            |
| -                             | Decrease view range                                            |
| K                             | Enable/disable fly mode (needs fly privilege)                  |
| J                             | Enable/disable fast mode (needs fast privilege)                |
| H                             | Enable/disable noclip mode (needs noclip privilege)            |
| E                             | Aux1 (Move fast in fast mode. Games may add special features)  |
| C                             | Cycle through camera modes                                     |
| V                             | Cycle through minimap modes                                    |
| Shift + V                     | Change minimap orientation                                     |
| F1                            | Hide/show HUD                                                  |
| F2                            | Hide/show chat                                                 |
| F3                            | Disable/enable fog                                             |
| F4                            | Disable/enable camera update (Mapblocks are not updated anymore when disabled, disabled in release builds)  |
| F5                            | Cycle through debug information screens                        |
| F6                            | Cycle through profiler info screens                            |
| F10                           | Show/hide console                                              |
| F12                           | Take screenshot                                                |

Paths
-----
Locations:

* `bin`   - Compiled binaries
* `share` - Distributed read-only data
* `user`  - User-created modifiable data

Where each location is on each platform:

* Windows .zip / RUN_IN_PLACE source:
    * `bin`   = `bin`
    * `share` = `.`
    * `user`  = `.`
* Windows installed:
    * `bin`   = `C:\Program Files\Minetest\bin (Depends on the install location)`
    * `share` = `C:\Program Files\Minetest (Depends on the install location)`
    * `user`  = `%APPDATA%\Minetest` or `%MINETEST_USER_PATH%`
* Linux installed:
    * `bin`   = `/usr/bin`
    * `share` = `/usr/share/minetest`
    * `user`  = `~/.minetest` or `$MINETEST_USER_PATH`
* macOS:
    * `bin`   = `Contents/MacOS`
    * `share` = `Contents/Resources`
    * `user`  = `Contents/User` or `~/Library/Application Support/minetest` or `$MINETEST_USER_PATH`

Worlds can be found as separate folders in: `user/worlds/`

Configuration file
------------------
- Default location:
    `user/minetest.conf`
- This file is created by closing Minetest for the first time.
- A specific file can be specified on the command line:
    `--config <path-to-file>`
- A run-in-place build will look for the configuration file in
    `location_of_exe/../minetest.conf` and also `location_of_exe/../../minetest.conf`

Command-line options
--------------------
- Use `--help`

Compiling
---------

#### Dependencies

| Dependency | Version | Commentary |
|------------|---------|------------|
| GCC        | 5.1+    | or Clang 3.5+ |
| CMake      | 3.5+    |            |
| IrrlichtMt | -       | Custom version of Irrlicht |
| Freetype   | 2.0+    |            |
| Zstd       | 1.0+    |            |
| LuaJIT     | 2.0+    | Bundled Lua 5.1 is used if not present |
| GMP        | 5.0.0+  | Bundled mini-GMP is used if not present |
| JsonCPP    | 1.0.0+  | Bundled JsonCPP is used if not present |

For Debian/Ubuntu users:

    sudo apt install g++ make libc6-dev cmake libpng-dev libjpeg-dev libxi-dev libgl1-mesa-dev libogg-dev libvorbis-dev libopenal-dev libcurl4-gnutls-dev libfreetype6-dev zlib1g-dev libgmp-dev libjsoncpp-dev libzstd-dev libluajit-5.1-dev

For Fedora users:

    sudo dnf install make automake gcc gcc-c++ kernel-devel cmake libcurl-devel openal-soft-devel libpng-devel libjpeg-devel libvorbis-devel libXi-devel libogg-devel freetype-devel mesa-libGL-devel zlib-devel jsoncpp-devel gmp-devel luajit-devel spatialindex-devel libzstd-devel

For Arch users:

    sudo pacman -S base-devel libcurl-gnutls cmake libxi libpng libogg libvorbis openal freetype2 jsoncpp gmp luajit zstd

For Alpine users:

    sudo apk add build-base cmake libpng-dev jpeg-dev libxi-dev mesa-dev libogg-dev libvorbis-dev openal-soft-dev curl-dev freetype-dev zlib-dev gmp-dev jsoncpp-dev luajit-dev zstd-dev

#### Download

Download source (this is the URL to the latest of source repository, which might not work at all times) using Git:

    git clone --depth 1 https://github.com/rollerozxa/voxelmanip-classic-mt
    cd minetest

Download IrrlichtMt to `lib/irrlichtmt`, it will be used to satisfy the IrrlichtMt dependency that way:

    git clone --depth 1 https://github.com/rollerozxa/irrlicht-vmc lib/irrlichtmt

#### Build

Build a version that runs directly from the source directory:

    cmake . -G Ninja -DRUN_IN_PLACE=TRUE
    ninja

Run it:

    ./bin/voxelmanip_classic

- Use `cmake . -LH` to see all CMake options and their current state.
- If you want to install it system-wide (or are making a distribution package),
  you will want to use `-DRUN_IN_PLACE=FALSE`.
- You can select between Release and Debug build by `-DCMAKE_BUILD_TYPE=<Debug or Release>`.
  - Debug build is slower, but gives much more useful output in a debugger.

### CMake options

General options and their default values:

    CMAKE_BUILD_TYPE=Release   - Type of build (Release vs. Debug)
        Release                - Release build
        Debug                  - Debug build
        SemiDebug              - Partially optimized debug build
        RelWithDebInfo         - Release build with debug information
        MinSizeRel             - Release build with -Os passed to compiler to make executable as small as possible
    ENABLE_LUAJIT=ON           - Build with LuaJIT (much faster than non-JIT Lua)
    ENABLE_SYSTEM_GMP=ON       - Use GMP from system (much faster than bundled mini-gmp)
    ENABLE_SYSTEM_JSONCPP=ON   - Use JsonCPP from system
    RUN_IN_PLACE=FALSE         - Create a portable install (worlds, settings etc. in current directory)
    USE_GPROF=FALSE            - Enable profiling using GProf
    VERSION_EXTRA=             - Text to append to version (e.g. VERSION_EXTRA=foobar -> Minetest 0.4.9-foobar)
    ENABLE_TOUCH=FALSE         - Enable Touchscreen support (requires support by IrrlichtMt)

Library specific options:

    EXTRA_DLL                       - Only on Windows; optional paths to additional DLLs that should be packaged
    FREETYPE_INCLUDE_DIR_freetype2  - Directory that contains files such as ftimage.h
    FREETYPE_INCLUDE_DIR_ft2build   - Directory that contains ft2build.h
    FREETYPE_LIBRARY                - Path to libfreetype.a/libfreetype.so/freetype.lib
    FREETYPE_DLL                    - Only on Windows; path to libfreetype-6.dll
    ICONV_LIBRARY                   - Optional/platform-dependent; path to libiconv.so/libiconv.dylib
    IRRLICHT_DLL                    - Only on Windows; path to IrrlichtMt.dll
    IRRLICHT_INCLUDE_DIR            - Directory that contains IrrCompileConfig.h (usable for server build only)
    LUA_INCLUDE_DIR                 - Only if you want to use LuaJIT; directory where luajit.h is located
    LUA_LIBRARY                     - Only if you want to use LuaJIT; path to libluajit.a/libluajit.so
    OGG_DLL                         - Only if building with sound on Windows; path to libogg.dll
    OGG_INCLUDE_DIR                 - Only if building with sound; directory that contains an ogg directory which contains ogg.h
    OGG_LIBRARY                     - Only if building with sound; path to libogg.a/libogg.so/libogg.dll.a
    OPENAL_DLL                      - Only if building with sound on Windows; path to OpenAL32.dll
    OPENAL_INCLUDE_DIR              - Only if building with sound; directory where al.h is located
    OPENAL_LIBRARY                  - Only if building with sound; path to libopenal.a/libopenal.so/OpenAL32.lib
    VORBISFILE_LIBRARY              - Only if building with sound; path to libvorbisfile.a/libvorbisfile.so/libvorbisfile.dll.a
    VORBIS_DLL                      - Only if building with sound on Windows; paths to vorbis DLLs
    VORBIS_INCLUDE_DIR              - Only if building with sound; directory that contains a directory vorbis with vorbisenc.h inside
    VORBIS_LIBRARY                  - Only if building with sound; path to libvorbis.a/libvorbis.so/libvorbis.dll.a
    ZLIB_DLL                        - Only on Windows; path to zlib1.dll
    ZLIB_INCLUDE_DIR                - Directory that contains zlib.h
    ZLIB_LIBRARY                    - Path to libz.a/libz.so/zlib.lib
    ZSTD_DLL                        - Only on Windows; path to libzstd.dll
    ZSTD_INCLUDE_DIR                - Directory that contains zstd.h
    ZSTD_LIBRARY                    - Path to libzstd.a/libzstd.so/ztd.lib
