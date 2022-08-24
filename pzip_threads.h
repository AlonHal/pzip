#ifndef _PZIP_THREADS_H_
#define _PZIP_THREADS_H_

#include "pzip_linkedlist.h"
#include "pzip_msgq.h"

struct reader_thread_args
{
	int file_count;
	char **file_paths;
	mqd_t qd_send;
};

struct zipper_thread_args
{
	int thread_count;
	mqd_t qd_reader_recv;
	mqd_t qd_writer_send;
};

struct writer_thread_args
{
	mqd_t qd_writer_recv;
};

struct zipper_thread_data
{
	char *buffer;
	size_t length;
};

struct pzip_section
{
	const char *buffer;
	size_t byte_from;
	size_t byte_to;
	struct pzip_node *head;
	struct pzip_node *tail;
};

struct pzip_thread_data
{
	pthread_t tid;
	struct pzip_section *pz_sec_ptr;
};

void * reader_thread(void *arg);
int count_char(const char *str, size_t size);
struct pzip_thread_data *create_pzip_thread(const char *buffer, size_t from, size_t to);
void * create_pzip_threads(void *arg);
void * writer_thread(void *arg);

pthread_t create_reader_thread(struct reader_thread_args *rd_args);
pthread_t create_zipper_thread(struct zipper_thread_args *zp_args);
pthread_t create_writer_thread(struct writer_thread_args *wr_args);

#endif // _PZIP_THREADS_H_