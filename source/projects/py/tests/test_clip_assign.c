
#include <stdio.h>

#define CLIP_ASSIGN(x,a,b) (x)=(x)<(a)?(a):(x)>(b)?(b):(x)


// gcc -E test_clip_assign.c
// gcc -o tesT_clip test_clip_assign.c


int clamp(int x, int min, int max)
{
	if (x < min)
		return min;
	if (x > max)
		return max;
	return x;
}

int main()
{
	int argc = 10;
	int dim0 = 5;
	int dim1 = 5;
	int offset0 = 0;
	int offset1 = 1;

	CLIP_ASSIGN(argc, 0, (dim0 * (dim1 - offset1)) - offset0);

	printf("argc = %d\n", argc);

	argc = clamp(argc, 0, (dim0 * (dim1 - offset1)) - offset0);

	printf("argc = %d\n", argc);

	return 0;
}

