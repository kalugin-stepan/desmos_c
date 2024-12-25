#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdbool.h>
#include <malloc.h>
#include <Windows.h>
#include <SDL2/SDL.h>

#define NAN 0.0/0.0

typedef struct expresion_node expresion_node;

struct expresion_node {
    expresion_node* a;
    expresion_node* b;
    char sign;
    char* expresion;
};

void set_t0();
char* normalize_input(char* s0, int* s_l);
expresion_node* parse(char* s, int l);
void expresion_node_free(expresion_node* node, bool delete);
double char2num(char c);
double parse_number(char* s, int l);
bool expresion_is_valid(char* s, int l);
double calculate_expresion_node(expresion_node* node, double x, double y);

#endif