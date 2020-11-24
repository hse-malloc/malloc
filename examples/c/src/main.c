#include <stdio.h>

int main() {

    int *ptr = (int*)malloc(sizeof(int));

    printf("allocated %ld bytes as %p\n", sizeof(int), (void*)ptr);
    printf("uninitialized int: %d\n", *ptr);

    *ptr = 1;

    printf("value was set to 1: %d\n", *ptr);
    fflush(stdout);
    free(ptr);
    printf("ptr is free\n");
    return 0;
}
