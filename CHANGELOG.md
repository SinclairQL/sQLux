# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
### Added
- Cycle counted CPU timing: every instruction charges its real 68008 cycle cost, and `SPEED = 1` is now the 7.5 MHz clock of an original QL
- Exact DIVU and DIVS timing when the divisor is a register, following the algorithm by Jorge Cwik
- `CPU_TIMING` option to use 68000 (16 bit bus) instruction timings
- `tools/validate_cycles.py` to check the 68000 timing table against the Tom Harte ProcessorTests
- Bit level microdrive emulation through the ZX8302 registers ($18020-$18023), translated from the MiSTer QL core. New options `MDV1`, `MDV2` (cartridge images) and `MDV_REVERSE`
- Beam position counted in emulated clock cycles from each vertical sync, with lines of 64 us (63.2 us NTSC), used by the line by line screen capture and the memory contention. The first visible line is 36 lines after the frame interrupt, calibrated against demos on a real QL (`ZX8301_VSYNC_LINES` option)
- `ZX8301_CONTENTION` option: wait states for CPU accesses to the internal RAM while the ZX8301 fetches the screen or refreshes the DRAM, at `SPEED = 1`, following the MiSTer QL core timing by Marcel Kilgus and Daniele Terdina
- `NTSC` option: 60 Hz frame interrupt and 262 lines per frame
- `tools/timing_tests_bas`: SuperBASIC timing tests to compare a real QL with the emulator
- `IPC_ROM` option: low level emulation of the IPC (Intel 8049) running its original firmware: keyboard matrix scanning, speaker output and the serial link with the ZX8302

### Changed
- The speed limiter counts the emulated clock cycles actually executed instead of fixed chunks of instructions

### Fixed
- The main thread waits for SDL events instead of polling them in a busy loop, which kept a whole CPU core busy
- QSound: tone and noise generators follow jt49: a null period mutes the generator instead of producing the highest frequency, and the noise output has the polarity of the real chip

## 1.1.1 - 2025-03-12
### Changed
- Updated German keypmap for MacOS and windows/linux

### Added
- Add EMU_EXIT and EMU_VER$ extensions
- Support for the keypad on PC keyboards
- Support for loading ROMs that are shorter than allocated space

## 1.1.0 - 2024-12-22
### Added
- Zip the complete source on release including submodules
- Use kacl-cli to generate version if git doesnt work
- Add install target to Makefile

### Fixed
- Fix the build instructions in the doc

## 1.0.10 - 2024-12-21
### Added
- workflows now generate a PDF manual
- workflows add an aarch64 macos build

## 1.0.9 - 2024-12-21
### Changed
- Rewrote CI (Github actions)
  See [README.md](README.md#releases-anchor)
  Creates automatically Github Releases for:
  - Linux (x86-64, armv7, aarch) on Ubuntu and Debian
  - MacOS (x86-64 Intel)
  - Windows (MSYS2 based on x86-64 and i686)
- Removed `.vscode`

### Added
- CHANGELOG.md
- Renamed/added some `cmake` arch. specific `Toolchain*` definitions
- .gitkeep to keep `build/` folder
- QL SW content included in artifacts:
  - `mdv*` folders (moved to `examples/`)
  - New `examples/sqlux.ini`
  - New `examples/mdv3` : SQLMISE_TestChart2.zip from Dilwyn SW repo
- Added `cmake` spec files `Toolchain-*.cmake` for `arm64` and `armhf` (used in CI scripts)

### Fixed
- Build was failing on `arm` due to missing CC flag `-lm` on `SDL2`

## [1.0.6] - 2022-11-22

## [1.0.5] - 2022-05-02

## [Unreleased]
### Added
- Cycle counted CPU timing: every instruction charges its real 68008 cycle cost, and `SPEED = 1` is now the 7.5 MHz clock of an original QL
- Exact DIVU and DIVS timing when the divisor is a register, following the algorithm by Jorge Cwik
- `CPU_TIMING` option to use 68000 (16 bit bus) instruction timings
- `tools/validate_cycles.py` to check the 68000 timing table against the Tom Harte ProcessorTests
- Bit level microdrive emulation through the ZX8302 registers ($18020-$18023), translated from the MiSTer QL core. New options `MDV1`, `MDV2` (cartridge images) and `MDV_REVERSE`
- Beam position counted in emulated clock cycles from each vertical sync, with lines of 64 us (63.2 us NTSC), used by the line by line screen capture and the memory contention. The first visible line is 41 lines after the frame interrupt, 6 lines of vertical sync and 35 of top border (`ZX8301_VSYNC_LINES` option)
- `ZX8301_CONTENTION` option: wait states for CPU accesses to the internal RAM while the ZX8301 fetches the screen or refreshes the DRAM, following the slot mechanism of the MiSTer QL core by Marcel Kilgus and Daniele Terdina, calibrated against a real QL (28 busy slots of 40 in the visible lines, 27 in the others). It applies at any speed except unlimited, scaled to the real time of the ZX8301
- `NTSC` option: 262 lines per frame (60.39 Hz)
- `tools/timing_tests_bas`: SuperBASIC timing tests to compare a real QL with the emulator
- `IPC_ROM` option: low level emulation of the IPC (Intel 8049) running its original firmware (Intel HEX or raw 2K binary): keyboard matrix scanning, speaker output, the serial link with the ZX8302 and BAUDx4, with its own 11 MHz clock, independent of `SPEED`
- F10 toggles between the configured speed and unlimited speed
- `HW_TRACE` option: hardware activity per second, to debug timing problems

### Changed
- The speed limiter counts the emulated clock cycles actually executed instead of fixed chunks of instructions
- A frame is a whole number of lines: 312 lines of 480 cycles (50.08 Hz) with PAL, 262 lines of 474 cycles (60.39 Hz) with NTSC
- Interrupt lines are levels, as on the ZX8302: an interrupt stays pending until its bit in $18021 is cleared, and it is taken right after the instruction that lowers the mask
- The display mode and blank bit are stored for each screen line, so mode changes in the middle of the screen are shown

### Fixed
- The main thread waits for SDL events instead of polling them in a busy loop, which kept a whole CPU core busy
- QSound: tone and noise generators follow jt49: a null period mutes the generator instead of producing the highest frequency, and the noise output has the polarity of the real chip
- The frame interrupt is raised at the cycle of the vertical sync: it was raised up to one chunk of 300 instructions late, and frames could be cut short when the host was late
- `CLR` reads its operand before writing it, and `MOVEM` from memory reads one more word, as on a 68000, so both pay their memory contention
- CPU writes to the screen line being scanned are shown in the same frame if the ZX8301 has not fetched those words yet
- With `CPU_HOG = 0` the emulated time no longer stops while QDOS is idle
- QSound is only mapped when `QSROM` is set, and it is disabled with a warning if the RAM overlaps its address space at $C0000: with more than 768K the RAM test of the JS ROM failed (white screen)
- With `IPC_ROM`, a key press delivered late by the host (seen with sdl2-compat on Wayland) could be missed by the IPC firmware: every key now stays in the keyboard matrix for at least 40 ms