#pragma once
#include <ctype.h>

void get_basename(const char *filename, char *basename)
{
	// Find last '/' in the path
	const char *last_slash = strrchr(filename, '/');
	const char *name_start = last_slash ? last_slash + 1 : filename;

	// Copy name to basename buffer
	strcpy(basename, name_start);

	// Find last '.' in the basename
	char *dot = strrchr(basename, '.');
	if(dot)
	{
		*dot = '\0'; // Remove extension
	}
}

static void strtoupper(char *str)
{
    for(char *p = str; *p; p++)
    {
        *p = toupper(*p);
    }
}