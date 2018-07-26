#include "shmfifo.h"

typedef struct stu {
	int id;
	char name[32];
}stu_t;

int main( void )
{
	int i;
	shmfifo_t *p = shmfifo_init(1234, sizeof(stu_t), 3);
	stu_t buf[8] = {
				{1, "张海涛"},
				{2, "王俊杰"},
				{3, "邢理博"},
				{4, "李松泽"},
				{5, "aaaaa"},
				{6, "bbbbbb"},
				{7, "ccccc"},
				{8, "ddddd"}
			};
	for (i=0; i<8; i++) {
		shmfifo_put(p, buf+i);
		printf("完成!\n");
	}
}

