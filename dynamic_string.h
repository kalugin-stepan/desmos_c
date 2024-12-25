#ifndef DYNAMIC_STRING_H
#define DYNAMIC_STRING

#include <malloc.h>
#include <SDL2/SDL.h>

typedef struct dynamic_string dynamic_string;

struct dynamic_string {
    char* data;
    int len;
    int cap;
};

void create_empty_dynamic_string(dynamic_string* s) {
    s->cap = 10;
    s->len = 0;
    s->data = (char*)malloc(s->cap);
}

void dynamic_string_append(dynamic_string* s, char c) {
    s->len++;
    if (s->len >= s->cap) {
        s->cap *= 2;
        char* new_s = (char*)malloc(s->cap);
        SDL_memcpy(new_s, s->data, s->len-1);
        free(s->data);
        s->data = new_s;
    }
    s->data[s->len] = '\0';
    s->data[s->len-1] = c;
}

void dynamic_string_delete_last_char(dynamic_string* s) {
    if (s->len <= 0) return;
    s->len--;
    s->data[s->len] = '\0';
}

void dynamic_string_free(dynamic_string* s) {
    free(s->data);
}

#endif