/*
* Copyright 2017 NetSync.tv Grey Huang (deepkh@gmail.com) All rights reserved. 
* license: GNU Lesser General Public License v2.1 
*/

#ifndef _FF_BUFFER_
#define _FF_BUFFER_

struct ff_buffer {
	char *buf_head;
	char *buf_tail;

	char *data_head;
	char *data_tail;
	
	int block_size;

	char tmp[4096];
};

struct ff_buffer *ff_buffer_new(int block_size);
int ff_buffer_free(struct ff_buffer *p);
int ff_buffer_reset(struct ff_buffer *p);

int ff_buffer_copy(struct ff_buffer *dst, struct ff_buffer *src);
char *ff_buffer_data(struct ff_buffer *p);
int ff_buffer_data_size(struct ff_buffer *p);
char *ff_buffer_buf(struct ff_buffer *p);
int ff_buffer_buf_size(struct ff_buffer *p);

int ff_buffer_push(struct ff_buffer *p, char *buf, int size);
int ff_buffer_push_sprintf(struct ff_buffer *p, char *fmt, ...);
int ff_buffer_pop(struct ff_buffer *p, char *buf, int size);
int ff_buffer_remove(struct ff_buffer *p, int size);

void ff_buffer_test(int count, int block_size);
#endif
