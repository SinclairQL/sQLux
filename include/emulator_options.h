#pragma once

#ifndef EMULATOR_OPTIONS_H
#define EMULATOR_OPTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

int emulatorOptionParse(int argc, char **argv);
void emulatorOptionsRemove(void);
char *emulatorOptionString(const char *name);
int emulatorOptionInt(const char *name);

#ifdef __cplusplus
};
#endif

#endif /* EMULATOR_OPTIONS_H */