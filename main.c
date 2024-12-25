#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL_ttf.h>
#include "parser.h"
#include "dynamic_string.h"

const char* allowed_chars = "1234567890+-*/^.()|= ";
const int allowed_chars_len = 21;

char* title = "Desmos";
int window_width = 800;
int window_height = 600;
int axis_x = 0;
int axis_y = 0;
double nums_per_pixel = 100;
int line_thickness;
int font_size = 24;
int expresion_font_size = 48;
int padding = 5;
int small_circle_r = 10;

int expresion_strs_cap = 10;
int expresion_strs_len = 1;
dynamic_string* expresion_strs;

int current_expresion_index = 0;
expresion_node* expressions;

int axis_step_n = 0;

SDL_Window* win = NULL;
SDL_Renderer* renderer = NULL;

TTF_Font* font = NULL;
TTF_Font* expresion_font = NULL;
SDL_Color text_color = {0, 0, 0};

bool running = true;

int init_window();

void draw_circle(int cx, int cy, int r);

void draw_line(int x1, int y1, int x2, int y2);

int digits_after_point(int n);

void num2text(double num, char* buffer, int n);

void draw_text(const char* text, int x, int y, bool cx, bool cy);

double axis_step(int n);

void draw_axis();

void update_expresion();

void draw_expresion();

void handle_events();

int main(int argc, char* argv[]) {
    if (init_window() != 0) {
        return 1;
    }

    expresion_strs = malloc(expresion_strs_cap*sizeof(dynamic_string));
    create_empty_dynamic_string(expresion_strs);
    expressions = malloc(expresion_strs_cap*sizeof(expresion_node));
    expressions->a = NULL;
    expressions->b = NULL;
    expressions->expresion = NULL;
    expressions->sign = '\0';

    set_t0();

    while (running) {

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        draw_axis();

        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        line_thickness = 3;

        for (int i = 0; i < expresion_strs_len; i++) {
            expresion_node* node = expressions + i;
            if (node == NULL) continue;
            for (int x = 1; x < window_width; x += 2) {
                for (int y = 0; y < window_height; y += 2) {
                    if (calculate_expresion_node(node, ((double)x-(double)window_width*0.5-(double)axis_x)/nums_per_pixel, -((double)y-(double)window_height*0.5-(double)axis_y)/nums_per_pixel) <= (double)line_thickness/nums_per_pixel) {
                        draw_circle(x, y, line_thickness);
                    }
                }
            }
        }

        draw_expresion();

        SDL_RenderPresent(renderer);
        SDL_UpdateWindowSurface(win);

        handle_events();
    }

    for (int i = 0; i < expresion_strs_len; i++) {
        dynamic_string_free(expresion_strs + i);
        expresion_node_free(expressions + i, false);
    }

    free(expresion_strs);
    free(expressions);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}

int init_window() {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("fuck you\n");
        return 1;
    }

    win = SDL_CreateWindow(
			    title,
			    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			    window_width, window_height,
			    SDL_WINDOW_SHOWN);
    if (win == NULL) {
        printf("fuck you\n");
        return 1;
    }

    renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == NULL) {
        printf("fuck you\n");
        return 1;
    }

    if (TTF_Init() != 0) {
        printf("fuck you\n");
        return 1;
    }

    font = TTF_OpenFont("font.ttf", font_size);

    if (font == NULL) {
        printf("fuck you\n");
        return 1;
    }

    expresion_font = TTF_OpenFont("font.ttf", expresion_font_size);

    if (expresion_font == NULL) {
        printf("fuck you\n");
        return 1;
    }

    return 0;
}

void draw_circle(int cx, int cy, int r) {
    for (int x = cx-r; x <= cx+r; x++) {
        for (int y = cy-r; y <= cy+r; y++) {
            int dx = cx - x;
            int dy = cy - y;
            if (dx*dx + dy*dy <= r*r) SDL_RenderDrawPoint(renderer, x, y);
        }
    }
}

void draw_line(int x1, int y1, int x2, int y2) {
    if (x1 == x2) {
        for (int y = y1; y <= y2; y++) {
            draw_circle(x1, y, line_thickness);
        }
        return;
    }
    if (y1 == y2) {
        for (int x = x1; x <= x2; x++) {
            draw_circle(x, y1, line_thickness);
        }
        return;
    }
    double k = (double)(y2 - y1)/(double)(x2 - x1);
    double b = y1 - k*x1;

    double step = SDL_min(1/SDL_fabs(k), 1);

    for (double x = (double)x1; x <= (double)x2; x += step) {
        double y = x*k + b;
        draw_circle((int)SDL_round(x), (int)SDL_round(y), line_thickness);
    }
}

void num2text(double num, char* buffer, int n) {
    char format[10];
    sprintf(format, "%%.%dlf", n);
    sprintf(buffer, format, num);
}

void draw_text(const char* text, int x, int y, bool cx, bool cy) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, text_color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    int w = font_size*SDL_strlen(text)/4;
    int h = font_size/2;

    SDL_Rect rect;
    rect.x = cx ? (x - w/2) : x;
    rect.y = cy ? (y - h/2) : y;
    rect.w = w;
    rect.h = h;

    SDL_RenderCopy(renderer, texture, NULL, &rect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

double axis_step(int n) {
    double k = 1.0;
    if (n >= 0) {
        for (int i = 0; i < n/3; i++) k *= 10.0;
    }
    else {
        for (int i = 0; i < -n/3 + (-n%3 == 0 ? 0 : 1); i++) k /= 10.0;
        while (n < 0) n += 3;
    }
    if (n % 3 == 0) {
        return k;
    }
    if (n % 3 == 1) {
        return 2.0*k;
    }
    return 5.0*k;
}

void draw_axis() {
    line_thickness = 1;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    int xc = (int)SDL_round((double)axis_x + window_width*0.5);
    int yc = (int)SDL_round((double)axis_y + window_height*0.5);

    double d = axis_step(axis_step_n);

    draw_text("0", xc + padding, yc + padding, false, false);

    while(d*nums_per_pixel < 75.0) {
        axis_step_n++;
        d = axis_step(axis_step_n);
    }
    
    while (d*nums_per_pixel > 100.0) {
        axis_step_n--;
        d = axis_step(axis_step_n);
    }

    for (int x = 0; x < window_width; x++) {
        draw_circle(x, yc, line_thickness);
    }
    for (int y = 0; y < window_height; y++) {
        draw_circle(xc, y, line_thickness);
    }

    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);

    int n1x = -(int)SDL_round(((double)axis_x + (double)window_width*0.5)/(nums_per_pixel*d));
    int n2x = -(int)SDL_round(((double)axis_x - (double)window_width*0.5)/(nums_per_pixel*d));

    for (int i = n1x; i <= n2x; i++) {
        int x = (int)SDL_round((double)i*d*nums_per_pixel + (double)window_width*0.5 + (double)axis_x);
        for (int y = 0; y < window_height; y++) {
            SDL_RenderDrawPoint(renderer, x, y);
        }
        if (i != 0) {
            char str[10];
            num2text((double)i*d, str, axis_step_n >= 0 ? 0 : -axis_step_n/3 + (-axis_step_n%3 == 0 ? 0 : 1));
            draw_text(str, x + padding, yc + padding, false, false);
        }
    }

    int n1y = -(int)SDL_round(((double)axis_y + (double)window_height*0.5)/(nums_per_pixel*d));
    int n2y = -(int)SDL_round(((double)axis_y - (double)window_height*0.5)/(nums_per_pixel*d));

    for (int i = n1y; i <= n2y; i++) {
        int y = (int)SDL_round((double)i*d*nums_per_pixel + (double)window_height*0.5 + (double)axis_y);
        for (int x = 0; x < window_width; x++) {
            SDL_RenderDrawPoint(renderer, x, y);
        }
        if (i != 0) {
            char str[10];
            num2text(-(double)i*d, str, axis_step_n >= 0 ? 0 : -axis_step_n/3 + (-axis_step_n%3 == 0 ? 0 : 1));
            draw_text(str, xc + padding, y + padding, false, false);
        }
    }
}

bool is_allowed(char c) {
    if (97 <= c && c <= 122) return true;
    for (int i = 0; i < allowed_chars_len; i++) {
        if (allowed_chars[i] == c) {
            return true;
        }
    }
    return false;
}

void update_expresion() {
    expresion_node* nodei = expressions;
    for (int i = 0; i < expresion_strs_len; i++) {
        int l;
        char* s = normalize_input(expresion_strs[i].data, &l);

        expresion_node_free(nodei, false);

        if (expresion_is_valid(s, l)) {
            expresion_node* node = parse(s, l);
            SDL_memcpy(nodei, node, sizeof(expresion_node));
            free(node);
        }
        else {
            nodei->a = NULL;
            nodei->b = NULL;
            nodei->expresion = NULL;
        }

        free(s);
        nodei++;
    }
}

void on_keyboard_input(SDL_KeyboardEvent event) {
    char c = event.keysym.sym;
    printf("%d\n", c);
    if (event.keysym.mod & 0x0001) {
        if (c == '=') c = '+';
        else if (c == '8') c = '*';
        else if (c == '6') c = '^';
        else if (c == '9') c = '(';
        else if (c == '0') c = ')';
        else if (c == '\\') c = '|';
    }
    if (c == 8) {
        if (expresion_strs[current_expresion_index].len > 0) {
            dynamic_string_delete_last_char(expresion_strs + current_expresion_index);
        }
        else if (expresion_strs_len > 1) {
            expresion_strs_len--;
            if (current_expresion_index == expresion_strs_len) current_expresion_index--;
        }
    }
    else if (c == 13) {
        expresion_strs_len++;
        if (expresion_strs_len >= expresion_strs_cap) {
            expresion_strs_cap *= 2;
            dynamic_string* new_strs = malloc(expresion_strs_cap*sizeof(dynamic_string));
            expresion_node* new_expresions = malloc(expresion_strs_cap*sizeof(expresion_node));
            SDL_memcpy(new_strs, expresion_strs, (expresion_strs_len-1)*sizeof(dynamic_string));
            SDL_memcpy(new_expresions, expressions, (expresion_strs_len-1)*sizeof(expresion_node));
            free(expresion_strs);
            free(expressions);
            expresion_strs = new_strs;
            new_expresions = expressions;
        }
        create_empty_dynamic_string(expresion_strs + expresion_strs_len - 1);
        expressions[expresion_strs_len - 1].a = NULL;
        expressions[expresion_strs_len - 1].b = NULL;
        expressions[expresion_strs_len - 1].expresion = NULL;
        expressions[expresion_strs_len - 1].sign = '\0';
        current_expresion_index++;
    }
    else if (c == 81) {
        if (current_expresion_index < expresion_strs_len - 1) current_expresion_index++;
    }
    else if (c == 82) {
        if (current_expresion_index > 0) current_expresion_index--;
    }
    if (is_allowed(c)) {
        dynamic_string_append(expresion_strs + current_expresion_index, c);
    }
    update_expresion();
}

void draw_expresion() {
    for (int i = 0; i < expresion_strs_len; i++) {
        if (expresion_strs[i].len == 0) continue;
        SDL_Surface* surface = TTF_RenderText_Solid(font, expresion_strs[i].data, text_color);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

        SDL_Rect rect;
        rect.x = padding;
        rect.y = window_height - surface->h - padding - (expresion_strs_len - 1 - i)*surface->h;
        rect.w = surface->w;
        rect.h = surface->h;

        SDL_RenderCopy(renderer, texture, NULL, &rect);

        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);

        if (i == current_expresion_index) {
            draw_circle(window_width - 2*small_circle_r - padding, window_height - surface->h/2 - padding - (expresion_strs_len - 1 - i)*surface->h, small_circle_r);
        }
    }
}

void handle_events() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
            continue;
        }
        if (event.type == SDL_MOUSEMOTION) {
            if (event.motion.state == SDL_PRESSED) {
                axis_x += event.motion.xrel;
                axis_y += event.motion.yrel;
            }
            continue;
        }
        if (event.type == SDL_MOUSEWHEEL) {
            int k = nums_per_pixel;
            if (event.wheel.y > 0) {
                nums_per_pixel *= 1.1;
                axis_x *= 1.1;
                axis_y *= 1.1;
            }
            if (event.wheel.y < 0) {
                nums_per_pixel /= 1.1;
                axis_x /= 1.1;
                axis_y /= 1.1;
            }
            continue;
        }
        if (event.type == SDL_KEYUP) {
            on_keyboard_input(event.key);
        }
    }
}
