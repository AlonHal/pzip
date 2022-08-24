#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/sysinfo.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#include <pthread.h>

#include "pzip_linkedlist.h"
#include "pzip_files.h"
#include "pzip_msgq.h"
#include "pzip_threads.h"


// #define PRINTF_MAIN_ENABLED
#ifndef PRINTF_MAIN_ENABLED
#define PRINTF_MAIN(...)
#else
#define PRINTF_MAIN fprintf
#endif

void display_help(const char *appname)
{
	printf("Usage: %s arg1 [arg2...]\n", appname);
}

int main(int argc, char **argv)
{
	pid_t main_tid = gettid();
	pthread_t pth_id = 0;

	PRINTF_MAIN(stderr, "===== app starts here! =====\n");

	if (argc == 1)
	{
		display_help(argv[0]);
		return 0;
	}

	struct mq_fds mq_fds_rd_thread = {};
	if (create_message_queue(&mq_fds_rd_thread, "/jobs_from_reader_thr", 10, sizeof(struct zipper_thread_data *)) == -1)
		exit(1);
	struct mq_fds mq_fds_wr_thread = {};
	if (create_message_queue(&mq_fds_wr_thread, "/jobs_for_writer_thr", 10, sizeof(struct pzip_section *)) == -1)
		exit(1);

	struct reader_thread_args rd_args = 
	{
		.file_count = argc - 1,
		.file_paths = argv + 1,
		.qd_send    = mq_fds_rd_thread.mq_fd_send
	};
	pthread_t reader_thr = create_reader_thread(&rd_args);
	PRINTF_MAIN(stderr, "main loaded reader\n");

	struct zipper_thread_args zip_args =
	{
		.thread_count = get_nprocs(),
		.qd_reader_recv = mq_fds_rd_thread.mq_fd_recv,
		.qd_writer_send = mq_fds_wr_thread.mq_fd_send
	};
	pthread_t zipper_thr = create_zipper_thread(&zip_args);
	PRINTF_MAIN(stderr, "main loaded zipper\n");

	struct writer_thread_args wr_args =
	{
		.qd_writer_recv = mq_fds_wr_thread.mq_fd_recv
	};
	pthread_t writer_thr = create_writer_thread(&wr_args);
	PRINTF_MAIN(stderr, "main loaded writer\n");

	pthread_join(reader_thr, NULL);
	PRINTF_MAIN(stderr, "main reader done\n");

	pthread_join(zipper_thr, NULL);
	PRINTF_MAIN(stderr, "main zipper done\n");

	pthread_join(writer_thr, NULL);
	PRINTF_MAIN(stderr, "main writer done\n");

	return 0;
}
