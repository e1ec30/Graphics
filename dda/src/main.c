#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <stdio.h>

typedef struct {
    size_t count;
    size_t capacity;
    void* xs;
} array;

#define new_darr(T, n) ((array) { \
    .count = 0, \
    .capacity = (n), \
    .xs = malloc(sizeof(T) * (n)) \
})

#define app_darr(T, x, e) do { \
    if ((x).count >= (x).capacity) { \
        (x).capacity = (x).capacity ? (x).capacity * 2 : 1; \
        (x).xs = realloc((x).xs, sizeof(T) * (x).capacity); \
    } \
    ((T*)(x).xs)[(x).count++] = (e); \
} while(0)

#define free_darr(x) do { \
    free((x).xs); \
    (x).xs = NULL; \
    (x).count = 0; \
    (x).capacity = 0; \
} while(0)

array position;
array color;
array textcoord;
array pointsize;

void init_arrays() {
    position = new_darr(gsl_vector*, 0);
    color = new_darr(gsl_vector*, 0);
    textcoord = new_darr(gsl_vector*, 0);
    pointsize = new_darr(double, 0);
}

// Next, copy paste code from anylang warmup for line-by-line processing
// And then start with DDA, then scanline

int main() {
    // Create a 3D vector

}