#include <stdio.h>
#include "shape.h"

void Rectangle_init(Rectangle* rect, float width, float height) {
    rect->width = width;
    rect->height = height;
}

float Rectangle_getSize(const Rectangle* rect) {
    return rect->width * rect->height;
}

void Rectangle_print(const Rectangle* rect) {
    printf("the size is %f\n", Rectangle_getSize(rect));
}
