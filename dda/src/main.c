#include "gsl/gsl_vector_double.h"
#include <fcntl.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <uselibpng.h>

typedef struct {
  size_t count;
  size_t capacity;
  void *xs;
} array;

typedef union {
    float a[4];
    struct {float x, y, z, w;};
    struct {float r, g, b, o;};
  } point;

#define new_darr(T, n)                                                         \
  ((array){.count = 0, .capacity = (n), .xs = malloc(sizeof(T) * (n))})

#define app_darr(T, x, e)                                                      \
  do {                                                                         \
    if ((x).count >= (x).capacity) {                                           \
      (x).capacity = (x).capacity ? (x).capacity * 2 : 1;                      \
      (x).xs = realloc((x).xs, sizeof(T) * (x).capacity);                      \
    }                                                                          \
    ((T *)(x).xs)[(x).count++] = (e);                                          \
  } while (0)

#define free_darr(x)                                                           \
  do {                                                                         \
    free((x).xs);                                                              \
    (x).xs = NULL;                                                             \
    (x).count = 0;                                                             \
    (x).capacity = 0;                                                          \
  } while (0)

char *image_name;
size_t width, height;
array position;
array color;
array textcoord;
array pointsize;

void init_arrays() {
  position = new_darr(gsl_vector *, 20);
  color = new_darr(gsl_vector *, 20);
  textcoord = new_darr(gsl_vector *, 20);
  pointsize = new_darr(double, 20);
}

void dump_vector(gsl_vector *v) {
  for (int i = 0; i < v->size; i++) {
    printf("%d: %g\t", i, gsl_vector_get(v, i));
  }
  puts("");
}

void ndc_to_pixel(gsl_vector *v, size_t w, size_t h) {
  float x = gsl_vector_get(v, 0);
  float y = gsl_vector_get(v, 1);

  float new_x = (x + 1.0f) * (w / 2);
  float new_y = (y + 1.0f) * (h / 2);

  gsl_vector_set(v, 0, new_x);
  gsl_vector_set(v, 1, new_y);
}

// Next, copy paste code from anylang warmup for line-by-line processing
// And then start with DDA, then scanline

// Swap two pointers
void swap(void **a, void **b) {

  void *temp = *a;
  *a = *b;
  *b = temp;
}

// Sort by the d component
void sort_vec3(gsl_vector **a, gsl_vector **b, gsl_vector **c, size_t d) {
  // Bubble Sort
  if (gsl_vector_get(*a,d) > gsl_vector_get(*b,d))
    swap((void **)a, (void **)b);
  if (gsl_vector_get(*b,d) > gsl_vector_get(*c,d))
    swap((void **)b, (void **)c);
  if (gsl_vector_get(*a,d) > gsl_vector_get(*b,d))
    swap((void **)a, (void **)b);
}

void dump_image(image_t *img) {
  for (int i = 0; i < img->width; i++) {
    for (int j = 0; j < img->height; j++) {
      int r = pixel_xy(img, i, j).red;
      printf("%d ", r);
    }
    printf("\n");
  }
}

void putpixel(gsl_vector *p, gsl_vector* color, image_t *img) {

  size_t x = (size_t)(gsl_vector_get(p, 0));
  size_t y = (size_t)(gsl_vector_get(p, 1));

  // printf("X: float: %.2f, size_t: %ld\n", x, (size_t)x);
  // printf("Y: float: %.2f, size_t: %ld\n", y, (size_t)y);
  // printf("x: %ld\n", x);
  // printf("y: %ld\n", y);
  
  float r = gsl_vector_get(color, 0);
  float g = gsl_vector_get(color, 1);
  float b = gsl_vector_get(color, 2);
  float a = gsl_vector_get(color, 3);

  // printf("r: %.3f g: %.3f b: %.3f a: %.3f\n", r * 255, g * 255, b * 255, a * 255);

  pixel_xy(img, x, y).red = r * 255;
  pixel_xy(img, x, y).green = g * 255;
  pixel_xy(img, x, y).blue = b * 255;
  pixel_xy(img, x, y).alpha = a * 255;
}

// A line between two vectors a and b where the dimension d should have integer
// coordinates
bool dda_draw_line(gsl_vector *a, gsl_vector *b, size_t d, image_t *img,
                   gsl_vector **p_out, gsl_vector **s_out, gsl_vector *color, bool draw) {
  float a_d = gsl_vector_get(a, d);
  float b_d = gsl_vector_get(b, d);

  if (a_d == b_d)
    return false;

  if (a_d > b_d)
    swap((void **)&a, (void **)&b);

  // The difference vector delta = b - a. First copy b, and then subtract a from
  // it.
  gsl_vector *delta = gsl_vector_alloc(b->size);
  gsl_vector_memcpy(delta, b);
  gsl_vector_sub(delta, a);

  // s = delta / d
  gsl_vector *s = delta;
  gsl_vector_scale(s, (1 / gsl_vector_get(delta, d)));
  if (p_out)
    gsl_vector_memcpy(*s_out, s);

  // e = ceil(a_d) - a_d
  float e = ceilf(a_d) - a_d;

  // o = es
  gsl_vector *o = gsl_vector_alloc(s->size);
  gsl_vector_memcpy(o, s);
  gsl_vector_scale(o, e);

  // p = o + a
  gsl_vector *p = o;
  gsl_vector_add(p, a);
  if (p_out)
    gsl_vector_memcpy(*p_out, p);

  // while p_d < b_d
  if (draw) {
    while (gsl_vector_get(p, d) < gsl_vector_get(b, d)) {
      // Do what you want with p
      putpixel(p, color, img);

      // Add s to p
      gsl_vector_add(p, s);
    }
  }
  return true;
}

// Void Scanline, where a, b, c are vertices
void dda_scan_line(gsl_vector *a, gsl_vector *b, gsl_vector *c,
                   gsl_vector *color, image_t *img) {
  // First, we sort, so that a is the smallest and c is the biggest
  sort_vec3(&a, &b, &c, 1);

  ndc_to_pixel(a, width, height);
  ndc_to_pixel(b, width, height);
  ndc_to_pixel(c, width, height);

  // printf("Scanline for:\n");
  // dump_vector(a);
  // dump_vector(b);
  // dump_vector(c);

  // DDA along a to c
  gsl_vector *p_long = gsl_vector_alloc(a->size);
  gsl_vector *s_long = gsl_vector_alloc(a->size);
  if ( !dda_draw_line(a, c, 1, img, &p_long, &s_long, color, false) ) return;

  // DDA along a to b
  gsl_vector *p = gsl_vector_alloc(a->size);
  gsl_vector *s = gsl_vector_alloc(a->size);
  if ( !dda_draw_line(a, b, 1, img, &p, &s, color, false) ) return ;

  while (gsl_vector_get(p, 1) < gsl_vector_get(b, 1)) {

    // Run DDA in x and actually draw the pixels
    // This is the top half of the triangle
    dda_draw_line(p, p_long, 0, img, NULL, NULL, color, true);
    gsl_vector_add(p, s);
    gsl_vector_add(p_long, s_long);
  }

  // DDA along b to c for the bottom half
  // p_long & s_long remain the same
  // But we can just reassign p and s

  if ( !dda_draw_line(b, c, 1, img, &p, &s, color, false) ) return;
  while (gsl_vector_get(p, 1) < gsl_vector_get(c, 1)) {

    // Run DDA in x and actually draw the pixels
    // This is the top half of the triangle
    dda_draw_line(p, p_long, 0, img, NULL, NULL, color, true);
    gsl_vector_add(p, s);
    gsl_vector_add(p_long, s_long);
  }
}

void process_line(char *line) {
  char *s = NULL;
  char *tok = strtok_r(line, " \t", &s);

  if (strcmp(tok, "position") == 0) {

    int n = atoi(strtok_r(NULL, " \t", &s));

    // float x, y, z = 0.0, w = 1.0;
    char *d = NULL;

    while (1) {
      bool b = false;
      point p = {.a = {0.0, 0.0, 0.0, 1.0}};
      for (int i = 0; i < n; i++) {
        d = strtok_r(NULL, " \t", &s);
        if (d == NULL) { 
          b = true;
          break;
        }
        p.a[i] = atof(d);
      }
      if (b) break;
      gsl_vector *pos_vec = gsl_vector_alloc(4);

      gsl_vector_set(pos_vec, 0, p.x);
      gsl_vector_set(pos_vec, 1, p.y);
      gsl_vector_set(pos_vec, 2, p.z);
      gsl_vector_set(pos_vec, 3, p.w);

      app_darr(gsl_vector *, position, pos_vec);
    }

    // while (1) {
    //   d = strtok_r(NULL, " \t", &s);
    //   if (d == NULL)
    //     break;
    //
    //   x = atof(d);
    //   y = atof(strtok_r(NULL, " \t", &s));
    //
    //   if (n == 3)
    //     z = atof(strtok_r(NULL, " \t", &s));
    //   else if (n == 4)
    //     w = atof(strtok_r(NULL, " \t", &s));
    //
    //   gsl_vector *pos_vec = gsl_vector_alloc(4);
    //
    //   gsl_vector_set(pos_vec, 0, x);
    //   gsl_vector_set(pos_vec, 1, y);
    //   gsl_vector_set(pos_vec, 2, z);
    //   gsl_vector_set(pos_vec, 3, w);
    //
    //   app_darr(gsl_vector *, position, pos_vec);
    // }

    return;
  }

  if (strcmp(tok, "color") == 0) {

    int n = atoi(strtok_r(NULL, " \t", &s));

    // float r, g, b, a = 1.0;
    char *d = NULL;

    // while (1) {
    //   d = strtok_r(NULL, " \t", &s);
    //   if (d == NULL)
    //     break;
    //
    //   r = atof(d);
    //   g = atof(strtok_r(NULL, " \t", &s));
    //
    //   b = atof(strtok_r(NULL, " \t", &s));
    //   if (n == 4)
    //     a = atof(strtok_r(NULL, " \t", &s));
    //
    //   gsl_vector *color_vec = gsl_vector_alloc(4);
    //
    //   gsl_vector_set(color_vec, 0, r);
    //   gsl_vector_set(color_vec, 1, g);
    //   gsl_vector_set(color_vec, 2, b);
    //   gsl_vector_set(color_vec, 3, a);
    //
    //   app_darr(gsl_vector *, color, color_vec);
    // }
    //
    while (1) {
      bool b = false;
      point p = {.a = {0.0, 0.0, 0.0, 1.0}};
      for (int i = 0; i < n; i++) {
        d = strtok_r(NULL, " \t", &s);
        if (d == NULL) { 
          b = true;
          break;
        }
        p.a[i] = atof(d);
      }
      if (b) break;
      gsl_vector *color_vec = gsl_vector_alloc(4);

      gsl_vector_set(color_vec, 0, p.r);
      gsl_vector_set(color_vec, 1, p.g);
      gsl_vector_set(color_vec, 2, p.b);
      gsl_vector_set(color_vec, 3, p.o);

      app_darr(gsl_vector *, color, color_vec);
    }


    return;
  }
  if (strcmp(tok, "png") == 0) {
    // There are 3 tokens left
    // The width
    char *w = strtok_r(NULL, " \t", &s);
    width = atoi(w);

    // The height
    char *h = strtok_r(NULL, " \t", &s);
    height = atoi(h);

    // The filename
    image_name = strtok_r(NULL, " \t", &s);
    // printf("Width: %zu\nHeight: %zu\nName: %s\n", width, height, image_name);
    return;
  }

  // if (strcmp(tok, "line") == 0) {
  // 	gsl_vector *a = ( (gsl_vector**)( position.xs ) )[0];
  // 	gsl_vector *b = ( (gsl_vector**)( position.xs ) )[1];
  //
  // 	image_t *img = new_image(width, height);
  //
  // 	dda_draw_line(a, b, 1, img);
  //
  // 	save_image(img, image_name);
  // 	// dump_image(img);
  // 	free_image(img);
  // 	return;
  // }

  if (strcmp(tok, "drawArraysTriangles") == 0) {

    image_t *img = new_image(width, height);
    int first = atoi(strtok_r(NULL, " \t", &s));
    int count = atoi(strtok_r(NULL, " \t", &s));

    gsl_vector **positions = position.xs;
    gsl_vector **colors = color.xs;

    int i = 0, j = 0;
    while (i < count) {
      gsl_vector *a = positions[first + i];
      gsl_vector *b = positions[first + i + 1];
      gsl_vector *c = positions[first + i + 2];
      gsl_vector *color = colors[first + j];

      dda_scan_line(a, b, c, color, img);

      i += 3;
      j += 3;
    }
    save_image(img, image_name);
    free_image(img);
    return;
  }
}

void process_commands(char *filename) {
  int fd;
  off_t len;
  void *data;
  char *saveptr = NULL;
  char *line;

  init_arrays();

  if ((fd = open(filename, O_RDONLY)) == -1) {
    fprintf(stderr, "%s: ", filename);
    perror("Couldn't open file");
    exit(-1);
  }

  len = lseek(fd, (off_t)0, SEEK_END);
  lseek(fd, (off_t)0, SEEK_SET);

  data = malloc((size_t)len);

  if (read(fd, data, len) != len) {
    perror("read");
    free(data);
    exit(1);
  }

  line = strtok_r(data, "\r\n", &saveptr);
  while (line != NULL) {
    // printf("%d: %s\n\n", ++i, line);
    process_line(line);
    // puts("\n");
    line = strtok_r(NULL, "\r\n", &saveptr);
  }

  close(fd);
  free(data);
}

int main(int argc, char **argv) {
  char *filename;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <inputfile.txt>", argv[0]);
    exit(1);
  }

  filename = argv[1];
  process_commands(filename);

  exit(0);
}
