#ifndef _OLIO_BUFFER_H_
#define _OLIO_BUFFER_H_

#include <inttypes.h>

typedef struct _olio_buffer_block {
	struct _olio_buffer_block * next;
	uint16_t start;
	uint16_t end;
	uint8_t data[1];
} olio_buffer_block;

typedef struct _olio_buffer_pool {
	uint16_t free_block_count;
	uint16_t free_block_max;
	olio_buffer_block * free_block_list;
	uint16_t block_size;
	uint16_t block_data_size;
	uint16_t use_count;
} olio_buffer_pool;

typedef struct _olio_buffer {
	olio_buffer_pool * pool;
	olio_buffer_block * head;
	olio_buffer_block * tail;
	uint32_t length;
} olio_buffer;

#ifdef __cplusplus
extern "C" {
#endif

void olio_buffer_pool_init(olio_buffer_pool *);
void olio_buffer_pool_init_param(olio_buffer_pool *,
				 uint16_t free_block_max,
				 uint16_t block_size);
void olio_buffer_pool_free(olio_buffer_pool *);

int olio_buffer_init(olio_buffer *, olio_buffer_pool * pool);
void olio_buffer_free(olio_buffer *);
void olio_buffer_clear(olio_buffer * buf);

int olio_buffer_add(olio_buffer *, const void * data, uint32_t len);
void olio_buffer_consume(olio_buffer *, uint32_t len);
uint16_t olio_buffer_copy(olio_buffer *, void * data, uint16_t len);
uint16_t olio_buffer_duplicate(olio_buffer *, void ** data, uint16_t len);

#ifdef __cplusplus
} /* extern C */
#endif

static inline uint32_t olio_buffer_length(olio_buffer * buf) 
{
	return buf->length;
}

static inline uint16_t olio_buffer_readable(olio_buffer * buf, void ** data)
{
        /* set the data to point to remainder of the head block's data */
        *data = buf->head->data + buf->head->start;
        /* return the length of the data remaining in the head block */
        return (buf->head->end - buf->head->start);
}

#endif /* _OLIO_BUFFER_H_ */
