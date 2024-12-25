#include <stdio.h>
#include "parser.h"

int main(int argc, char* argv[]) {
    char* s0 = "y=(x-1)*(x+2)";

    int l;

    char* s = normalize_input(s0, &l);

    expresion_node* node = parse(s, l);

    double x = calculate_expresion_node(node, 0, 0);

    printf("%g\n", x);

    free(s);
}