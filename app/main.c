#include "shape.h"

int main(void) {
    Rectangle rect;
    Rectangle_init(&rect, 10, 30);
    Rectangle_print(&rect);

    return 0;
}
