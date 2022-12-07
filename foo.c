#include "defs.h"

void main()
{
	int t, x, i, j, a, b, c;

    for (i = 0; i < 1000000; i++)
    {
        t = rand() % 8;

        for (j = 1; j < rand() % 20; j++)
        {
            x = rand() % 8192;
            b = rand() % 64;

            a = ((t & TAG_MASK) << TAG_SHIFT) | ((x & ID_MASK) << ID_SHIFT) | b;
            c = a % 3;
            printf("%d %x\n", c, a);
        }    
    }

    /* simple fill all buckets twice to test fill, evict */
/*
    i = 1;
    b = 3;

	for (t = 1; t < 16; t++)
	{
		a = ((t & TAG_MASK) << TAG_SHIFT) | ((i & ID_MASK) << ID_SHIFT) | b;
		printf("0 %x\n", a);
	}
*/

}
