#include <assert.h>

#include "shmfifo.h"

union semun { int val; };

shmfifo_t *shmfifo_init(key_t key, int blksz, int blks)
{
	shmfifo_t *fifo = malloc(sizeof(shmfifo_t));
	assert(fifo);
	memset(fifo, 0x00, sizeof(shmfifo_t));
	
	int shmid = shmget(key, 0, 0);
	int size = sizeof(shmhead_t)+blksz*blks;
	if ( shmid == -1 ) { // 新创建
		shmid = shmget(key, size, IPC_CREAT|0666);
		if ( shmid == -1 ) ERR_EXIT("shmget");
		fifo->shmid = shmid;
		fifo->p_shm = shmat(shmid, NULL, 0);
		fifo->p_payload = (char*)(fifo->p_shm+1);

		fifo->p_shm->blksize  = blksz;
		fifo->p_shm->blocks   = blks;
		fifo->p_shm->rd_index = 0;
		fifo->p_shm->wr_index = 0;
		
		fifo->sem_mutex = semget(key, 1, 0644|IPC_CREAT);
		fifo->sem_full  = semget(key+1, 1, 0644|IPC_CREAT);
		fifo->sem_empty = semget(key+2, 1, 0644|IPC_CREAT);
		
		union semun su;
		su.val = 1;
		semctl(fifo->sem_mutex, 0, SETVAL, su);
		su.val = 0;
		semctl(fifo->sem_empty, 0, SETVAL, su);
		su.val = blks;
		semctl(fifo->sem_full, 0, SETVAL, su);
	} else { // 打开
		fifo->shmid = shmid;
		fifo->p_shm = shmat(shmid, NULL, 0);
		fifo->p_payload = (char*)(fifo->p_shm+1);
		fifo->sem_mutex = semget(key, 0, 0);
		fifo->sem_full  = semget(key+1, 0, 0);
		fifo->sem_empty = semget(key+2, 0, 0);
	}

	return fifo;
}

int sem_p(int semid)
{
	struct sembuf buf[1] = {0, -1, 0};
	return semop(semid, buf, 1);
}

int sem_v(int semid)
{
	struct sembuf buf[1] = {0, 1, 0};
	return semop(semid, buf, 1);
}

void shmfifo_put(shmfifo_t *fifo, const void *buf)
{
	sem_p(fifo->sem_full);
	sem_p(fifo->sem_mutex);

	memcpy(fifo->p_payload+fifo->p_shm->wr_index*fifo->p_shm->blksize, buf, fifo->p_shm->blksize);
	fifo->p_shm->wr_index = (fifo->p_shm->wr_index+1) % fifo->p_shm->blocks;

	sem_v(fifo->sem_mutex);
	sem_v(fifo->sem_empty);
}

void shmfifo_get(shmfifo_t *fifo, void *buf)
{
	sem_p(fifo->sem_empty);
	sem_p(fifo->sem_mutex);

	memcpy(buf, fifo->p_payload+fifo->p_shm->rd_index*fifo->p_shm->blksize, fifo->p_shm->blksize);
	fifo->p_shm->rd_index = (fifo->p_shm->rd_index+1) % fifo->p_shm->blocks;

	sem_v(fifo->sem_mutex);
	sem_v(fifo->sem_full);
}

void shmfifo_destroy(shmfifo_t *fifo)
{
	semctl(fifo->sem_mutex, 0, IPC_RMID, 0);
	semctl(fifo->sem_empty, 0, IPC_RMID, 0);
	semctl(fifo->sem_full,  0, IPC_RMID, 0);
	shmdt(fifo->p_shm);
	shmctl(fifo->shmid, IPC_RMID, 0);
	free(fifo);
}

