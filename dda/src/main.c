#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <stdio.h>

int main() {
    // Create a 3D vector
    gsl_vector *v = gsl_vector_alloc(3);
    gsl_vector_set(v, 0, 1.0);
    gsl_vector_set(v, 1, 2.0);
    gsl_vector_set(v, 2, 3.0);
    
    printf("Vector: ");
    for (size_t i = 0; i < 3; i++) {
        printf("%g ", gsl_vector_get(v, i));
    }
    printf("\n");
    
    gsl_vector_free(v);
    return 0;
}