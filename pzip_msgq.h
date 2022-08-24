#ifndef _PZIP_MSGQ_H_
#define _PZIP_MSGQ_H_

#include <mqueue.h>

struct mq_fds
{
	mqd_t mq_fd_recv;
	mqd_t mq_fd_send;
};

int create_message_queue(struct mq_fds *p_ret, const char *vritual_path, int maxmsg, int msgsize);

#endif // _PZIP_MSGQ_H_