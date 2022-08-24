#ifndef _PZIP_FILES_H_
#define _PZIP_FILES_H_

int get_file_size(const char *filename);
size_t get_stream_input_size(int filecount, char **files);
size_t read_from_file(char *buffer, size_t bytes_to_read, char *filepath, int *is_reached_eof);

#endif // _PZIP_FILES_H_