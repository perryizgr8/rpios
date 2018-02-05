#include <kernel/atag.h>

uint32_t get_mem_size(atag_t *tag)
{
// If MODEL_1 is defined it means I'm using hardware, otherwise qemu, which doesn't support atags.
#ifdef MODEL_1
    while (tag->tag != NONE)
    {
        if (tag->tag == MEM)
        {
            return tag->mem.size;
        }
        tag = (atag_t *)(((uint32_t *)tag) + tag->tag_size);
    }
    return 0;
#else
    return 128 * 1024 * 1024;
#endif
}
