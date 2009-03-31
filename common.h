#include <pthread.h>

typedef struct {
	int c;
	char **v;
} fsinit_arg;

extern int fsinit(void *);

