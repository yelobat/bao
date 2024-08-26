#ifndef BAOLIB_H_
#define BAOLIB_H_

#ifndef BAOLIBDEF
#ifdef BAOLIBSTATIC
#define BAOLIBDEF static
#else /* !defined(BAOLIBSTATIC) */
#define BAOLIBDEF extern
#endif /* BAOLIBSTATIC */
#endif /* BAOLIBDEF */

#if !defined(BAO_MALLOC) || !defined(BAO_REALLOC) \
	|| !defined(BAO_CALLOC) || !defined(BAO_FREE) || !defined(BAO_STRDUP)
#include <stdlib.h>
#define BAO_MALLOC(sz) malloc(sz)
#define BAO_REALLOC(x, newsz) realloc(x, newsz)
#define BAO_CALLOC(nmemb, size) calloc(nmembb, size)
#define BAO_STRDUP(s) strdup(s)
#define BAO_FREE(x) ((void) (free(x), x = NULL))
#endif

#ifdef BAO_IMPLEMENTATION

#endif /* BAO_IMPLEMENTATION */
#endif /* BAOLIB_H_ */
