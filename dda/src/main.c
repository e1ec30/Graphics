#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>


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

char *image_name;
size_t width, height;
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

void process_line(char *line) {
    char *s = NULL;
    char *tok = strok_r(line, " \t", &s);

    if (strcmp(tok, "size") == 0) {

        int n = atoi(strtok_r(NULL, " \t", &s));

        float x, y, z = 0.0, w = 1.0;
        char *d = NULL;

        while (1) { 
           d = strtok_r(NULL, " \t", &s);
           if (d == NULL) break;
           
           x = atof(d);
           y = atof(strtok_r(NULL, " \t", &s));
           
           if (s == 3) z = atof(strtok_r(NULL, " \t", &s));
           if (s == 4) w = atof(strtok_r(NULL, " \t", &s));

        }

        return;
    }
    if (strcmp(tok, "png") == 0) {
        //There are 3 tokens left
        //The width
        char *w = strtok_r(NULL, " \t", &s);
        width = atoi(w);

        //The height
        char *h = strtok_r(NULL, " \t", &s);
        height = atoi(h);

        //The filename
        image_name = strtok_r(NULL, " \t", &s);
        // printf("Width: %zu\nHeight: %zu\nName: %s\n", width, height, image_name);
        return;
    }
}

void process_commands(char *filename) {
    int fd;
    off_t len;
    void* data;
    char *saveptr = NULL;
    int i = 0;
    char *line;

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

int main(int argc, char** argv) {
    int fd;
    off_t len;
    void* data;
    char *saveptr = NULL;
    int i = 0;
    char *line;
    char *filename;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s [-d] <inputfile.txt>", argv[0]);
        exit(1);
    }


    if (argc >= 3 & strcmp(argv[1], "-d") == 0) {
        filename = argv[2];
        dump_image(filename);
    }
    else {
        filename = argv[1];
        process_commands(filename);
    }

    close(fd);
    free(data);
    exit(0);

}
