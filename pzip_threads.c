#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#include <pthread.h>

#include "pzip_linkedlist.h"
#include "pzip_files.h"
#include "pzip_threads.h"

#define PRINTF_READER_ENABLED
#ifndef PRINTF_READER_ENABLED
#define PRINTF_READER(...)
#else
#define PRINTF_READER fprintf
#endif

// #define PRINTF_PZIP_ENABLED
#ifndef PRINTF_PZIP_ENABLED
#define PRINTF_PZIP(...)
#else
#define PRINTF_PZIP fprintf
#endif

// #define PRINTF_WRITER_ENABLED
#ifndef PRINTF_WRITER_ENABLED
#define PRINTF_WRITER(...)
#else
#define PRINTF_WRITER fprintf
#endif

#define SLEEP_MILISECONDS 1000

const size_t chunk_size = 80;
size_t g_stream_input_sz = 0;

void * reader_thread(void *arg)
{
	struct reader_thread_args *rd_args_ptr = (struct reader_thread_args *) arg;
	g_stream_input_sz = get_stream_input_size(rd_args_ptr->file_count, rd_args_ptr->file_paths);
	int file_ind = 0;
	int is_reached_eof = 0;

	size_t bytes_red = 0;
	size_t bytes_sent = 0;

	while (is_reached_eof != 1 && file_ind != rd_args_ptr->file_count)
	{
		struct zipper_thread_data *zip_data_ptr = (struct zipper_thread_data *) malloc(sizeof(struct zipper_thread_data));
		if (zip_data_ptr == NULL)
		{
			perror("malloc");
			exit(1);
		}
		memset(zip_data_ptr, 0, sizeof(*zip_data_ptr));
		// allocate chunk size bytes
		zip_data_ptr->buffer = (char *) malloc(chunk_size);
		if (zip_data_ptr->buffer == NULL)
		{
			perror("malloc");
			exit(1);
		}

		memset(zip_data_ptr->buffer, 0, chunk_size);
		bytes_red = 0;

		// read chunk size bytes
		while (bytes_red < chunk_size)
		{
			char *filepath = rd_args_ptr->file_paths[file_ind];
			PRINTF_READER(stderr, "reader thread buffer %p, chunk_size - bytes_red %ld, filepath %s, &is_reached_eof %d\n",
				zip_data_ptr->buffer, chunk_size - bytes_red, filepath, is_reached_eof);
			bytes_red += read_from_file(zip_data_ptr->buffer + bytes_red, chunk_size - bytes_red, filepath, &is_reached_eof);

			if (is_reached_eof)
			{
				if (file_ind == rd_args_ptr->file_count)
					break;
				file_ind++;
				is_reached_eof = 0;
			}
		}
		zip_data_ptr->length = bytes_red;

		// send buffer
		PRINTF_READER(stderr, "reader thread sending(%ld): %s\n", zip_data_ptr->length, zip_data_ptr->buffer);
		mq_send(rd_args_ptr->qd_send, (char *) &zip_data_ptr, sizeof(struct zipper_thread_data *), 0);

		bytes_sent += bytes_red;

		PRINTF_READER(stderr, "reader thread bytes_sent %ld, g_stream_input_sz %ld\n", bytes_sent, g_stream_input_sz);
	}
}

int count_char(const char *str, size_t size)
{
	pid_t tid = gettid();
	char c = str[0];
	int i = 1;

	PRINTF_PZIP(stderr, "\t(%d) %c%d\n", tid, c, i);

	while (i < size && c == str[i]) {
		i++;

		#ifdef SLEEP_MILISECONDS
		usleep(1000 * 1000);
		#endif

		PRINTF_PZIP(stderr, "\t(%d) %c%d\n", tid, c, i);
	}

	return i;
}

void * pzip_compress_thread(void *arg)
{
	pid_t tid = gettid();
	struct pzip_section *pzip_section_ptr = (struct pzip_section *) arg;
	struct pzip_node *head = NULL, *tail = NULL;

	for (int i = pzip_section_ptr->byte_from, cnt = 0; i < pzip_section_ptr->byte_to; i += cnt)
	{
		struct zipped_elem elem = {0};
		cnt = count_char(pzip_section_ptr->buffer + i, pzip_section_ptr->byte_to - i);
		PRINTF_PZIP(stderr, "\t(%d) cnt is %d, i is %d\n", tid, cnt, i);

		elem.c = pzip_section_ptr->buffer[i];
		elem.cnt = cnt;

		head = pzip_node_add_next(head, &elem);
		if (tail == NULL) tail = head;
	}

	pzip_node_print_nodes_moving_next(tail);
	PRINTF_PZIP(stderr, "\n");

	pzip_section_ptr->head = head;
	pzip_section_ptr->tail = tail;

	return NULL;
}

void * create_pzip_threads(void *arg)
{
	struct zipper_thread_args *zipper_args = (struct zipper_thread_args *) arg;
	struct pzip_thread_data **td = malloc(zipper_args->thread_count * sizeof(struct pzip_thread_data *));
	size_t bytes_zipped = 0;
	do
	{
		struct zipper_thread_data *zipper_data = NULL;
		if (mq_receive(zipper_args->qd_reader_recv, (char *) &zipper_data, sizeof(struct zipper_thread_data *), NULL) == -1)
		{
			perror("mq_receive");
			exit(1);
		}
		PRINTF_PZIP(stderr, "zipper_data {%ld};\n", zipper_data->length);

		for (size_t i = 0, from = 0; i < zipper_args->thread_count; i++)
		{
			size_t to = (i + 1) * zipper_data->length / zipper_args->thread_count;
			PRINTF_PZIP(stderr, "create_pzip_thread(%ld, %ld, %ld);\n", i, from, to);
			td[i] = create_pzip_thread(zipper_data->buffer, from, to);
			from = to;
		}

		for (size_t i = 0, from = 0; i < zipper_args->thread_count; i++)
		{
			PRINTF_PZIP(stderr, "pthread_join(%ld);\n", i);
			pthread_join(td[i]->tid, NULL); // join create_pzip_thread(i, zipper_data->buffer, from, to);

			PRINTF_PZIP(stderr, "sending %ld to writer\n", i);
			mq_send(zipper_args->qd_writer_send, (char *) &td[i]->pz_sec_ptr, sizeof(struct pzip_section *), 0); // mq_send(cl_args->qd_writer_send, (char *) &td[i]->pz_sec_ptr, sizeof(struct pzip_section *), 0);

			PRINTF_PZIP(stderr, "freeing %ld\n", i);
			free(td[i]);
			td[i] = NULL;
		}

		bytes_zipped += zipper_data->length;
	} while (bytes_zipped < g_stream_input_sz);

	free(td);
}

struct pzip_thread_data *create_pzip_thread(const char *buffer, size_t from, size_t to)
{
	pid_t main_tid = gettid();
	// struct thread_data pointer should be freed by pzip thread
	struct pzip_thread_data *td_ptr = (struct pzip_thread_data *) malloc(sizeof(struct pzip_thread_data));
	if (td_ptr == NULL)
	{
		fprintf(stderr, "out of memory\n");
		exit(1);
	}

	td_ptr->tid = 0;
	// struct pzip_section pointer should be freed by writer thread
	td_ptr->pz_sec_ptr = (struct pzip_section *) malloc(sizeof(struct pzip_section));
	if (td_ptr->pz_sec_ptr == NULL)
	{
		fprintf(stderr, "out of memory\n");
		exit(1);
	}

	td_ptr->pz_sec_ptr->buffer    = buffer;
	td_ptr->pz_sec_ptr->byte_from = from;
	td_ptr->pz_sec_ptr->byte_to   = to;

	PRINTF_PZIP(stderr, "\t(%d) starting thread...\n", main_tid);
	pthread_create(&td_ptr->tid, NULL, &pzip_compress_thread, td_ptr->pz_sec_ptr);
	PRINTF_PZIP(stderr, "\t(%d) thread# %lu  created\n", main_tid, td_ptr->tid);

	return td_ptr;
}

void * writer_thread(void *arg)
{
	struct writer_thread_args *wr_args = (struct writer_thread_args *) arg;
	size_t bytes_processed = 0;
	struct zipped_elem last_elem = {};

	PRINTF_WRITER(stderr, "writer thread qd_recv %d\n", wr_args->qd_writer_recv);
	do
	{
		// Recv message vi mq from pzip threads creator
		struct pzip_section *pz_sec_ptr = NULL;
		if (mq_receive(wr_args->qd_writer_recv, (char *) &pz_sec_ptr, sizeof(struct pzip_section *), NULL) == -1)
		{
			perror("mq_receive");
			exit(1);
		}
		PRINTF_WRITER(stderr, "writer thread received %p\n", pz_sec_ptr);

		// write to output file
		struct pzip_node *tail = pz_sec_ptr->tail;

		// if not first run
		if (last_elem.cnt != 0)
		{
			// add the last elem to the current linked list
			if (last_elem.c == tail->elem.c)                  // if same char
				tail->elem.cnt += last_elem.cnt;              //     then add counter
			else                                              // else
				tail = pzip_node_add_prev(tail, &last_elem);  //     then add to linked list
		}

		// copy last element of the received linked list
		memcpy(&last_elem, &pz_sec_ptr->head->elem, sizeof(last_elem));

		while (tail->next)
		{
			printf("%c%ld", tail->elem.c, tail->elem.cnt);
			tail = pzip_node_del_and_next(tail);
		}

		pzip_node_del_and_next(tail);

		// advance bytes processed
		bytes_processed += pz_sec_ptr->byte_to - pz_sec_ptr->byte_from;

		// free struct pzip_section pointer (allocated by create_pzip_thread())
		free(pz_sec_ptr);
	} while (bytes_processed < g_stream_input_sz);

	// write down last element
	printf("%c%ld\n", last_elem.c, last_elem.cnt);

	return NULL;
}

pthread_t create_reader_thread(struct reader_thread_args *rd_args)
{
	pthread_t ret = 0;
	pthread_create(&ret, NULL, &reader_thread, rd_args);
	return ret;
}

pthread_t create_zipper_thread(struct zipper_thread_args *zp_args)
{
	pthread_t ret = 0;
	pthread_create(&ret, NULL, &create_pzip_threads, zp_args);
	return ret;
}

pthread_t create_writer_thread(struct writer_thread_args *wr_args)
{
	pthread_t ret = 0;
	pthread_create(&ret, NULL, &writer_thread, wr_args);
	return ret;
}
