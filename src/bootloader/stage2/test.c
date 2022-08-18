#include "stdio.h"

#include "memory.h"

typedef struct
{
    int a[3];
} temp;

int main()
{
    
    int a[3] = {1,2,3};
    int b[3];
    memcpy(b,a, 3*sizeof(int));
    for(int i= 0; i < 3; i++)
	printf("%d ", b[i]);
   
    

    return 0;
}

char f(char *ar[2])
{
    return *ar[2];
}
