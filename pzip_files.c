#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "pzip_files.h"

#define PRINTF_FILES_ENABLED
#ifndef PRINTF_FILES_ENABLED
#define PRINTF_FILES(...)
#else
#define PRINTF_FILES fprintf
#endif

int get_file_size(const char *filename)
{
	struct stat flstat;
	int         status = 0;
	int         retval = 0;

	memset (&flstat, 0, sizeof(flstat));

	status = stat(filename, &flstat);
	retval = (0 != status)?(status):(flstat.st_size);

	return retval;
}

size_t get_stream_input_size(int filecount, char **files)
{
	size_t ret = 0;
	for (int i = 0; i < filecount; i++)
	{
		char *filename = files[i];
		int filesize = get_file_size(filename);

		if (filesize < 0)
			exit(1);

		ret += filesize;
	}

	return ret;
}

size_t read_from_file(char *buffer, size_t bytes_to_read, char *filepath, int *is_reached_eof)
{
	static char *current_filepath = NULL;
	static size_t filesize = 0;
	static int fd = 0;
	size_t ret = 0;
	size_t seek_cur = 0;

	PRINTF_FILES(stderr, "in %s\n", __func__);

	if (filepath != current_filepath)
	{
		// get file size
		filesize = get_file_size(filepath);
		if (filesize == -1)
		{
			perror("get_file_size");
			exit(1);
		}

		if (filepath == NULL) // given filepath is null
		{
			PRINTF_FILES(stderr, "closing file fd %d path: %s\n", fd, current_filepath);
			if (fd > 2) close(fd);
			fd = 0;
			return 0;
		}

		PRINTF_FILES(stderr, "opening file %s\n", filepath);
		fd = open(filepath, O_RDONLY);
		if (fd == -1)
		{
			perror("open");
			exit(1);
		}
		current_filepath = filepath;
	}

	ret = read(fd, buffer, bytes_to_read);
	seek_cur = lseek(fd, 0, SEEK_CUR);
	PRINTF_FILES(stderr, "fd %d reached eof %d: red %ld bytes of bytes_to_read %ld: %s\n", fd, ret, bytes_to_read, buffer);
	printf("file at %ld of %ld\n", seek_cur, filesize);
	if (seek_cur == filesize)
	{
		*is_reached_eof = 1;
		PRINTF_FILES(stderr, "fd %d reached to the end of file: %s (closing)\n", fd, filepath);
		current_filepath = NULL;
		close(fd);
	}
	return ret;
}
