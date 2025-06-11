#include "CVector.h"

int main(void) {
    vector(int) vec;

    vector_init(vec);

    vector_push_back(vec,5);
    vector_push_back(vec,12);
    vector_push_back(vec,13);
    vector_push_back_args(vec,14,48,50);

    vector_foreach(vec,item)
        printf("%d, ", *item);

    vector_destroy(vec);
    return 0;
}
// output -> 5, 12, 13, 14, 48, 50, 
