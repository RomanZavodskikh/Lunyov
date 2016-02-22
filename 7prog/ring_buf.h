#ifndef RING_BUF_H
#define RING_BUF_H 
#include "consts.h"

struct ring_buf
{
    //static characteristics
    char* buffer;
    char* eobuffer;
    unsigned long capacity;

    //dynamic characteristics
    char* start;
    char* end;
    int full;
};

ssize_t ring_buf_read(struct ring_buf* rb, int fd)
{
    //reads from ring buffer bytes and writes them to ->fd<- file
    unsigned long bytes_to_read = 0;
    unsigned long bytes_readen = 0;
    //fprintf(stderr, "RBR:%p...%p||s:%p<-->%p:e(full:%i)\n", rb->buffer,
    //    rb->eobuffer, rb->start, rb->end, rb->full);
    if (rb->start < rb->end)
    {
        bytes_to_read = rb->end - rb->start;
    }
    else if (rb->end < rb->start)
    {
        bytes_to_read = rb->eobuffer-rb->start;
    }
    else if (rb->full)
    {
        bytes_to_read = rb->eobuffer-rb->start;
    }
    else
    {
        //do nothing
    }
    
    bytes_to_read = (bytes_to_read>CH_BUF_SIZE)?CH_BUF_SIZE:bytes_to_read;
    bytes_readen = write_err(fd, rb->start, bytes_to_read);
    rb->start += bytes_readen;

    if (rb->start==rb->eobuffer)
    {
        rb->start = rb->buffer;
    }

    if (rb->full && bytes_readen != 0)
    {
        rb->full = 0;
    }

    return bytes_readen;
}

ssize_t ring_buf_write(struct ring_buf* rb, int fd)
{
    //writes to ring buffer bytes from ->fd<- file
    unsigned long bytes_to_write = 0;
    unsigned long bytes_written = 0;
    //fprintf(stderr, "RBW:%p...%p||s:%p<-->%p:e(full:%i)\n", rb->buffer,
    //    rb->eobuffer, rb->start, rb->end, rb->full);
    if (rb->start < rb->end)
    {
        bytes_to_write = rb->eobuffer - rb->end;
    }
    else if (rb->end < rb->start)
    {
        bytes_to_write = rb->start - rb->end;
    }
    else if (!rb->full)
    {
        bytes_to_write = rb->eobuffer - rb->end;
    }
    else
    {
        //do nothing
    }
    bytes_written = read_err(fd, rb->end, bytes_to_write);
    rb->end += bytes_written;

    if (rb->end == rb->eobuffer)
    {
        rb->end = rb->buffer;
    }

    if (rb->start == rb->end && bytes_written != 0)
    {
        rb->full = 1;
    }
    return bytes_written;
}

void ring_buf_ctor(struct ring_buf* rb, unsigned long cap)
{
    rb->buffer = calloc(cap, sizeof(char));
    rb->eobuffer = rb->buffer + cap;
    rb->capacity = cap;

    rb->start = rb->buffer;
    rb->end = rb->buffer;
    rb->full=0;
}

void ring_buf_dtor(struct ring_buf* rb)
{
    free(rb->buffer);
    rb->buffer=rb->eobuffer=rb->start=rb->end=NULL;
    rb->capacity=0;
    rb->full=0;
}
#endif //RING_BUF_H
