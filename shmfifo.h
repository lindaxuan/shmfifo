#ifndef __SHMFIFO_H__
#define __SHMFIFO_H__

#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

extern int errno;

#define ERR_EXIT(msg) \
do { \
	fprintf(stderr, "[%s][%d] %s : %s\n",__FILE__,__LINE__,\
						msg,strerror(errno)); \
	exit(EXIT_FAILURE); \
}while ( 0 )

typedef struct shmhead {
	unsigned int blksize;    // 块大小
	unsigned int blocks;     // 块总数
	unsigned int rd_index;   // 读索引
	unsigned int wr_index;   // 写索引
}shmhead_t;

typedef struct shmfifo {
	shmhead_t *p_shm; // 共享内存的头部地址 
	char *p_payload;  // 有效负载起始地址
	int shmid;        // 共享内存ID
	int sem_mutex;    // 用来互斥的信号量
	int sem_full;     // 用来控制共享内存满的信号量
	int sem_empty;    // 用来控制共享内存空的信号量
}shmfifo_t;

shmfifo_t *shmfifo_init(key_t key, int blksz, int blks);
void shmfifo_put(shmfifo_t *fifo, const void *buf);
void shmfifo_get(shmfifo_t *fifo, void *buf);
void shmfifo_destroy(shmfifo_t *fifo);

#endif //__SHMFIFO_H__

