# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

The Last Gate is a modified version of Mercenaries of Astonia v2, an MMORPG game engine originally developed by Daniel Brockhaus. This fork is based on openMerc by dylanyaga.

The codebase is split into two main components:
- **Server**: Linux-based C server (32-bit) that handles game logic, player data, and world state
- **Client**: Windows-based C client using DirectDraw for rendering

## Build Instructions
- DO NOT automatically run cmake or build commands
- Only suggest build commands for the user to run manually


## Build System

### Server (Linux - Ubuntu 16.04.1 LTS 32-bit recommended)

**Initial Setup:**
```bash
cd Source/Server
chmod +x setup.sh
./setup.sh
```
The setup script installs libraries, configures Apache2, compiles the server, sets up web-based editors, and copies files to `/home/merc/`.

**Compile (after initial setup):**
```bash
cd Source/Server
make
```

The Makefile compiles the server and utilities, then copies them to `/home/merc/`. Key targets:
- `server` - Main server executable
- `respawn` - Respawn utility
- `password` - Password utility
- `cgi/info.cgi`, `cgi/acct.cgi`, `cgi/mapper.cgi` - Web-based editors
- `owner` - Sets file ownership to current user

**Start Server:**
```bash
cd /home/merc
./server console    # Log to terminal (shutdown with CTRL+\ or CTRL+C)
./server            # Log to file (shutdown via system shutdown/restart)
```

**Web Administration:**
Navigate to `localhost/cgi-imp/acct.cgi` to edit items, NPCs, and templates. This edits LIVE server data.

### Client (Windows)

The client uses CMake or can be built with the provided `make.bat`:
```bash
cd Source/Client
# Using CMake:
cmake -B cmake-build-debug -S .
cmake --build cmake-build-debug

# Or using make.bat (legacy)
make.bat
```

The client requires graphic files from http://www.brockhaus.org/merc2.html and overlay images moved from `gfx/overlays/` to `gfx/`.

## Architecture

### Server Architecture

The server is structured around a tick-based game loop (20 ticks per second, defined in `gendefs.h`):
- `server.c` - Main server entry point, network handling, player connections
- `svr_tick.c` - Main game loop tick processing
- `driver.c` - NPC AI and behavior system (see NPC driver architecture below)
- `svr_do.c` - Player command processing and actions
- `svr_act.c` - Character action handling
- `build.c` - World building and area management
- `skill.c` / `skill_driver.c` - Skill system and calculations
- `use_driver.c` - Item usage logic
- `look_driver.c` - Visual information and map rendering
- `talk.c` - NPC dialogue system
- `helper.c` - Utility functions

**Data Structures:**
- `data.h` - Core data structures (character, item, map, effects)
- `gendefs.h` - Game constants and defines (map size: 1024x2048, max chars: 16K, max items: 256K)
- `funcs.h` - Function declarations
- `macros.h` - Gameplay macros
- `numbers.h` - Game balance numbers

### Client Architecture

DirectDraw-based Windows client:
- `main.c` - Entry point and window management
- `dd.c` - DirectDraw initialization and rendering
- `engine.c` - Core game engine and rendering loop
- `socket.c` - Network communication with server
- `inter.c` - User interface and input handling
- `options.c` - Configuration and settings
- `sound.c` - Audio playback

### NPC Driver System

NPCs use a data-driven driver system where behavior is controlled through `data[]` fields (indices 0-99). Key driver data slots are documented in `driver.c`:
- `data[10-18]`: Patrol stops (stored as `x+y*MAPX`)
- `data[19]`: Next stop in patrol
- `data[24]`: Fight mode prevention (-1=defend evil, 0=neutral, 1=defend good)
- `data[25]`: Special driver type (1=Grolmy/Timid, 2=City-attack, 3=Malte, 4=Shiva)
- `data[26]`: Special sub-driver (guards, temple NPCs, bosses)
- `data[29]`: Resting position
- `data[37-40]`: Last character talked to
- `data[42]`: Group
- `data[49]`: Wants item X
- `data[50]`: Teaches skill in exchange for item
- `data[63]`: Obey and protect character X
- `data[80-91]`: Kill list

### Player Data System

Players have 100 data slots (indices 0-99) for storing state. See `Source/Server/README.MD` for complete mapping. Key slots:
- `data[0]`: Away from keyboard status
- `data[13]`: Money in bank
- `data[14]`: Number of deaths
- `data[18]`: Experience for pentagramas
- `data[22]`: Current arena monster
- `data[29]`: Other players killed outside arena
- `data[42]`: Group
- `data[90]`: Database number

## Important Notes

### Server Development

- Take care not to call `act_xxx()` functions twice from a driver
- `MAXCHARS` cannot exceed 0x7fff (32767) due to flag usage in `do_look_depot()`
- The server uses 32-bit compilation (`gcc -m32`) with specific optimization flags
- Server directories: `/home/merc/` (main), `.dat/` (data), `.save/` (saves), `.tmp/` (temporary)

### Map and World

- Map size: 1024x2048 (defined in `gendefs.h`)
- Map coordinates stored as single value: `m = x + y * MAPX`
- Game time: 1 MD hour = 5 real minutes, 1 MD day = 24 MD hours

### Code Style

- C89/C90 standard
- Packed structs with `__attribute__ ((packed))`
- Extensive use of bitflags for character/item/map properties
- Global state stored in `struct global` (see `data.h`)

### Version

Current version: 0x000D01 (13.1)
Minimum compatible version: 0x000D00 (13.0)

See `gendefs.h` for version numbers and game constants.
