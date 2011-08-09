#include <stdlib.h>

#include "list.h"

void olio_list_init(olio_list * l)
{
  /* initialize the head and tail */
  l->head = l->tail = NULL;
  /* count of items */
  l->count = 0;
}

void olio_list_free(olio_list * l)
{
  olio_list_entry * a;
  olio_list_entry * b;

  a = l->head;
  while (a != NULL) {
    b = a->next;
    free(a);
    a = b;
  }
  
  /* reset to initialized state */
  l->head = l->tail = NULL;
  l->count = 0;
}

int olio_list_append(olio_list * l, void * data)
{
  olio_list_entry * a = (olio_list_entry *) malloc(sizeof(olio_list_entry));
  if (a == NULL) return -1;
  a->data = data;
  a->next = NULL;
  a->prev = l->tail;
  if (l->tail != NULL) l->tail->next = a;
  else l->head = a;
  l->tail = a;
  l->count++;
  return 0;
}

void olio_list_head(olio_list * l, void ** opaque)
{
  *opaque = l->head;
}

void * olio_list_next(olio_list * l, void ** opaque)
{
  olio_list_entry * h = (olio_list_entry *) (*opaque);
  if (h == NULL) return NULL;
  *opaque = h->next;
  return h->data;
}

void olio_list_tail(olio_list * l, void ** opaque)
{
  *opaque = l->tail;
}

void * olio_list_prev(olio_list * l, void ** opaque)
{
  olio_list_entry * h = (olio_list_entry *) (*opaque);
  if (h == NULL) return NULL;
  *opaque = h->prev;
  return h->data;
}
