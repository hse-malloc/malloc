#include <malloc/malloc.h>
#include <stdio.h>

int main() {

    int *ptr = (int*)hse_malloc(sizeof(int));

    printf("allocated %ld bytes as 0x%p\n", sizeof(int), (void*)ptr);
    printf("uninitialized int: %d\n", *ptr);

    *ptr = 1;

    printf("value was set to 1: %d\n", *ptr);
    hse_free(ptr);
    printf("freed at 0x%p\n", (void*)ptr);
    return 0;
}
