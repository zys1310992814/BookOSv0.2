#include <stdio.h>
#include <stdlib.h>
#include <sys/pipe.h>
int main(int argc, char *argv[])
{
	uint64_t data = new_pipe(100);
	printf("testtttt");
	close_pipe(data);
	return 0;
}
