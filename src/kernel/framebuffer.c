#include <kernel/framebuffer.h>
#include <kernel/mem.h>
#include <kernel/kerio.h>
#include <kernel/mbproperty.h>
#include <common/stdlib.h>

static framebuffer_info_t fbinfo;
const pixel_t RED = {0xff, 0x00, 0x00};
const pixel_t GREEN = {0x00, 0xff, 0x00};
const pixel_t BLUE = {0x00, 0x00, 0xff};

static int try_init_framebuffer(void)
{
    property_message_tag_t tags[4];

    tags[0].proptag = FB_SET_PHYSICAL_DIMENSIONS;
    tags[0].value_buffer.fb_screen_size.width = 640;
    tags[0].value_buffer.fb_screen_size.height = 480;

    tags[1].proptag = FB_SET_VIRTUAL_DIMENSIONS;
    tags[1].value_buffer.fb_screen_size.width = 640;
    tags[1].value_buffer.fb_screen_size.height = 480;

    tags[2].proptag = FB_SET_BITS_PER_PIXEL;
    tags[2].value_buffer.fb_bits_per_pixel.bits_per_pixel = 24;

    tags[3].proptag = NULL_TAG;

    // Send the init request
    printf("Initializing...\n");
    if (send_messages(tags) != 0)
    {
        printf("Init failed!\n");
        return -1;
    }

    printf("Initialized :)\n");
    fbinfo.width = tags[0].value_buffer.fb_screen_size.width;
    fbinfo.height = tags[0].value_buffer.fb_screen_size.height;
    printf("Can display at %dx%d\n", fbinfo.width, fbinfo.height);
    fbinfo.pitch = 640; //TODO seems wrong, should be 640*3

    // Request a framebuffer
    tags[0].proptag = FB_ALLOCATE_BUFFER;
    tags[0].value_buffer.fb_screen_size.width = 0;
    tags[0].value_buffer.fb_screen_size.height = 0;
    tags[0].value_buffer.fb_allocate_req.alignment = 16;

    tags[1].proptag = NULL_TAG;

    printf("Allocation request received\n"); //TODO ???
    if (send_messages(tags) != 0)
    {
        printf("Couldn't request framebuffer.\n");
        return -1;
    }
    printf("Allocation request granted.\n");

    fbinfo.buf = tags[0].value_buffer.fb_allocate_res.fb_addr;
    fbinfo.buf_size = tags[0].value_buffer.fb_allocate_res.fb_size;
    printf("Granted buffer at %x of size %d.\n", fbinfo.buf, fbinfo.buf_size);
    return 0;
}

static void write_pixel(uint32_t x, uint32_t y, const pixel_t *pix)
{
    pixel_t *location = fbinfo.buf + y * fbinfo.pitch + x * 3;
    memcpy(location, pix, sizeof(pixel_t));
}

void gpu_init(void)
{
    // This sometimes fails, so try in a loop.
    while (try_init_framebuffer())
        ;

    // Try to draw something.
    printf("Drawing colored squares soon...\n");
    for (volatile uint32_t yolo = 0; yolo < 1000000000; yolo++)
        ;
    for (int i = 0; i < 100; i++)
    {
        for (int j = 0; j < 100; j++)
        {
            write_pixel(i, j, &BLUE);
        }
    }
    printf("Drawn. Can you see them?\n");
}
