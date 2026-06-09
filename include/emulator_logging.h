/*
 * Copyright (c) 2022 Graeme Gregory
 *
 * SPDX: GPL-2.0-only
 */
#pragma once

#ifndef EMULATOR_LOGGING
#define EMULATOR_LOGGING

#include <SDL3/SDL.h>

enum {
	EMU_LOG_DISK = SDL_LOG_CATEGORY_CUSTOM,
	EMU_LOG_HW,
	EMU_LOG_IO,
	EMU_LOG_IPC,
	EMU_LOG_MDV,
	EMU_LOG_SCREEN,
	EMU_LOG_SD,
	EMU_LOG_SOUND,
};

#endif // EMULATOR_LOGGING
