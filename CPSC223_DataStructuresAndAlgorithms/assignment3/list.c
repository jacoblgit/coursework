// Array implemenation of generic list
// adapted from Glenn's Notes

#include "list.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct _list
{
  void *(*copy)(const void *);
  void (*print)(FILE *, const void *);
  void (*destroy)(void *);
  void **elements;
  size_t size;
  size_t capacity;
};

#define LIST_INITIAL_CAPACITY (2)

/**
 * Performs mergesort on the given array, using the given workspace
 * to perform the split and merge.
 */
static void list_mergesort(const void **arr, size_t n, const void **work, int (*compare)(const void *, const void *, const void *), const void *arg);

/**
 * Merges the given two sorted lists into the destination in sorted order according
 * to the given comparison function when passed the given extra argument.
 *
 * @param dest a pointer to an array of size n1 + n2
 * @param l1 a pointer to a sorted array
 * @param n1 the size of the array l1 points to
 * @param l2 a pointer to a sorted array
 * @param n2 the size of the array l2 points to
 * @param compare a comparison function for integers
 * @param arg a pointer
 */
static void list_merge(const void **dest, const void **l1, size_t n1, const void **l2, size_t n2, int (*compare)(const void *, const void *, const void *), const void *arg);


/**
 * Enlarges the array inside the given list if it is full.
 *
 * @param l a pointer to a list
 */
static void list_embiggen(list *l);


list *list_create(void *(*copy)(const void *), void (*print)(FILE *, const void *), void (*destroy)(void *))
{
  list *result = malloc(sizeof(*result));

  result->elements = malloc(sizeof(*result->elements) * LIST_INITIAL_CAPACITY);
  result->size = 0;
  result->capacity = result->elements != NULL ? LIST_INITIAL_CAPACITY : 0;
  result->copy = copy;
  result->print = print;
  result->destroy = destroy;

  return result;
}


size_t list_size(const list *l)
{
  return l->size;
}


const void *list_get(const list *l, size_t i)
{
  return l->elements[i];
}


void list_embiggen(list *l)
{
  // "ensure_capacity" may be a better name for this since
  // it tests whether the resize is necessary, but saying "embiggen"
  // reminds us that "a noble spirit embiggens the smallest [hu]man"
  if (l->capacity > 0 && l->size == l->capacity)
    {
      void **bigger = realloc(l->elements, sizeof(*l->elements) * l->capacity * 2);
      if (bigger != NULL)
	{
	  l->elements = bigger;
	  l->capacity *= 2;
	}
    }
}


void list_add(list *l, const void *item)
{
  list_embiggen(l);
      
  l->elements[l->size] = l->copy(item);
  l->size++;
}


void list_add_at_index(list *l, const void *item, int insertion_index)
{
  list_embiggen(l);

  for (size_t copy_to = l->size; copy_to > insertion_index; copy_to--)
    {
      l->elements[copy_to] = l->elements[copy_to - 1];
    }

  // add new element at insertion_index
  l->elements[insertion_index] = l->copy(item);
  l->size++;
}


void list_for_each(const list *l, void (*f)(const void *elt, size_t i, void *arg), void *arg)
{
  for (size_t i = 0; i < l->size; i++)
    {
      f(l->elements[i], i, arg);
    }
}

void list_destroy_range(list* l, int i, int j) {
  int num_removed = j - i;
  
  // delete elements
  for (int index = i; index < j; index++) {
    l->destroy(l->elements[index]);
    // l->elements[index] = NULL;
  }

  // move all other elements forward, as long as there are elements to move forwards
  for (int index = i; index < j && (index + num_removed) < l->size; index++) {
    // l->elements[i] = l->elements[i + num_removed];
    l->elements[index] = l->elements[index + num_removed];
  }

  l->size -= num_removed;
}


void list_sort(list *l, int (*compare)(const void *, const void *, const void *), const void *arg)
{
  // allocate extra space to do the split/merge
  const void **work = malloc(sizeof(*l->elements) * l->size);

  // call recursive function to do the work
  list_mergesort((const void **)l->elements, l->size, work, compare, arg);

  free(work);
}

void list_mergesort(const void **arr, size_t n, const void **work, int (*compare)(const void *, const void *, const void *), const void *arg)
{
  if (n >= 2)
    {
      // copy the array into the work space
      memcpy(work, arr, sizeof(*arr) * n);

      // sort the first half using the original array as work space
      list_mergesort(work, n / 2, arr, compare, arg);

      // sort the second half using the original array as work space
      list_mergesort(work + (n / 2), n - (n / 2), arr + (n / 2), compare, arg);

      // merge the sorted lists back into the original array
      list_merge(arr, work, n / 2, work + (n / 2), n - (n / 2), compare, arg);
    }
}


void list_merge(const void **dest, const void **l1, size_t n1, const void **l2, size_t n2, int (*compare)(const void *, const void *, const void *), const void *arg)
{
  size_t i = 0; // the current location in the destination array

  while (n1 > 0 && n2 > 0)
    {
      // both lists non-empty; compare first elt of each
      if (compare(*l1, *l2, arg) <= 0)
	{
	  // elt at beginning of 1st list comes before elt at beginning of 2nd list
	  dest[i++] = *l1;
	  l1++;
	  n1--;
	}
      else
	{
	  // elt at beginning of 2st list comes before elt at beginning of 1st list
	  dest[i++] = *l2;
	  l2++;
	  n2--;
	}
    }

  // copy remaining items from each list
  while (n1 > 0)
    {
      dest[i++] = *l1;
      l1++;
      n1--;
    }

  while (n2 > 0)
    {
      dest[i++] = *l2;
      l2++;
      n2--;
    }
}


void list_destroy(list *l)
{
  // free the copies of the elements
  for (size_t i = 0; i < l->size; i++)
    {
      l->destroy(l->elements[i]);
    }
  
  // free the array of pointers
  free(l->elements);

  // free the list struct
  free(l);
}


struct list_print_info
{
  FILE *out;
  const list *l;
};


static void list_print_helper(const void *elt, size_t i, void *arg)
{
  struct list_print_info *info = arg;
  if (i > 0)
    {
      fputs(" ", info->out);
    }
  info->l->print(info->out, elt);
}


void list_print(const list *l, FILE *out)
{
  struct list_print_info info = {out, l};
  // fputc('[', out);
  list_for_each(l, list_print_helper, &info);
  // fputc(']', out);
}