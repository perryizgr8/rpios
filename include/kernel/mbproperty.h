#ifndef MBPROPERTY_H
#define MBPROPERTY_H

#include <stdint.h>
#include <kernel/peripheral.h>

#define MAILBOX_BASE PERIPHERAL_BASE + MAILBOX_OFFSET
#define MAIL0_READ (((mail_message_t *)(0x00 + MAILBOX_BASE)))
#define MAIL0_STATUS (((mail_status_t *)(0x18 + MAILBOX_BASE)))
#define MAIL0_WRITE (((mail_message_t *)(0x20 + MAILBOX_BASE)))
#define PROPERTY_CHANNEL 8

typedef struct
{
    uint8_t channel : 4;
    uint32_t data : 28;
} mail_message_t;

typedef struct
{
    uint32_t reserved : 30;
    uint8_t empty : 1;
    uint8_t full : 1;
} mail_status_t;

mail_message_t mailbox_read(void);
void mailbox_send(mail_message_t msg);

// A property msg can be a request or a response.
// And a response can be successful or an error
typedef enum {
    REQUEST = 0x00000000,
    RESPONSE_SUCCESS = 0x80000000,
    RESPONSE_ERROR = 0x80000001
} buffer_req_res_code_t;

// A buffer that holds many property messages.
// The last tag must be 4 byte of zeros, and then padding to make the whole thing 4-byte aligned.
typedef struct
{
    uint32_t size; // Including the size itself
    buffer_req_res_code_t req_res_code;
    uint32_t tags[1]; // A sequence of tags, not just one
} property_message_buffer_t;

// A message is identified by a tag. These are some possible tags.
typedef enum {
    NULL_TAG = 0,
    FB_ALLOCATE_BUFFER = 0x00040001,
    FB_RELEASE_BUFFER = 0x00048001,
    FB_GET_PHYSICAL_DIMENSIONS = 0x00040003,
    FB_SET_PHYSICAL_DIMENSIONS = 0x00048003,
    FB_GET_VIRTUAL_DIMENSIONS = 0x00040004,
    FB_SET_VIRTUAL_DIMENSIONS = 0x00048004,
    FB_GET_BITS_PER_PIXEL = 0x00040005,
    FB_SET_BITS_PER_PIXEL = 0x00048005,
    FB_GET_BYTES_PER_ROW = 0x00040008 // pitch = bytes per row
} property_tag_t;

// For each tag, we create a struct for the req value buffer, and the response value buffer.
typedef struct
{
    uint32_t alignment;
} fb_allocate_req_t;

typedef struct
{
    void *fb_addr;
    uint32_t fb_size;
} fb_allocate_res_t;

typedef struct
{
    uint32_t width;
    uint32_t height;
} fb_screen_size_t;

typedef struct
{
    uint32_t bits_per_pixel;
} fb_bits_per_pixel_t;

typedef struct
{
    uint32_t bytes_per_row;
} fb_bytes_per_row_t;

// The value buffer can be any one of these types
typedef union {
    fb_allocate_req_t fb_allocate_req;
    fb_allocate_res_t fb_allocate_res;
    fb_screen_size_t fb_screen_size;
    fb_bits_per_pixel_t fb_bits_per_pixel;
    fb_bytes_per_row_t fb_bytes_per_row;
} value_buffer_t;

// A message_buffer can contain any number of these
typedef struct
{
    property_tag_t proptag;
    value_buffer_t value_buffer;
} property_message_tag_t;

// Given an array of tags, will send all of the tags given, and will populate that array with the responses.
// The given array should end with a "null tag" with the proptag field set to 0.
// Returns 0 on success
int send_messages(property_message_tag_t *tags);

#endif