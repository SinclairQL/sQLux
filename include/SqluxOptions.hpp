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
float optionFloat(const char *optionName);
int optionArgc(void);
const char *optionArgv(int index);
#ifdef __cplusplus
}
#endif
