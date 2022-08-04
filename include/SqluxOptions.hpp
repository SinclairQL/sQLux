#pragma once

#ifdef __cplusplus
namespace emulator
{
int optionParse(int argc, char *argv[]);
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
int optionInt(const char *optionName);
const char *optionString(const char *optionName);
float optionFloat(const char *optionName);
#ifdef __cplusplus
}
#endif
