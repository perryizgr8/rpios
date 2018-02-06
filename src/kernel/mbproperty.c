#include <kernel/mbproperty.h>
#include <kernel/mem.h>
#include <common/stdlib.h>

mail_message_t mailbox_read(void)
{
    mail_status_t stat;
    mail_message_t res;

    //printf("%x,%x\n", MAIL0_STATUS, MAIL0_READ);
    // Make sure msg is from the right channel.
    do
    {
        // Make sure there is mail to receive.
        do
        {
            stat = *MAIL0_STATUS;
        } while (stat.empty);

        // Get the message.
        res = *MAIL0_READ;
    } while (res.channel != PROPERTY_CHANNEL);
    return res;
}

void mailbox_send(mail_message_t msg)
{
    mail_status_t stat;
    msg.channel = PROPERTY_CHANNEL;

    // Make sure we can send mail.
    do
    {
        stat = *MAIL0_STATUS;
    } while (stat.full);

    // Send the message.
    *MAIL0_WRITE = msg;
}

// Returns max of req and result buffer of each tag
static uint32_t get_value_buffer_len(property_message_tag_t *tag)
{
    switch (tag->proptag)
    {
    case FB_ALLOCATE_BUFFER:
    case FB_GET_PHYSICAL_DIMENSIONS:
    case FB_SET_PHYSICAL_DIMENSIONS:
    case FB_GET_VIRTUAL_DIMENSIONS:
    case FB_SET_VIRTUAL_DIMENSIONS:
        return 8;
    case FB_GET_BITS_PER_PIXEL:
    case FB_SET_BITS_PER_PIXEL:
    case FB_GET_BYTES_PER_ROW:
        return 4;
    case FB_RELEASE_BUFFER:
    default:
        return 0;
    }
}

int send_messages(property_message_tag_t *tags)
{
    property_message_buffer_t *msg;
    mail_message_t mail;
    uint32_t bufsize = 0, i, len, bufpos;

    // Calculate the size of each tag
    for (i = 0; tags[i].proptag != NULL_TAG; i++)
    {
        bufsize += get_value_buffer_len(&tags[i]) + 3 * sizeof(uint32_t);
        // TODO What's this 3*uint32_t?
    }

    // Add the buffer size, buffer req/resp code and buffer end tag sizes.
    bufsize += 3 * sizeof(uint32_t);

    // Align the size to 16 bytes
    bufsize += (bufsize % 16) ? 16 - (bufsize % 16) : 0;

    msg = kmalloc(bufsize);
    if (!msg)
    {
        //printf("Malloc failed for msg.\n");
        return -1;
    }

    msg->size = bufsize;
    msg->req_res_code = REQUEST;

    // Copy the messages into the buffer
    for (i = 0, bufpos = 0; tags[i].proptag != NULL_TAG; i++)
    {
        len = get_value_buffer_len(&tags[i]);
        msg->tags[bufpos++] = tags[i].proptag;
        msg->tags[bufpos++] = len;
        msg->tags[bufpos++] = 0;
        memcpy(msg->tags + bufpos, &tags[i].value_buffer, len);
        bufpos += len / 4;
    }

    msg->tags[bufpos] = 0;

    // Send the message
    mail.data = ((uint32_t)msg) >> 4;

    mailbox_send(mail);
    mail = mailbox_read();

    if (msg->req_res_code = REQUEST)
    {
        //printf("Send failed.\n");
        kfree(msg);
        return 1;
    }

    if (msg->req_res_code == RESPONSE_ERROR)
    {
        //printf("Error response received.\n");
        kfree(msg);
        return 2;
    }

    //Copy tags back into the array.
    for (i = 0, bufpos = 0; tags[i].proptag != NULL_TAG; i++)
    {
        len = get_value_buffer_len(&tags[i]);
        bufpos += 3; // Skip over the bookkeeping
        memcpy(&tags[i].value_buffer, msg->tags + bufpos, len);
        bufpos += len / 4;
    }

    kfree(msg);
    return 0;
}