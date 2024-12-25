#include "parser.h"

#define E 2.7182818284590455
#define PI 3.141592653589793

double t0;

double time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return 1000*(tv.tv_sec) + (tv.tv_usec)/1000;
}

void set_t0() {
    t0 = time_ms();
}

char* normalize_input(char* s0, int* s_l) {
    int s0_len = SDL_strlen(s0);

    int s_len = 0;

    for (int i = 0; i < s0_len; i++)
        if (s0[i] != ' ') s_len++;
    
    char* s = malloc(s_len+1);
    s[s_len] = '\0';
    int j = 0;
    for (int i = 0; i < s0_len; i++) {
        if (s0[i] != ' ') {
            s[j] = s0[i];
            j++;
        }
    }
    *s_l = s_len;
    return s;
}

expresion_node* parse(char* s, int l) {
    expresion_node* node = malloc(sizeof(expresion_node));
    for (int i = 0; i < l; i++) {
        if (s[i] == '=') {
            expresion_node* a = parse(s, i);
            expresion_node* b = parse(s+i+1, l-i-1);
            node->a = a;
            node->b = b;
            node->sign = s[i];
            node->expresion = NULL;
            return node;
        }
    }

    int n = 0;
    bool an = false;
    for (int i = 0; i < l; i++) {
        if (s[i] == '(') n++;
        if (s[i] == ')') n--;
        if (n > 0) continue;
        if (n < 0) return NULL;
        if (s[i] == '|') an = !an;
        if (an) continue;
        if (s[i] == '+') {
            expresion_node* a = parse(s, i);
            expresion_node* b = parse(s+i+1, l-i-1);
            node->a = a;
            node->b = b;
            node->sign = s[i];
            node->expresion = NULL;
            return node;
        }
        if (s[i] == '-' && i > 0) {
            expresion_node* a = parse(s, i);
            expresion_node* b = parse(s+i, l-i);
            node->a = a;
            node->b = b;
            node->sign = '+';
            node->expresion = NULL;
            return node;
        }
        if (s[i] == '-' && i == 0) {
            expresion_node* a = malloc(sizeof(expresion_node));
            expresion_node* b = parse(s+i+1, l-i-1);
            a->a = NULL;
            a->b = NULL;
            a->sign = '\0';
            a->expresion = malloc(2);
            a->expresion[0] = '0';
            a->expresion[1] = '\0';
            node->a = a;
            node->b = b;
            node->sign = '-';
            node->expresion = NULL;
            return node;
        }
    }
    if (n != 0 || an) {
        free(node);
        return NULL;
    }

    for (int i = 0; i < l; i++) {
        if (s[i] == '(') n++;
        if (s[i] == ')') n--;
        if (n > 0) continue;
        if (n < 0) return NULL;
        if (s[i] == '|') an = !an;
        if (an) continue;
        if (s[i] == '*' || s[i] == '/') {
            expresion_node* a = parse(s, i);
            expresion_node* b = parse(s+i+1, l-i-1);
            node->a = a;
            node->b = b;
            node->sign = s[i];
            node->expresion = NULL;
            return node;
        }
    }
    if (n != 0 || an) {
        free(node);
        return NULL;
    }

    for (int i = 0; i < l; i++) {
        if (s[i] == '(') n++;
        if (s[i] == ')') n--;
        if (n > 0) continue;
        if (n < 0) return NULL;
        if (s[i] == '|') an = !an;
        if (an) continue;
        if (s[i] == '^') {
            expresion_node* a = parse(s, i);
            expresion_node* b = parse(s+i+1, l-i-1);
            node->a = a;
            node->b = b;
            node->sign = s[i];
            node->expresion = NULL;
            return node;
        }
    }
    if (n != 0 || an) {
        free(node);
        return NULL;
    }

    if (s[0] == '(' && s[l-1] == ')') {
        free(node);
        return parse(s + 1, l-2);
    }

    if (l > 6 && s[0] == 's' && s[1] == 'q' && s[2] == 'r' && s[3] == 't') {
        node->a = parse(s+5, l-6);
        node->b = NULL;
        node->expresion = NULL;
        node->sign = 'q';
        return node;
    }

    if (l > 5 && s[0] == 's' && s[1] == 'i' && s[2] == 'n') {
        node->a = parse(s+4, l-5);
        node->b = NULL;
        node->expresion = NULL;
        node->sign = 's';
        return node;
    }

    if (l > 5 && s[0] == 'c' && s[1] == 'o' && s[2] == 's') {
        node->a = parse(s+4, l-5);
        node->b = NULL;
        node->expresion = NULL;
        node->sign = 'c';
        return node;
    }

    if (l > 5 && s[0] == 'l' && s[1] == 'o' && s[2] == 'g') {
        node->a = parse(s+4, l-5);
        node->b = NULL;
        node->expresion = NULL;
        node->sign = 'l';
        return node;
    }

    if (l > 4 && s[0] == 'l' && s[1] == 'n') {
        node->a = parse(s+3, l-4);
        node->b = NULL;
        node->expresion = NULL;
        node->sign = 'n';
        return node;
    }

    if (l > 7 && s[0] == 'r' && s[1] == 'o' && s[2] == 'u' && s[3] == 'n' && s[4] == 'd') {
        node->a = parse(s+6, l-7);
        node->b = NULL;
        node->expresion = NULL;
        node->sign = 'r';
        return node;
    }

    if (l > 2 && s[0] == '|' && s[l-1] == '|') {
        node->a = parse(s+1, l-2);
        node->b = NULL;
        node->expresion = NULL;
        node->sign = 'a';
        return node;
    }

    node->a = NULL;
    node->b = NULL;
    node->sign = '\0';
    node->expresion = malloc(l+1);
    memcpy(node->expresion, s, l);
    node->expresion[l] = '\0';
    return node;
}

void expresion_node_free(expresion_node* node, bool delete) {
    if (node == NULL) return;
    if (node->a != NULL) {
        expresion_node_free(node->a, true);
    }
    if (node->b != NULL) {
        expresion_node_free(node->b, true);
    }
    if (node->expresion != NULL) free(node->expresion);
    if (delete) free(node);
}

double char2num(char c) {
    if (c == '0') return 0.0;
    if (c == '1') return 1.0;
    if (c == '2') return 2.0;
    if (c == '3') return 3.0;
    if (c == '4') return 4.0;
    if (c == '5') return 5.0;
    if (c == '6') return 6.0;
    if (c == '7') return 7.0;
    if (c == '8') return 8.0;
    if (c == '9') return 9.0;
    return NAN;
}

double parse_number(char* s, int l) {
    if (l == 1 && s[0] == 'e') return E;
    if (l == 2 && s[0] == 'p' && s[1] == 'i') return PI;
    double sign = 1;
    if (s[0] == '-') {
        sign = -1;
        s += 1;
    }
    double val = 0;
    double p = 1.0;
    int point_index = -1;
    for (int i = 0; i < l; i++) {
        if (s[i] == '.') {
            point_index = i;
            break;
        }
    }
    if (point_index == -1) {
        for (int i = l-1; i >= 0; i--) {
            double digit = char2num(s[i]);
            if (digit == NAN) return NAN;
            val += digit*p;
            p *= 10.0;
        }
        return sign*val;
    }
    for (int i = point_index-1; i >= 0; i--) {
        double digit = char2num(s[i]);
        if (digit == NAN) return NAN;
        val += digit*p;
        p *= 10.0;
    }
    p = 0.1;
    for (int i = point_index+1; i < l; i++) {
        val += char2num(s[i])*p;
        p /= 10.0;
    }
    return sign*val;
}

bool expresion_is_valid(char* s, int l) {
    int c = 0;
    int n = 0;
    bool an = false;
    bool x = false;
    bool y = false;
    for (int i = 0; i < l; i++) {
        if (s[i] == '=') c++;
        else if (s[i] == '(') n++;
        else if (s[i] == ')') n--;
        else if (s[i] == '|') an = !an;
        else if (s[i] == 'x') x = true;
        else if (s[i] == 'y') y = true;;
        if (n < 0) return false;
    }
    return c == 1 && n == 0 && !an && x && y;
}

double calculate_expresion_node(expresion_node* node, double x, double y) {
    if (node == NULL) return NAN;
    if (node->a != NULL && node->b != NULL) {
        double a = calculate_expresion_node(node->a, x, y);
        double b = calculate_expresion_node(node->b, x, y);
        if (node->sign == '+') return a + b;
        if (node->sign == '-') return a - b;
        if (node->sign == '*') return a * b;
        if (node->sign == '/') return a / b;
        if (node->sign == '^') return SDL_pow(a, b);
        if (node->sign == '=') return SDL_fabs(a-b);
    }
    if (node->a != NULL && node->b == NULL) {
        double a = calculate_expresion_node(node->a, x, y);
        if (node->sign == 'q') return SDL_sqrt(a);
        if (node->sign == 's') return SDL_sin(a); 
        if (node->sign == 'c') return SDL_cos(a);
        if (node->sign == 'a') return SDL_fabs(a);
        if (node->sign == 'l') return SDL_log10(a);
        if (node->sign == 'n') return SDL_log(a);
        if (node->sign == 'r') return SDL_round(a);
    }
    if (node->expresion == NULL) return NAN;
    if (SDL_strlen(node->expresion) == 1) {
        if (node->expresion[0] == 'x') return x;
        if (node->expresion[0] == 'y') return y;
        if (node->expresion[0] == 't') {
            double t = time_ms();
            double dt = t - t0;
            return dt;
        }
    }
    return parse_number(node->expresion, SDL_strlen(node->expresion));
}