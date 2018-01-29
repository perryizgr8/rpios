#include <stddef.h>
#include <stdint.h>
#include <kernel/uart.h>
#include <kernel/mem.h>
#include <kernel/atag.h>
#include <common/stdio.h>
#include <common/stdlib.h>

void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags)
{
    char buf[256];
    (void) r0;
    (void) r1;
    (void) atags;

    uart_init();
    puts("Initializing memory module...\r\n");
    mem_init((atag_t *)atags);

    puts("Hello, kernel world!\r\n");
    while(1) {
        gets(buf, 256);
        puts(buf);
        putc('\n');
    }
}
