//-----------------------------------------------------------------------------------
// Implements unix style directory functions for windows.  This module used for
// windows test development of imgcomp.  Not part of the linux build.
//
// Copied from:
// http://www.opensource.apple.com/source/apache_mod_php/apache_mod_php-4.3/php/win32/readdir.c
//-----------------------------------------------------------------------------------
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include "readdir_win.h"

/**********************************************************************
 * Implement dirent-style opendir/readdir/closedir on Window 95/NT
 *
 * Functions defined are opendir(), readdir() and closedir() with the
 * same prototypes as the normal dirent.h implementation.
 *
 * Does not implement telldir(), seekdir(), rewinddir() or scandir(). 
 * The dirent struct is compatible with Unix, except that d_ino is 
 * always 1 and d_off is made up as we go along.
 *
 * The DIR typedef is not compatible with Unix.
 **********************************************************************/

DIR *opendir(const char *dir)
{
	DIR *dp;
	char *filespec;
	long handle;
	int index;

	filespec = malloc(strlen(dir) + 2 + 1);
	strcpy(filespec, dir);
	index = strlen(filespec) - 1;
	if (index >= 0 && (filespec[index] == '/' || filespec[index] == '\\'))
		filespec[index] = '\0';
	strcat(filespec, "/*");

	dp = (DIR *) malloc(sizeof(DIR));
	dp->offset = 0;
	dp->finished = 0;
	dp->dir = _strdup(dir);

	if ((handle = _findfirst(filespec, &(dp->fileinfo))) < 0) {
		if (errno == ENOENT)
			dp->finished = 1;
		else
			return NULL;
	}
	dp->handle = handle;
	free(filespec);

	return dp;
}

struct dirent *readdir(DIR *dp)
{
	if (!dp || dp->finished)
		return NULL;

	if (dp->offset != 0) {
		if (_findnext(dp->handle, &(dp->fileinfo)) < 0) {
			dp->finished = 1;
			return NULL;
		}
	}
	dp->offset++;

	strncpy(dp->dent.d_name, dp->fileinfo.name, _MAX_FNAME+1);
	dp->dent.d_ino = 1;
	dp->dent.d_reclen = strlen(dp->dent.d_name);
	dp->dent.d_off = dp->offset;

	return &(dp->dent);
}

int readdir_r(DIR *dp, struct dirent *entry, struct dirent **result)
{
	if (!dp || dp->finished) {
		*result = NULL;
		return 0;
	}

	if (dp->offset != 0) {
		if (_findnext(dp->handle, &(dp->fileinfo)) < 0) {
			dp->finished = 1;
			*result = NULL;
			return 0;
		}
	}
	dp->offset++;

	strncpy(dp->dent.d_name, dp->fileinfo.name, _MAX_FNAME+1);
	dp->dent.d_ino = 1;
	dp->dent.d_reclen = strlen(dp->dent.d_name);
	dp->dent.d_off = dp->offset;

	memcpy(entry, &dp->dent, sizeof(*entry));

	*result = &dp->dent;

	return 0;
}

int closedir(DIR *dp)
{
	if (!dp)
		return 0;
	_findclose(dp->handle);
	if (dp->dir)
		free(dp->dir);
	if (dp)
		free(dp);

	return 0;
}

void rewinddir(DIR *dir_Info)
{
	/* Re-set to the beginning */
	char *filespec;
	long handle;
	int index;

	dir_Info->handle = 0;
	dir_Info->offset = 0;
	dir_Info->finished = 0;

	filespec = malloc(strlen(dir_Info->dir) + 2 + 1);
	strcpy(filespec, dir_Info->dir);
	index = strlen(filespec) - 1;
	if (index >= 0 && (filespec[index] == '/' || filespec[index] == '\\'))
		filespec[index] = '\0';
	strcat(filespec, "/*");

	if ((handle = _findfirst(filespec, &(dir_Info->fileinfo))) < 0) {
		if (errno == ENOENT) {
			dir_Info->finished = 1;
		}
	}
	dir_Info->handle = handle;
	free(filespec);
}


#include <windows.h>
void sleep(int a)
{
    printf("Pretending to sleep for %d seconds\n",a);
    getchar();
}