/*
 * @description A standard library for C.
 */

#ifndef BAOLIB_H_
#define BAOLIB_H_

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#ifndef BAOLIBDEF
#ifdef BAOLIBSTATIC
#define BAOLIBDEF static
#else /* !defined(BAOLIBSTATIC) */
#define BAOLIBDEF extern
#endif /* BAOLIBSTATIC */
#endif /* BAOLIBDEF */

#if !defined(BAO_LOG_MESSAGE) && !defined(BAO_POP_MESSAGE)
#include <stdarg.h>
#include <stdio.h>
#define BAO_LOG
#define BAO_LOG_STACK_CAPACITY (20)
#define BAO_LOG_MESSAGE_CAPACITY (256)
#define BAO_LOG_MESSAGE(fmt, ...)					\
	bao_log_message("Error in file '%s' at %s on line %d: " fmt,	\
			__FILE__, __func__, __LINE__ __VA_OPT__(,) __VA_ARGS__)

#define BAO_POP_MESSAGE()			\
	bao_log_pop_message()
#endif 

#if !defined(BAO_MALLOC) || !defined(BAO_REALLOC)			\
	|| !defined(BAO_CALLOC) || !defined(BAO_FREE) || !defined(BAO_STRDUP)
#define BAO_MALLOC(sz) malloc(sz)
#define BAO_REALLOC(x, newsz) realloc(x, newsz)
#define BAO_CALLOC(nmemb, size) calloc(nmemb, size)
#define BAO_STRDUP(s) strdup(s)
#define BAO_FREE(x) ((void) (free(x), x = NULL))
#endif

#define BAO_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define BAO_MIN(a, b) (((a) > (b)) ? (b) : (a))

struct bao_arena_chunk_t {
	struct bao_arena_chunk_t *prev;
	char *avail;
	char *limit;
};

typedef struct bao_arena_chunk_t *bao_arena_chunk_t;

struct bao_arena_t {
	bao_arena_chunk_t bao_arena_freechunks;
	size_t bao_arena_nfree;
	bao_arena_chunk_t first;
};

typedef struct bao_arena_t *bao_arena_t;

union bao_align_t {
	int i;
	long l;
	long *lp;
	void *p;
	void (*fp)(void);
	float f;
	double d;
	long double ld;
};

union bao_header_t {
	struct bao_arena_t b;
	union bao_align_t a;
};

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

struct bao_map_t {
	size_t size;
	size_t length;
	int (*compare)(const void *, const void *);
	size_t (*hash)(const void *);
	struct bao_mapping_t {
		struct bao_mapping_t *next;
		void *key;
		void *value;
	} **buckets;
};

typedef struct bao_map_t *bao_map_t;

struct bao_set_t {
	size_t size;
	size_t length;
	int (*compare)(const void *, const void *);
	size_t (*hash)(const void *);
	struct bao_member_t {
		struct bao_member_t *next;
		void *member;
	} **buckets;
};

typedef struct bao_set_t *bao_set_t;

BAOLIBDEF void bao_log_message(const char *fmt, ...);
BAOLIBDEF const char *bao_log_pop_message(void);

BAOLIBDEF bao_arena_t bao_arena_create(void);
BAOLIBDEF void *      bao_arena_alloc(bao_arena_t arena, size_t size);
BAOLIBDEF void *      bao_arean_calloc(bao_arena_t arena, size_t nmemb, size_t size);
BAOLIBDEF void        bao_arena_free(bao_arena_t arena);
BAOLIBDEF void        bao_arena_release(bao_arena_t *arena);

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

BAOLIBDEF bao_map_t bao_map_create(size_t hint,
				   int (*compare)(const void *, const void *),
				   size_t hash(const void *));
BAOLIBDEF int       bao_map_insert(bao_map_t map, void *key, void *v,
				   void **prev);
BAOLIBDEF void *    bao_map_find(bao_map_t map, void *key);
BAOLIBDEF size_t    bao_map_length(bao_map_t map);
BAOLIBDEF void      bao_map_free(bao_map_t *map);

BAOLIBDEF bao_set_t bao_set_create(size_t hint,
				   int (*compare)(const void *, const void *),
				   size_t (*hash)(const void *));
BAOLIBDEF int       bao_set_insert(bao_set_t set, void *member, void **prev);
BAOLIBDEF void *    bao_set_inside(bao_set_t set, void *member);
BAOLIBDEF void      bao_set_apply(bao_set_t set, void (*apply)(const void *, void *),
				  void *arg);
BAOLIBDEF bao_set_t bao_set_copy(bao_set_t set, size_t size);
BAOLIBDEF bao_set_t bao_set_union(bao_set_t set_a, bao_set_t set_b);
BAOLIBDEF void      bao_set_free(bao_set_t *set);

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

#ifdef BAO_LOG
static char bao_log_stack[BAO_LOG_STACK_CAPACITY][BAO_LOG_MESSAGE_CAPACITY];
static int bao_log_stack_ptr;
static int bao_log_stack_size;

static int bao_log_empty(void)
{
	return bao_log_stack_size == 0;
}

static int bao_log_full(void)
{
	return bao_log_stack_size == BAO_LOG_STACK_CAPACITY;
}
#endif /* BAO_LOG */

BAOLIBDEF void bao_log_message(const char *fmt, ...)
{
#ifdef BAO_LOG
	va_list args;
	if (!bao_log_full()) {
		bao_log_stack_size++;
	}

	va_start(args, fmt);
	vsnprintf((char *) (bao_log_stack + bao_log_stack_ptr),
		  BAO_LOG_MESSAGE_CAPACITY - 1, fmt, args);
	va_end(args);
	bao_log_stack_ptr = (bao_log_stack_ptr + 1) % BAO_LOG_STACK_CAPACITY;
#else /* !defined(BAO_LOG) */
	(void) 0;
#endif /* BAO_LOG */
}

BAOLIBDEF const char *bao_log_pop_message(void)
{
#ifdef BAO_LOG
	if (!bao_log_empty()) {
		bao_log_stack_size--;
		bao_log_stack_ptr--;
		if (bao_log_stack_ptr < 0) {
			bao_log_stack_ptr += BAO_LOG_STACK_CAPACITY;
		}
		return (const char *) bao_log_stack[bao_log_stack_ptr];
	}
	return NULL;
#else /* !defined(BAO_LOG) */
	return "Debugging disabled!";
#endif /* BAO_LOG */
}

BAOLIBDEF bao_arena_t bao_arena_create(void)
{
	bao_arena_t arena = BAO_MALLOC(sizeof(*arena));
	if (!arena) {
		BAO_LOG_MESSAGE("Ran out of memory!");
		return NULL;
	}

	arena->bao_arena_freechunks = NULL;
	arena->bao_arena_nfree = 0;
	arena->first = BAO_MALLOC(sizeof(*arena->first));
	if (!arena->first) {
		BAO_LOG_MESSAGE("Ran out of memory!");
		BAO_FREE(arena);
		return NULL;
	}
	
	arena->first->prev = NULL;
	arena->first->limit = arena->first->avail = NULL;
	return arena;
}

BAOLIBDEF void *bao_arena_alloc(bao_arena_t arena, size_t size)
{
	assert(arena);
	assert(size > 0);
	size = ((size + sizeof(union bao_align_t) - 1) /
		(sizeof(union bao_align_t))) * (sizeof(union bao_align_t));
	bao_arena_chunk_t first = arena->first;
	while (size > first->limit - first->avail) {
		char *limit;
		bao_arena_chunk_t new_arena_chunk;
		if ((new_arena_chunk = arena->bao_arena_freechunks) != NULL) {
			arena->bao_arena_freechunks = arena->bao_arena_freechunks->prev;
			arena->bao_arena_nfree--;
			limit = new_arena_chunk->limit;
		} else {
			size_t m = sizeof(union bao_header_t) + size + 10*1024;
			new_arena_chunk = BAO_MALLOC(m);
			if (new_arena_chunk == NULL) {
				BAO_LOG_MESSAGE("Ran out of memory!");
				return NULL;
			}
			limit = (char *) new_arena_chunk + m;
		}
		*new_arena_chunk = *first;
		first->avail = (char *)((union bao_header_t *) new_arena_chunk + 1);
		first->limit = limit;
		first->prev  = new_arena_chunk;
	}
	first->avail += size;
	return first->avail - size;
}

BAOLIBDEF void *bao_arean_calloc(bao_arena_t arena, size_t nmemb, size_t size)
{
	void *ptr;

	assert(size > 0);
	assert(nmemb > 0);

	ptr = bao_arena_alloc(arena, nmemb * size);
	if (!ptr) {
		return NULL;
	}

	memset(ptr, '\0', nmemb * size);
	return ptr;
}

#define BAO_ARENA_THRESHOLD (10)

BAOLIBDEF void bao_arena_free(bao_arena_t arena)
{
	assert(arena);
	bao_arena_chunk_t first = arena->first;
	while (first->prev) {
		struct bao_arena_chunk_t tmp = *first->prev;
		if (arena->bao_arena_nfree < BAO_ARENA_THRESHOLD) {
			first->prev->prev = arena->bao_arena_freechunks;
			arena->bao_arena_freechunks = first->prev;
			arena->bao_arena_nfree++;
			arena->bao_arena_freechunks->limit = first->limit;
		} else {
			BAO_FREE(first->prev);
		}
		*first = tmp;
	}

	assert(first->limit == NULL);
	assert(first->avail == NULL);
}

BAOLIBDEF void bao_arena_release(bao_arena_t *arena)
{
	assert(arena && *arena);
	bao_arena_chunk_t first = (*arena)->first;
	while (first->prev) {
		bao_arena_chunk_t tmp = first->prev;
		BAO_FREE(first);
		first = tmp;
	}
	BAO_FREE(first);
	BAO_FREE(*arena);
}

BAOLIBDEF bao_array_t bao_array_create(size_t size, size_t memb_size)
{
	bao_array_t array;
	assert(memb_size > 0);

	array = BAO_MALLOC(sizeof(*array));
	if (!array) {
		BAO_LOG_MESSAGE("Ran out of memory!");
		return NULL;
	}

	array->size = 0;
	array->memb_size = memb_size;
	array->capacity = bao_npo2(size);

	array->data = BAO_MALLOC(array->memb_size * array->capacity);
	if (!array->data) {
		BAO_LOG_MESSAGE("Ran out of memory!");
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
		BAO_LOG_MESSAGE("Ran out of memory!");
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
		BAO_LOG_MESSAGE("Ran out of memory!");
		return NULL;
	}

	list->data = v;
	list->rest = NULL;
	return list;
}

BAOLIBDEF bao_list_t bao_list_push(bao_list_t list, void *v)
{
	bao_list_t rest;
	assert(v);

	rest = bao_list_create(v);
	if (!rest) {
		BAO_LOG_MESSAGE("Ran out of memory!");
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

	while (*head) {
		head = &(*head)->rest;
	}
	*head = b_list;
	return a_list;
}

BAOLIBDEF void bao_list_free(bao_list_t *list)
{
	bao_list_t next;

	assert(list);
	for (; *list; *list = next) {
		next = (*list)->rest;
		BAO_FREE(*list);
	}
}

BAOLIBDEF bao_map_t bao_map_create(size_t hint,
				   int (*compare)(const void *, const void *),
				   size_t hash(const void *))
{
	bao_map_t map;
	size_t i;
	static int primes[] = {
		509, 509, 1021, 2053, 4093,
		8191, 16381, 32771, 65521, INT_MAX 
	};

	assert(compare);
	assert(hash);

	for (i = 1; primes[i] < hint; i++)
		;
	map = BAO_MALLOC(sizeof(*map) + primes[i-1] * sizeof(map->buckets[0]));
	if (!map) {
		BAO_LOG_MESSAGE("Ran out of memory!");
		return NULL;
	}
	
	map->size = primes[i-1];
	map->length = 0;
	map->compare = compare;
	map->hash = hash;
	map->buckets = (struct bao_mapping_t **) (map + 1);
	for (i = 0; i < map->size; i++)
		map->buckets[i] = NULL;
	return map;
}

#include <stdio.h>
BAOLIBDEF int bao_map_insert(bao_map_t map, void *key, void *v, void **prev)
{
	size_t i;
	struct bao_mapping_t *p;

	assert(map);
	assert(key);
	assert(v);
	
	i = map->hash(key) % map->size;
	for (p = map->buckets[i]; p; p = p->next)
		if (map->compare(key, p->key) == 0) 
			break;

	if (p == NULL) {
		p = BAO_MALLOC(sizeof(*p));
		if (!p) {
			BAO_LOG_MESSAGE("Ran out of memory!");
			return -ENOMEM;
		}
		p->key = key;
		p->next = map->buckets[i];
		map->buckets[i] = p;
		map->length++;
		if (prev) *prev = NULL;
	} else if (prev) {
		*prev = p->value;
	}

	p->value = v;
	return 0;
}

BAOLIBDEF void *bao_map_find(bao_map_t map, void *key)
{
	size_t i;
	struct bao_mapping_t *p;
	assert(map);
	assert(key);
	i = map->hash(key) % map->size;
	for (p = map->buckets[i]; p; p = p->next)
		if (map->compare(key, p->key) == 0)
			break;
	return p ? p->value : NULL;
}

BAOLIBDEF size_t bao_map_length(bao_map_t map)
{
	assert(map);
	return map->length;
}

BAOLIBDEF void bao_map_free(bao_map_t *map)
{
	size_t i;
	
	assert(map);
	assert(*map);
	
	struct bao_mapping_t *p, *q;
	for (i = 0; i < (*map)->size; i++) {
		for (p = (*map)->buckets[i]; p; p = q) {
			q = p->next;
			BAO_FREE(p);
		}
	}
	BAO_FREE(*map);
}

BAOLIBDEF bao_set_t bao_set_create(size_t hint,
				   int (*compare)(const void *, const void *),
				   size_t (*hash)(const void *))
{
	size_t i;
	bao_set_t set;
	static int primes[] = {
		509, 509, 1021, 2053, 4093,
		8191, 16381, 32771, 65521, INT_MAX 
	};

	for (i = 1; primes[i] < hint; i++)
		;
	set = BAO_MALLOC(sizeof(*set) + primes[i-1] * sizeof(set->buckets[0]));
	if (!set) {
		BAO_LOG_MESSAGE("Ran out of memory!");
		return NULL;
	}

	set->size = primes[i-1];
	set->compare = compare;
	set->hash = hash;
	set->buckets = (struct bao_member_t **) (set + 1);
	for (i = 0; i < set->size; i++)
		set->buckets[i] = NULL;
	set->length = 0;
	return set;
}

BAOLIBDEF int bao_set_insert(bao_set_t set, void *member, void **prev)
{
	size_t i;
	struct bao_member_t *p;

	assert(set);
	assert(member);

	i = set->hash(member) % set->size;
	for (p = set->buckets[i]; p; p = p->next)
		if (set->compare(member, p->member) == 0)
			break;
	if (p == NULL) {
		p = BAO_MALLOC(sizeof(*p));
		if (!p) {
			BAO_LOG_MESSAGE("Ran out of memory!");
			return -ENOMEM;
		}
		p->member = member;
		p->next = set->buckets[i];
		set->buckets[i] = p;
		set->length++;
		if (prev) *prev = NULL;
	} else {
		if (prev) *prev = p->member;
		p->member = member;
	}

	return 0;
}

BAOLIBDEF void *bao_set_inside(bao_set_t set, void *member)
{
	size_t i;
	struct bao_member_t *p;

	assert(set);
	assert(member);

	i = set->hash(member) % set->size;
	for (p = set->buckets[i]; p; p = p->next)
		if (set->compare(member, p->member) == 0)
			break;
	return p ? p->member : NULL;
}

BAOLIBDEF void bao_set_apply(bao_set_t set, void (*apply)(const void *, void *),
			     void *arg)
{
	size_t i;
	struct bao_member_t *p;
	assert(set);
	assert(apply);

	for (i = 0; i < set->size; i++) {
		for (p = set->buckets[i]; p; p = p->next) {
			apply(p->member, arg);
		}
	}
}

BAOLIBDEF bao_set_t bao_set_copy(bao_set_t set, size_t size)
{
	size_t i, j;
	void *member;
	bao_set_t new_set;
	struct bao_member_t *p, *q;

	assert(set);
	new_set = bao_set_create(size, set->compare, set->hash);
	if (!new_set) {
		BAO_LOG_MESSAGE("Ran out of memory!");
		return NULL;
	}

	for (i = 0; i < set->size; i++) {
		for (p = set->buckets[i]; p; p = p->next) {
			member = p->member;
			j = set->hash(member) % new_set->size;
			q = BAO_MALLOC(sizeof(*q));
			if (!q) {
				BAO_LOG_MESSAGE("Ran out of memory!");
				bao_set_free(&new_set);
				return NULL;
			}
			q->member = member;
			q->next = new_set->buckets[j];
			new_set->buckets[j] = q;
			new_set->length++;
		}
	}

	return new_set;
}

BAOLIBDEF bao_set_t bao_set_union(bao_set_t set_a, bao_set_t set_b)
{
	size_t i;
	bao_set_t new_set;
	struct bao_member_t *p;
	if (set_a == NULL) {
		assert(set_b);
		return bao_set_copy(set_b, set_b->size);
	} else if (set_b == NULL) {
		assert(set_a);
		return bao_set_copy(set_a, set_a->size);
	}
	
	assert(set_a->compare == set_b->compare && set_a->hash == set_b->hash);
	new_set = bao_set_copy(set_a, BAO_MAX(set_a->size, set_b->size));
	for (i = 0; i < set_b->size; i++) {
		for (p = set_b->buckets[i]; p; p = p->next) {
			if (bao_set_insert(new_set, p->member, NULL) != 0) {
				bao_set_free(&new_set);
				return NULL;
			}
		}
	}

	return new_set;
}

BAOLIBDEF void bao_set_free(bao_set_t *set)
{
	size_t i;
	struct bao_member_t *p, *q;

	assert(set);
	assert(*set);

	for (i = 0; i < (*set)->size; i++) {
		for (p = (*set)->buckets[i]; p; p = q) {
			q = p->next;
			BAO_FREE(p);
		}
	}

	BAO_FREE(*set);
}

#endif /* BAO_IMPLEMENTATION */
#endif /* BAOLIB_H_ */
