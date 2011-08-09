#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "skiplist.h"

#define M OLIO_SKIPLIST_BLOCK_M
#define M2 (M/2)

static olio_skiplist_block * _new_block(olio_skiplist * list)
{
	return (olio_skiplist_block *) malloc(sizeof(olio_skiplist_block));
}

static void _free_block(olio_skiplist * list, olio_skiplist_block * block)
{
	free(block);
}

static void _shift_right(olio_skiplist_block * block, uint8_t i)
{
	assert(block->count < M - 1);
	memmove(block->entry + i + 1, block->entry + i, 
		(block->count - i) * sizeof(olio_skiplist_entry));
	block->count++;
}

static void _shift_left(olio_skiplist_block * block, uint8_t i)
{
	assert(block->count > 0);
	memmove(block->entry + i, block->entry + i + 1,
		(block->count - i - 1) * sizeof(olio_skiplist_entry));
	block->count--;
}

static olio_skiplist_block * 
	_split_block(olio_skiplist * list, olio_skiplist_block * block)
{
	assert(block->count == M - 1);
	olio_skiplist_block * new_block = _new_block(list);
	new_block->leaf = block->leaf;
	new_block->next = block->next;
	new_block->count = M2 - 1;
	block->next = new_block;
	block->count = M2;
	memmove(new_block->entry, block->entry + M2, 
		(M2 - 1) * sizeof(olio_skiplist_entry));
	return new_block;
}

static void
_merge_blocks(olio_skiplist * list, olio_skiplist_block * first, 
	olio_skiplist_block * second)
{
	assert(first->count + second->count < M);
	first->next = second->next;
	memmove(first->entry + first->count, second->entry, 
		second->count * sizeof(olio_skiplist_entry));
	first->count += second->count;
	_free_block(list, second);
}

static void
_balance_blocks(olio_skiplist_block * first, olio_skiplist_block * second)
{
	assert(first->count >= M2-1);
	assert(second->count >= M2-1);
	int diff = first->count - second->count;
	if (diff < 0) {
		memmove(first->entry + first->count, second->entry,
			-diff * sizeof(olio_skiplist_entry));
		memmove(second->entry, second->entry + -diff,
			(second->count - -diff) * sizeof(olio_skiplist_entry));
	} else if (diff > 0) {
		memmove(second->entry + diff, second->entry,
			second->count * sizeof(olio_skiplist_entry));
		memmove(second->entry, first->entry + first->count - diff,
			diff * sizeof(olio_skiplist_entry));
	}
	first->count -= diff;
	assert(first->count >= M2 - 1);
	second->count += diff;
	assert(second->count >= M2 - 1);
}

int olio_skiplist_init(olio_skiplist * list)
{
	list->head = _new_block(list);
	if (list->head == NULL) return -1;
	list->count = 0;
	list->height = 0;

	list->head->count = 1;
	list->head->leaf = 1;
	list->head->next = NULL;
	list->head->entry[0].key = OLIO_SKIPLIST_MAX_KEY;
	list->head->entry[0].data = NULL;

	return 0;
}

static void _recurse_free(olio_skiplist * list, olio_skiplist_block * block)
{
	if (!block->leaf) {
		int i;
		for (i = 0; i < block->count; i++)
			_recurse_free(list, block->entry[i].block);
	}
	_free_block(list, block);
}

void olio_skiplist_free(olio_skiplist * list)
{
	_recurse_free(list, list->head);
	list->head = NULL;
	list->count = 0;
	list->height = 0;
}

void * olio_skiplist_find(const olio_skiplist * list, uint32_t key)
{
	olio_skiplist_block * block = list->head;

	while (block != NULL) {
		int i;
		for (i = 0; i < block->count && key > block->entry[i].key; i++) ;
		assert(i != block->count);
		if (block->leaf) {
			if (key == block->entry[i].key)
				return block->entry[i].data;
			return NULL;
		}
		block = block->entry[i].block;
	}

	return NULL;
}

void olio_skiplist_range_begin(const olio_skiplist * list, uint32_t key, 
	olio_skiplist_cursor * cursor)
{
	olio_skiplist_block * block = list->head;

	while (block != NULL) {
		int i;
		for (i = 0; i < block->count && key > block->entry[i].key; i++) ;
		assert(i != block->count);
		if (block->leaf) {
			cursor->block = block;
			cursor->index = i;
			return;
		}
		block = block->entry[i].block;
	}

	cursor->block = NULL;
}

void * olio_skiplist_range_next(const olio_skiplist * list, uint32_t mkey, uint32_t * ckey, 
	olio_skiplist_cursor * cursor)
{
	void * data = NULL;
	uint32_t key = OLIO_SKIPLIST_MAX_KEY;

	if (cursor->block != NULL) {
		key = cursor->block->entry[cursor->index].key;
		if (mkey >= key) {
			data = cursor->block->entry[cursor->index].data;

			cursor->index++;
			if (cursor->index >= cursor->block->count) {
				cursor->index = 0;
				cursor->block = cursor->block->next;
			}			
		} else {
			cursor->block = NULL;
			key = OLIO_SKIPLIST_MAX_KEY;
		}
	}

	if (ckey != NULL) (*ckey) = key;
	return data;
}

int olio_skiplist_add(olio_skiplist * list, uint32_t key, void * data)
{
	olio_skiplist_block * block = list->head;
	olio_skiplist_block * new_block;
	olio_skiplist_block * child;

	if (block->count == M - 1) {
		new_block = _split_block(list, block);
		list->head = _new_block(list);
		list->height++;

		list->head->count = 2;
		list->head->leaf = 0;
		list->head->next = NULL;

		list->head->entry[0].block = block;
		list->head->entry[0].key = block->entry[M2 - 1].key;

		list->head->entry[1].block = new_block;	
		list->head->entry[1].key = new_block->entry[M2 - 2].key;

		block = list->head;
	}

	while (block != NULL) {
		int i;		
		for (i = 0; i < block->count && key > block->entry[i].key; i++) ;
		assert(i != block->count);
		if (block->leaf) {
			if (key == block->entry[i].key)
				return -1;

			_shift_right(block, i);
			list->count++;
			block->entry[i].key = key;
			block->entry[i].data = data;
			return 0;
		}

		child = block->entry[i].block;

		if (child->count == M - 1) {
			new_block = _split_block(list, child);
			_shift_right(block, i);
			block->entry[i].key = child->entry[M2 - 1].key;
			block->entry[i].block = child;
			block->entry[i + 1].block = new_block;
			if (key > block->entry[i].key)
				child = new_block;
		}

		block = child;		
	}

	return -1;
}

int olio_skiplist_remove(olio_skiplist * list, uint32_t key, void ** data)
{
	olio_skiplist_block * block = list->head;
	olio_skiplist_block * new_block;
	olio_skiplist_block * child;

	while (block != NULL) {
		int i;
		for (i = 0; i < block->count && key > block->entry[i].key; i++) ;
		assert(i != block->count);
		if (block->leaf) {
			if (key != block->entry[i].key)
				return -1;

			if (data != NULL)
				(*data) = block->entry[i].data;

			_shift_left(block, i);
			list->count--;
			return 0;
		}

		child = block->entry[i].block;

		if (child->count == M2 - 1) {

			uint8_t li = (i == 0 ? 0 : i - 1);
			olio_skiplist_block * lchild = block->entry[li].block;
			olio_skiplist_block * rchild = block->entry[li + 1].block;

			if (lchild->count + rchild->count < M) {
				_merge_blocks(list, lchild, rchild);
				if (block == list->head && block->count == 2) {
					list->head = lchild;
					list->height--;
					_free_block(list, block);
				} else {
					_shift_left(block, li);
					block->entry[li].block = lchild;
				}
				child = lchild;
			} else {
				_balance_blocks(lchild, rchild);
				block->entry[li].key = lchild->entry[lchild->count - 1].key;
			}
		}

		block = child;
	}

	return -1;
}

#ifdef OLIO_DEBUG

#include <stdio.h>

static void _recurse_blocks_display(const olio_skiplist_block * block, const int current_height)
{
	int i;
	fprintf(stderr, " @ height: %u\n  block: %p\n  leaf: %u\n  count: %u\n", 
		current_height, block, block->leaf, block->count);
		
	for (i = 0; i < M-1; i++) {
		printf("  %u%c  key: 0x%x  data: %p\n", 
			i, (i < block->count) ? '>' : '|',
			block->entry[i].key, block->entry[i].data);
	}
	
	if (!block->leaf)
		for (i = 0; i < block->count; i++)
			_recurse_blocks_display(block->entry[i].block, current_height-1);
}

void olio_skiplist_display(const olio_skiplist * list)
{
	fprintf(stderr, "skiplist:\ncount: %lu\nheight: %u\n", 
		list->count, list->height);

	if (list->head == NULL)
		fprintf(stderr, "not initialized\n");
	else
		_recurse_blocks_display(list->head, list->height);
}

#endif /* OLIO_DEBUG */
