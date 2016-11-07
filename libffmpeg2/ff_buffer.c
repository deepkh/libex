/*
* Copyright 2017 NetSync.tv Grey Huang (deepkh@gmail.com) All rights reserved. 
* license: GNU Lesser General Public License v2.1 
*/

#include "ff_buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
/**
  private
  */
static int free_buf(struct ff_buffer *p)
{
	if (p->buf_head) {
		free(p->buf_head);
	}
	p->buf_head = 0;
	p->buf_tail = 0;
	p->data_head = 0;
	p->data_tail = 0;

	return 1;
}

static int increase_buf(struct ff_buffer *p, int size)
{
	int remain_size = p->buf_tail - p->data_tail;
	int data_size = ff_buffer_data_size(p);
	char *buf_new = 0;
	int buf_size_new = 0;
	
	if (remain_size <= size) {
		buf_size_new = ((data_size + size)/p->block_size + 1) * p->block_size;
#ifdef __APPLE__
		//very very very slow
		buf_new = (char *) malloc(buf_size_new);//calloc(1, buf_size_new);//malloc(buf_size_new);//calloc(1, buf_size_new);
		memcpy(buf_new, ff_buffer_data(p), data_size);
		free_buf(p);
#else
		buf_new = (char *) realloc(p->buf_head, buf_size_new);//malloc(buf_size_new);//calloc(1, buf_size_new);
#endif

		p->buf_head = buf_new;
		p->buf_tail = p->buf_head + buf_size_new;

		p->data_head = buf_new;
		p->data_tail = p->data_head + data_size;
	}

	return (p->buf_tail - p->data_tail) - remain_size;		//increase_size
}

static int decrease_buf(struct ff_buffer *p, int size)
{
	int data_size = ff_buffer_data_size(p);
	if (data_size <= size) {
		free_buf(p);
		return data_size;
	}
	p->data_head += size;
	return size;
}

/**
  public
  */
struct ff_buffer *ff_buffer_new(int block_size)
{
	struct ff_buffer *p = (struct ff_buffer *) calloc(1, sizeof(struct ff_buffer));
	p->block_size = block_size;
	return p;
}

int ff_buffer_free(struct ff_buffer *p)
{
	if (p) {
		free_buf(p);
		free(p);
		return 1;
	}
	return 0;
}

int ff_buffer_reset(struct ff_buffer *p)
{
	if (!p) {
		return -1;
	}

	free_buf(p);
	return 0;
}

int ff_buffer_copy(struct ff_buffer *dst, struct ff_buffer *src)
{
	if (!src || !dst) {
		return -1;
	}

	ff_buffer_push(dst, ff_buffer_data(src), ff_buffer_data_size(src));
	return 0;
}

char *ff_buffer_data(struct ff_buffer *p)
{
	return p->data_head;
}

int ff_buffer_data_size(struct ff_buffer *p)
{
	return p->data_tail-p->data_head;
}

char *ff_buffer_buf(struct ff_buffer *p)
{
	return p->buf_head;
}

int ff_buffer_buf_size(struct ff_buffer *p)
{
	return p->buf_tail-p->buf_head;
}

int ff_buffer_push(struct ff_buffer *p, char *buf, int size)
{
	increase_buf(p, size);
	memcpy(p->data_tail, buf, size);
	p->data_tail[size] = '\0';
	p->data_tail += size;
	return size;
}

int ff_buffer_push_sprintf(struct ff_buffer *p, char *fmt, ...)
{
	//char buf[4096] = {0};
	va_list args;

	va_start(args,fmt);
	//vsprintf(p->tmp, fmt, args);
	vsnprintf(p->tmp, sizeof(p->tmp), fmt, args);
	va_end (args);
	return ff_buffer_push(p, p->tmp, strlen(p->tmp));
}

int ff_buffer_pop(struct ff_buffer *p, char *buf, int size)
{
	int pop_size = size;
	
	if (ff_buffer_data_size(p) < pop_size) {
		pop_size = ff_buffer_data_size(p);
	}

	if (buf) {
		memcpy(buf, ff_buffer_data(p), pop_size);
	}

	decrease_buf(p, pop_size);
	return pop_size;
}

int ff_buffer_remove(struct ff_buffer *p, int size)
{
	return decrease_buf(p, size);
}
