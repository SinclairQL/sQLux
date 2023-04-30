#pragma once

#ifndef EMULATOR_INIT_H
#define EMULATOR_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

int emulatorLoadRom(const char *romDir, const char *romName, uint32_t addr, size_t size);

#ifdef __cplusplus
};
#endif

#endif /* EMULATOR_INIT_H */