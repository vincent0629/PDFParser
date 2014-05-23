#ifndef _NAMETOUNICODE_H_
#define _NAMETOUNICODE_H_

#include <wchar.h>

typedef struct _SNameToUnicode
{
	wchar_t unicode;
	const char *name;
} SNameToUnicode;

#ifdef __cplusplus
extern "C"
{
#endif
extern SNameToUnicode nameToUnicode[];
#ifdef __cplusplus
};
#endif

#endif
