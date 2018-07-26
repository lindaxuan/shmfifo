#include "shmfifo.h"

typedef struct stu {
	int id;
	char name[32];
}stu_t;

int main( void )
{
	int i;
	stu_t buf;
	shmfifo_t *p = shmfifo_init(1234, sizeof(stu_t), 3);
	for (i=0; i<3; i++) {
		shmfifo_get(p, &buf);
		printf("%d %s\n", buf.id, buf.name);
	}
	
	//shmfifo_destroy(p);
}

