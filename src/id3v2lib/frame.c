/*
 * This file is part of the id3v2lib library
 *
 * Copyright (c) 2013, Lorenzo Ruiz
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "frame.h"
#include "utils.h"
#include "constants.h"

#include "../options.h"

ID3v2_frame *parse_frame(char *bytes, int offset, int version, int max)
{
    ID3v2_frame *frame = new_frame();
    
    if (frame == NULL)
        return NULL;

    // Parse frame header
    memcpy (frame->frame_id, bytes + offset, ID3_FRAME_ID);
    // Check if we are into padding
    if (memcmp (frame->frame_id, "\0\0\0\0", 4) == 0)
    {
        free_frame(frame);
        return NULL;
    }

    frame->size = btoi (bytes, 4, offset += ID3_FRAME_ID);
    if (version == ID3v24)
    {
        frame->size = syncint_decode (frame->size);
    }

    memcpy (frame->flags, bytes + (offset += ID3_FRAME_SIZE), 2);
    
    // Load frame data
    if (frame->size > max || frame->size < 0)
    {
        fprintf (stderr, "reached max size\n"); // error
        free_frame(frame);
        return NULL;
    }
    frame->data = (char *) malloc (frame->size  *sizeof (char));

    if (frame->data == NULL)
    {
        fprintf (stderr, "%s: out of memory\n", prog);
        free_frame(frame);
        return NULL;
    }

    memcpy (frame->data,
            bytes + (offset += ID3_FRAME_FLAGS), frame->size);
    
    return frame;
}

int get_frame_type(char* frame_id)
{
    switch(frame_id[0])
    {
        case 'T':
            return TEXT_FRAME;
        case 'C':
            return COMMENT_FRAME;
        case 'A':
            return APIC_FRAME;
        default:
            return INVALID_FRAME;
    }
}

ID3v2_frame_text_content* parse_text_frame_content(ID3v2_frame* frame)
{
    if(frame == NULL)
    {
        return NULL;
    }
    
    ID3v2_frame_text_content* content = new_text_content(frame->size);
    content->encoding = frame->data[0];
    content->size = frame->size - ID3_FRAME_ENCODING;
    memcpy(content->data, frame->data + ID3_FRAME_ENCODING, content->size);
    return content;
}

ID3v2_frame_comment_content* parse_comment_frame_content(ID3v2_frame* frame)
{
    if(frame == NULL)
    {
        return NULL;
    }
    
    ID3v2_frame_comment_content* content = new_comment_content(frame->size);
    
    content->text->encoding = frame->data[0];
    content->text->size = frame->size - ID3_FRAME_ENCODING - ID3_FRAME_LANGUAGE - ID3_FRAME_SHORT_DESCRIPTION;
    memcpy(content->language, frame->data + ID3_FRAME_ENCODING, ID3_FRAME_LANGUAGE);
    content->short_description = "\0"; // Ignore short description
    memcpy(content->text->data, frame->data + ID3_FRAME_ENCODING + ID3_FRAME_LANGUAGE + 1, content->text->size);
    
    return content;
}

char* parse_mime_type(char* data, int* i)
{
    char* mime_type = (char*) malloc(30 * sizeof(char));
    
    while(data[*i] != '\0')
    {
        mime_type[*i - 1] = data[*i];
        (*i)++;
    }
    
    return mime_type;
}

ID3v2_frame_apic_content* parse_apic_frame_content(ID3v2_frame* frame)
{
    if (frame == NULL)
        return NULL;

    if (frame->size <= 0 || frame->data == NULL)
        return NULL;

    ID3v2_frame_apic_content* content = new_apic_content();

    if (content == NULL)
        return NULL;

    content->encoding = frame->data[0];

    // hacked together header skipping not from id3v2lib
    int i = 1; // Skip ID3_FRAME_ENCODING
    while (frame->data [i++] != '\00' && i < frame->size) ; // skip mime type
    i++; // skip picture type
    if (i >= frame->size)
        return content;
    while (frame->data [i++] != '\00' && i < frame->size) ; // skip description
    content->picture_size = frame->size - i;
    if (i >= content->picture_size)
        return content;

    content->data = (char*) malloc(content->picture_size);
    memcpy(content->data, frame->data + i, content->picture_size);

    return content;
}
