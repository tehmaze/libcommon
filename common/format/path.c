#include <talloc.h>
#include <stdbool.h>
#include <string.h>
#include <sys/param.h>
#include "common/debug.h"
#include "common/platform.h"

#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#define SEPCHR '\\'
#define SEPSTR "\\"

#else // PLATFORM_WINDOWS
#include <stdlib.h>
#include <unistd.h>
#define SEPCHR '/'
#define SEPSTR "/"

#endif

PRIVATE int format_path_canonical(char *dst, size_t len, const char *path)
{
	if (path == NULL)
		return -1;
	
#if defined(PLATFORM_WINDOWS)
	return GetFullPathName(path, len, dst, NULL);
#else
	char resolved[PATH_MAX + 1];
	(void)resolved;
	char *ptr;
	(void)ptr;
	if ((ptr = realpath(path, resolved)) == NULL) {
		return -1;
	}
	strncpy(dst, resolved, len);
#endif
    return 0;
}

PRIVATE char format_path_sep(void)
{
	return SEPCHR;
}

const char *format_path_ext(const char *path)
{
	const char* ptr = strrchr(path, '.');
	return ptr;
}

PRIVATE void format_path_join(char *dst, size_t len, const char *dir, const char *file)
{
	if (dir == NULL) {
		dst = NULL;
	} else if (file == NULL) {
		strncpy(dst, dir, len);
	} else if (file[0] == SEPCHR || dir[strlen(dir) - 1] == SEPCHR) {
		snprintf(dst, len, "%s%s", dir, file);
	} else {
		snprintf(dst, len, "%s%c%s", dir, SEPCHR, file);
	}
}
