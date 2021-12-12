#ifndef SHAPE_H
#define SHAPE_H

typedef struct _Rectangle {
    float width;
    float height;
} Rectangle;

void Rectangle_init(Rectangle* rect, float width, float height);
float Rectangle_getSize(const Rectangle* rect);
void Rectangle_print(const Rectangle* rect);

#endif // !SHAPE_H