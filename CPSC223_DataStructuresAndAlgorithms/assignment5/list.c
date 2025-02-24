// adapted from Aspenes' Notes

#include "list.h"
#include "location.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct _list
{
  location *elements;
  int size;
  int capacity;
};

#define LIST_INITIAL_CAPACITY (2)

static void list_embiggen(list *l);

list *list_create()
{
  list *result = malloc(sizeof(*result));

  result->elements = malloc(sizeof(*result->elements) * LIST_INITIAL_CAPACITY);
  result->size = 0;
  result->capacity = result->elements != NULL ? LIST_INITIAL_CAPACITY : 0;

  return result;
}

int list_size(const list *l)
{
  return l->size;
}

void list_embiggen(list *l)
{
  // "ensure_capacity" may be a better name for this since
  // it tests whether the resize is necessary, but saying "embiggen"
  // reminds us that "a noble spirit embiggens the smallest [hu]man"
  if (l->capacity > 0 && l->size == l->capacity)
    {
      int *bigger = realloc(l->elements, sizeof(*l->elements) * l->capacity * 2);
      if (bigger != NULL)
	{
	  l->elements = bigger;
	  l->capacity *= 2;
	}
    }
}

void list_add(list *l, location item)
{
  list_embiggen(l);
      
  l->elements[l->size] = item;
  l->size++;
}

location* list_copy_data_array(const list* l)
{
    list* copy = malloc(sizeof(*l) * l->size);
    memcpy(copy, l->elements, sizeof(*l) * l->size);
    return copy;
}

void list_destroy(list *l)
{
  // free the array of pointers
  free(l->elements);

  // free the list struct
  free(l);
}