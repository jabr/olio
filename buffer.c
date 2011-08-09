#include <stdlib.h>
#include <string.h>

#include "buffer.h"

/* a simple FIFO byte buffer */

void olio_buffer_pool_init_param(olio_buffer_pool * pool, 
				 uint16_t free_block_max, 
				 uint16_t block_size)
{
	pool->free_block_max = free_block_max;
	pool->block_size = block_size;

	pool->use_count = 0;
	pool->free_block_count = 0;
	pool->free_block_list = NULL;
	pool->block_data_size = pool->block_size
		- sizeof(olio_buffer_block)
		+ sizeof(uint8_t);
}

void olio_buffer_pool_init(olio_buffer_pool * pool)
{
	olio_buffer_pool_init_param(pool, 1024, 512);
}

void olio_buffer_pool_free(olio_buffer_pool * pool)
{
	olio_buffer_block *block1 = pool->free_block_list;
	olio_buffer_block *block2;

	while (block1 != NULL) {
		block2 = block1->next;
		free(block1);
		block1 = block2;
	}
}

static olio_buffer_block * _get_block(olio_buffer_pool * pool)
{
	olio_buffer_block * block = NULL;

	if (pool->free_block_count > 0) {
		block = pool->free_block_list;
		pool->free_block_list = block->next;
		pool->free_block_count--;
	} else {
#ifdef OLIO_DEBUG_BUFFER
		printf("-----------> MALLOC block\n");
#endif
		block = (olio_buffer_block *) malloc(pool->block_size);
	}
	return block;
}

static void _drop_block(olio_buffer_block * block, olio_buffer_pool * pool)
{
	if (pool->free_block_count < pool->free_block_max) {
		block->next = pool->free_block_list;
		pool->free_block_list = block;
		pool->free_block_count++;
	} else {
		free(block);
	}
}

/* contructor for a buffer */
int olio_buffer_init(olio_buffer * buf, olio_buffer_pool * pool)
{
	olio_buffer_block * block;

        pool->use_count++;
        buf->pool = pool;

	block = _get_block(buf->pool);
	if (block == NULL) {
		pool->use_count--;
		return -1;
	}

	block->next = NULL;
	block->start = block->end = 0;
	buf->head = buf->tail = block;

        buf->length = 0;

	return 0;
}

/* destructor for a buffer */
void olio_buffer_free(olio_buffer * buf)
{
	olio_buffer_block *block1 = buf->head;
	olio_buffer_block *block2;

	while (block1 != NULL) {
		block2 = block1->next;
		_drop_block(block1, buf->pool);
		block1 = block2;
	}

	buf->pool->use_count--;
}

/* clear the contents of the buffer */
void olio_buffer_clear(olio_buffer * buf)
{
	olio_buffer_block *block1 = buf->head;
	olio_buffer_block *block2;

	while (block1 != buf->tail) {
		block2 = block1->next;
		_drop_block(block1, buf->pool);
		block1 = block2;
	}

	buf->head = buf->tail;
	buf->head->start = buf->head->end = 0;
	buf->length = 0;
}

int olio_buffer_add(olio_buffer * buf, const void * data, uint32_t len)
{
	uint32_t off = 0;

	/* return if no data to add */
	if (len == 0) return 0;

	/* any room left at the end of our tail block? */
	if (buf->tail->end < buf->pool->block_data_size) {
		uint32_t amt = buf->pool->block_data_size - buf->tail->end;
		if (len < amt) amt = len;
		memcpy(buf->tail->data + buf->tail->end, data, amt);
		len -= amt;
		off += amt;
		buf->length += amt;
		buf->tail->end += amt;
	}

	/* add new blocks to hold the rest */
	while (len > 0) {
		uint32_t amt = buf->pool->block_data_size;
		olio_buffer_block * block = _get_block(buf->pool);
		if (block == NULL) return -1;
		if (len < amt) amt = len;
		block->start = 0;
		block->end = amt;
		block->next = NULL;
		buf->tail->next = block;
		buf->tail = block;
		memcpy(block->data, (const char *) data + off, amt);
		len -= amt;
		off += amt;
		buf->length += amt;
	}

	return 0;
}

/* remove "len" bytes from the head of the buffer */
void olio_buffer_consume(olio_buffer * buf, uint32_t len)
{
	/* if they're taking everything, just call clear */
	if (len >= buf->length) {
		olio_buffer_clear(buf);
		return;
	}

	while (len > 0) {
		/* how much is in this block? */
		uint32_t amt = buf->head->end - buf->head->start;
		/* more than we need? */
		if (len < amt) {
			/* yes, so just take the remainder... */
			amt = len;
			/* and move up the head block's start index */
			buf->head->start += amt;
		} else {
			/* drop the whole block */
			olio_buffer_block * block = buf->head;
			buf->head = buf->head->next;
			_drop_block(block, buf->pool);
		}
		len -= amt;
		buf->length -= amt;
	}
}

/* copies "len" bytes from the start of the buffer into 
 * a preallocated array "data" */
uint16_t olio_buffer_copy(olio_buffer * buf, void * data, uint16_t len)
{
	uint16_t i = 0;
	olio_buffer_block * current = buf->head;
	uint16_t offset = buf->head->start;
	uint8_t * bp = (uint8_t *) data;

	while (i < len) {
		if (offset == current->end) {
			current = current->next;
			if (current == NULL) break;
			offset = current->start;
		}
		bp[i++] = current->data[offset++];
	}

	return i;
}

/* copies "len" bytes from the start of the buffer into
 * an array we (potentially) allocate for the caller.
 * if len == 0, duplicate the entire contents of the buffer. */
uint16_t olio_buffer_duplicate(olio_buffer * buf, void ** data, uint16_t len)
{
	if (len == 0) len = buf->length;

	*data = (void*) realloc(*data, sizeof(uint8_t) * len);
	if (*data == NULL) return 0;

	return olio_buffer_copy(buf, *data, len);
}
