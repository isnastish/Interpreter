/*
 * sample.c — Demonstrates the C99 features the parser supports.
 */

/* ---- Enums ---- */

enum Direction { NORTH, SOUTH = 10, EAST, WEST };

/* ---- Structs ---- */

struct Point {
    int x;
    int y;
};

struct Node {
    int value;
    struct Node *next;
};

/* ---- Unions ---- */

union Value {
    int i;
    float f;
    char *s;
};

/* ---- Typedefs ---- */

typedef unsigned int uint;
typedef const char *cstr;
typedef struct Point Point;

/* ---- Global variables ---- */

int global_count = 0;
static int internal_flag = 1;
extern int shared_resource;

/* ---- Forward declarations ---- */

int add(int a, int b);
void print_point(struct Point p);

/* ---- Functions ---- */

int add(int a, int b) {
    return a + b;
}

int factorial(int n) {
    if (n <= 1)
        return 1;
    else
        return n * factorial(n - 1);
}

int fibonacci(int n) {
    int a = 0;
    int b = 1;
    for (int i = 0; i < n; i++) {
        int tmp = b;
        b = a + b;
        a = tmp;
    }
    return a;
}

void sort(int *arr, int len) {
    int i;
    for (i = 0; i < len - 1; i++) {
        int j;
        for (j = i + 1; j < len; j++) {
            if (arr[j] < arr[i]) {
                int tmp = arr[i];
                arr[i] = arr[j];
                arr[j] = tmp;
            }
        }
    }
}

int classify(int x) {
    switch (x) {
        case 0:
            return 0;
        case 1:
        case 2:
            return 1;
        default:
            return -1;
    }
}

void loops(void) {
    int i = 0;

    while (i < 10) {
        i++;
    }

    do {
        i--;
    } while (i > 0);

    for (i = 0; i < 100; i++) {
        if (i % 2 == 0)
            continue;
        if (i > 50)
            break;
    }
}

int using_goto(int n) {
    int result = 1;
    int i = 1;
loop:
    if (i > n) goto done;
    result = result * i;
    i++;
    goto loop;
done:
    return result;
}

int main(int argc, char **argv) {
    int x = add(3, 4);
    int f = factorial(5);
    int fib = fibonacci(10);

    struct Point p;
    p.x = x;
    p.y = f;

    int *ptr = &x;
    *ptr = 42;

    int a = (x > 10) ? 1 : 0;
    int b = sizeof(int);
    int c = ~a | (b & 0xFF);
    int d = x << 2;

    return 0;
}
