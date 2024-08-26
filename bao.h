/*
 * @description A standard library for C.
 */

#ifndef BAOLIB_H_
#define BAOLIB_H_

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#ifndef BAOLIBDEF
#ifdef BAOLIBSTATIC
#define BAOLIBDEF static
#else /* !defined(BAOLIBSTATIC) */
#define BAOLIBDEF extern
#endif /* BAOLIBSTATIC */
#endif /* BAOLIBDEF */

#if !defined(BAO_MALLOC) || !defined(BAO_REALLOC)			\
	|| !defined(BAO_CALLOC) || !defined(BAO_FREE) || !defined(BAO_STRDUP)
#define BAO_MALLOC(sz) malloc(sz)
#define BAO_REALLOC(x, newsz) realloc(x, newsz)
#define BAO_CALLOC(nmemb, size) calloc(nmembb, size)
#define BAO_STRDUP(s) strdup(s)
#define BAO_FREE(x) ((void) (free(x), x = NULL))
#endif

struct bao_array_t {
	size_t size;
	size_t memb_size;
	size_t capacity;
	void *data;
};

typedef struct bao_array_t *bao_array_t;

struct bao_list_t {
	struct bao_list_t *rest;
	void *data;
};

typedef struct bao_list_t *bao_list_t;

BAOLIBDEF bao_array_t bao_array_create(size_t size, size_t memb_size);
BAOLIBDEF int         bao_array_insert(bao_array_t array, void *v);
BAOLIBDEF void *      bao_array_get(bao_array_t array, size_t i);
BAOLIBDEF void *      bao_array_find(bao_array_t array, void *v,
				     int (*compare)(void *, void *));
BAOLIBDEF void        bao_array_free(bao_array_t *array);

BAOLIBDEF bao_list_t  bao_list_create(void *v);
BAOLIBDEF bao_list_t  bao_list_push(bao_list_t list, void *v);
BAOLIBDEF bao_list_t  bao_list_pop(bao_list_t list, void **v);
BAOLIBDEF void *      bao_list_get(bao_list_t list, size_t i);
BAOLIBDEF bao_list_t  bao_list_append(bao_list_t a_list, bao_list_t b_list);
BAOLIBDEF void        bao_list_free(bao_list_t *list);

#ifdef BAO_IMPLEMENTATION

static size_t bao_npo2(size_t n)
{
	if (n == 0) return 1;
	n--;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	n++;
	return n;
}

BAOLIBDEF bao_array_t bao_array_create(size_t size, size_t memb_size)
{
	bao_array_t array;
	assert(memb_size > 0);

	array = BAO_MALLOC(sizeof(*array));
	if (!array) {
		return NULL;
	}

	array->size = size;
	array->memb_size = memb_size;
	array->capacity = bao_npo2(array->size);

	array->data = BAO_MALLOC(array->memb_size * array->capacity);
	if (!array->data) {
		BAO_FREE(array);
		return NULL;
	}

	return array;
}

static int bao_array_resize(bao_array_t array)
{
	void *new_data;
	size_t new_capacity;

	new_capacity = array->capacity << 1;
	new_data = BAO_REALLOC(array->data, new_capacity * array->memb_size);
	if (!new_data) {
		return -ENOMEM;
	}

	array->capacity = new_capacity;
	array->data = new_data;
	return 0;
}

BAOLIBDEF int bao_array_insert(bao_array_t array, void *v)
{
	int ret;
	assert(array);
	assert(v);

	if (array->size == array->capacity) {
		if ((ret = bao_array_resize(array)) != 0) {
			return ret;
		}
	}

	memcpy(((char *) array->data) + array->memb_size * array->size,
	       v, array->memb_size);
	array->size++;
	return 0;
}

BAOLIBDEF void *bao_array_get(bao_array_t array, size_t i)
{
	assert(array);
	assert(i >= 0 && i < array->size);
	return ((char *) array->data) + i * array->memb_size;
}

BAOLIBDEF void *bao_array_find(bao_array_t array, void *v,
			       int (*compare)(void *, void *))
{
	size_t i;
	void *elem;
	assert(array);
	assert(compare);
	assert(v);

	for (i = 0; i < array->size; i++) {
		elem = ((char *) array->data) + i * array->memb_size;
		if (compare(elem, v) == 0) return elem;
	}
	
	return NULL;
}

BAOLIBDEF void bao_array_free(bao_array_t *array)
{
	assert(array && *array);
	BAO_FREE((*array)->data);
	BAO_FREE(*array);
}

BAOLIBDEF bao_list_t bao_list_create(void *v)
{
	bao_list_t list;

	assert(v);
	
	list = BAO_MALLOC(sizeof(*list));
	if (!list) {
		return NULL;
	}

	list->data = v;
	list->rest = NULL;
	return list;
}

BAOLIBDEF bao_list_t bao_list_push(bao_list_t list, void *v)
{
	bao_list_t rest;
	assert(list);
	assert(v);

	rest = bao_list_create(v);
	if (!rest) {
		return list;
	}

	rest->data = v;
	rest->rest = list;
	return rest;
}

BAOLIBDEF bao_list_t bao_list_pop(bao_list_t list, void **v)
{
	bao_list_t head;
	if (!list) return list;

	head = list->rest;
	if (v) *v = list->data;
	BAO_FREE(list);
	return head;
}

BAOLIBDEF void *bao_list_get(bao_list_t list, size_t i)
{
	size_t j;
	for (j = 0; j < i && list; j++) {
		list = list->rest;
	}

	return list ? list->data : NULL;
}

BAOLIBDEF bao_list_t bao_list_append(bao_list_t a_list, bao_list_t b_list)
{
	bao_list_t *head = &a_list;

	while (*head)
		head = &(*head)->rest;
	*head = b_list;
	return a_list;
}

BAOLIBDEF void bao_list_free(bao_list_t *list)
{
	bao_list_t next;

	assert(list);
	for (; *list; *list = next) {
		next = (*list)->rest;
		BAO_FREE((*list)->data);
		BAO_FREE(*list);
	}
}

#endif /* BAO_IMPLEMENTATION */
#endif /* BAOLIB_H_ */
