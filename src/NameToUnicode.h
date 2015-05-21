#ifndef _NAMETOUNICODE_H_
#define _NAMETOUNICODE_H_

#include <wchar.h>

typedef struct _NameToUnicode
{
	wchar_t unicode;
	const char *name;
} NameToUnicode;

#ifdef __cplusplus
extern "C"
{
#endif
extern NameToUnicode nameToUnicode[];
#ifdef __cplusplus
};
#endif

#endif
