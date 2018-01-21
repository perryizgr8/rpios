#include <common/stdlib.h>

void memcpy(void *dest, void *src, int bytes) {
    char *d = dest;
    char *s = src;
    while(bytes--) {
        *d++ = *s++;
    }
}

void bzero(void *dest, int bytes) {
    char *d = dest;
    while(bytes--) {
        *d++ = 0;
    }
}
