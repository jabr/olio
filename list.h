#ifndef _OLIO_LIST_H_
#define _OLIO_LIST_H_

#include <inttypes.h>

typedef struct _olio_list_entry {
  struct _olio_list_entry * next;
  struct _olio_list_entry * prev;
  void * data;
} olio_list_entry;

typedef struct _olio_list {
  struct _olio_list_entry * head;
  struct _olio_list_entry * tail;
  uint32_t count;
} olio_list;

#ifdef __cplusplus
extern "C" {
#endif

/* basic init/free functions for the list */
void olio_list_init(olio_list *);
void olio_list_free(olio_list *);

/* append an entry to the list */
int olio_list_append(olio_list *, void * data);

void olio_list_head(olio_list *, void ** opaque);
void olio_list_tail(olio_list *, void ** opaque);
void * olio_list_next(olio_list *, void ** opaque);
void * olio_list_prev(olio_list *, void ** opaque);

#ifdef __cplusplus
} /* extern C */
#endif

static inline olio_list_length(const olio_list * l)
{
  return l->count;
}

#endif /* _OLIO_LIST_H_ */
