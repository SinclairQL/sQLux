#pragma once

#ifdef __cplusplus
namespace emulator
{
int optionParse(int argc, char *argv[]);
void deviceParse(void);
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
int optionInt(const char *optionName);
const char *optionString(const char *optionName);
float optionFloat(const char *optionName);
int optionArgc(void);
const char *optionArgv(int index);
#ifdef __cplusplus
}
#endif
