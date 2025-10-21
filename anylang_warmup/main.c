#include "libs/uselibpng.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

typedef struct{
    int x;
    int y;
} point;

char *image_name;
size_t width, height;

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


void process_line(char* line, array* pixels, array* coords) {
    char *s = NULL;
    char *tok = strtok_r(line, " \t", &s);
    
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

    if (strcmp(tok, "position") == 0) {
        //First, we should get the number 2
        char *n = strtok_r(NULL, " \t", &s);
        point p;

        //Then we get pairs of two coordinates
        while (1) {
            tok = strtok_r(NULL, " \t", &s);
            if (tok == NULL) break;
            p.x = atoi(tok);

            tok = strtok_r(NULL, " \t", &s);
            if (tok == NULL) break;
            p.y = atoi(tok);
            
            app_darr(point, *coords, p);
        }
        return;
    }

    if (strcmp(tok, "color") == 0) {
        //First, the number 4
        char *n = strtok_r(NULL, " \t", &s);
        pixel_t p;

        //Then pixel values in 4's
        while(1) {
            tok = strtok_r(NULL, " \t", &s);
            if (tok == NULL) break;
            p.p[0] = atoi(tok);

            for(int i = 1; i < 4; i++) {
                tok = (strtok_r(NULL, " \t", &s));
                p.p[i] = atoi(tok);

            }
            app_darr(pixel_t, *pixels, p);
            // printf("r: %d, g: %d, b: %d, a: %d\n", p.r, p.g, p.b, p.a);
        }
        return;
    }

    if (strcmp(tok, "drawPixels") == 0) {
        //First, the number of pixels to draw
        int n = atoi(strtok_r(NULL, " \t", &s));

        image_t *img = new_image(width, height);

        //fill in the last n pixels
        for (int i = n - 1; i >= 0; i--) {
            pixel_t* ps = pixels->xs;
            point* cs = coords->xs;
            
            for (int j = 0; j < 4; j++)
            {
                pixel_xy(img, cs[i].x, cs[i].y).p[j] = ps[i].p[j];
            }
        }

        save_image(img, image_name);
        free_image(img);
        return;
    }

    if (strcmp(tok, "line") == 0) {

        // An experiment, to first implement DDA with lines, before going to triangles

        // The two coordinate pairs
        int x0 = atoi(strok_r(NULL, " \t", &s));
        int x1 = atoi(strok_r(NULL, " \t", &s));
        int y0 = atoi(strok_r(NULL, " \t", &s));
        int y1 = atoi(strok_r(NULL, " \t", &s));
        
        
       
    }
}

//Implement this function to see which pixels were actually plotted
void dump_image(char *img_name) {
    image_t *img = load_image(img_name);
    for (int x = 0; x < img->width; x++) {
        for (int y = 0; y < img->height; y++) {
            pixel_t p = pixel_xy(img, x, y);

            //Avoid empty pixels
            if ((p.r == 0) && (p.g == 0) && (p.b == 0) && (p.a == 0)) continue;
            printf("x: %d, y: %d\n", x, y);
            printf("r: %d, g: %d, b: %d, a: %d\n\n", p.r, p.g, p.b, p.a);
        }
    }
}

void process_commands(char *filename) {
    int fd;
    off_t len;
    void* data;
    char *saveptr = NULL;
    int i = 0;
    char *line;
    array pixels = new_darr(pixel_t, 20);
    array coords = new_darr(point, 20);

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
        process_line(line, &pixels, &coords);
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
    array pixels = new_darr(pixel_t, 20);
    array coords = new_darr(point, 20);

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
