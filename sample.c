enum Color { RED, GREEN = 10, BLUE };

struct Point {
    int x;
    int y;
};

int global_count = 42;

int add(int a, int b) {
    return a + b;
}

int factorial(int n) {
    if (n <= 1)
        return 1;
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

int gcd(int a, int b) {
    while (b != 0) {
        int t = b;
        b = a % b;
        a = t;
    }
    return a;
}

int is_prime(int n) {
    if (n < 2) return 0;
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return 0;
    }
    return 1;
}

void fizzbuzz(int limit) {
    for (int i = 1; i <= limit; i++) {
        if (i % 15 == 0)
            printf("FizzBuzz ");
        else if (i % 3 == 0)
            printf("Fizz ");
        else if (i % 5 == 0)
            printf("Buzz ");
        else
            printf("%d ", i);
    }
    printf("\n");
}

int classify(int x) {
    switch (x) {
        case 0:  return 0;
        case 1:
        case 2:  return 1;
        default: return -1;
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

int power(int base, int exp) {
    int result = 1;
    for (int i = 0; i < exp; i++) {
        result = result * base;
    }
    return result;
}

int main(int argc, char **argv) {
    printf("=== C99 Interpreter Demo ===\n\n");

    printf("--- Arithmetic ---\n");
    printf("add(3, 4) = %d\n", add(3, 4));
    printf("add(-10, 25) = %d\n", add(-10, 25));
    printf("2 + 3 * 4 = %d\n", 2 + 3 * 4);
    printf("(2 + 3) * 4 = %d\n", (2 + 3) * 4);
    printf("17 mod 5 = %d\n", 17 % 5);
    printf("global_count = %d\n\n", global_count);

    printf("--- Recursion ---\n");
    printf("factorial(0) = %d\n", factorial(0));
    printf("factorial(1) = %d\n", factorial(1));
    printf("factorial(5) = %d\n", factorial(5));
    printf("factorial(10) = %d\n", factorial(10));
    printf("\n");

    printf("--- Fibonacci (first 15) ---\n");
    for (int i = 0; i < 15; i++) {
        printf("%d ", fibonacci(i));
    }
    printf("\n\n");

    printf("--- GCD ---\n");
    printf("gcd(48, 18) = %d\n", gcd(48, 18));
    printf("gcd(100, 75) = %d\n", gcd(100, 75));
    printf("\n");

    printf("--- Primes up to 50 ---\n");
    for (int i = 2; i <= 50; i++) {
        if (is_prime(i))
            printf("%d ", i);
    }
    printf("\n\n");

    printf("--- FizzBuzz (1-20) ---\n");
    fizzbuzz(20);
    printf("\n");

    printf("--- Power ---\n");
    printf("2^10 = %d\n", power(2, 10));
    printf("3^5 = %d\n", power(3, 5));
    printf("\n");

    printf("--- Loops ---\n");
    int sum = 0;
    for (int i = 1; i <= 100; i++) {
        sum = sum + i;
    }
    printf("sum(1..100) = %d\n", sum);

    int countdown = 5;
    printf("do-while countdown: ");
    do {
        printf("%d ", countdown);
        countdown--;
    } while (countdown > 0);
    printf("\n");

    printf("while with break: ");
    int w = 0;
    while (1) {
        if (w >= 10) break;
        printf("%d ", w);
        w++;
    }
    printf("\n");

    printf("for with continue (odds only): ");
    for (int i = 0; i < 20; i++) {
        if (i % 2 == 0) continue;
        printf("%d ", i);
    }
    printf("\n\n");

    printf("--- Switch ---\n");
    printf("classify(0) = %d\n", classify(0));
    printf("classify(1) = %d\n", classify(1));
    printf("classify(2) = %d\n", classify(2));
    printf("classify(99) = %d\n", classify(99));
    printf("\n");

    printf("--- Goto ---\n");
    printf("goto_factorial(5) = %d\n", using_goto(5));
    printf("goto_factorial(7) = %d\n", using_goto(7));
    printf("\n");

    printf("--- Ternary ---\n");
    int a = 10;
    int b = 20;
    int max = (a > b) ? a : b;
    printf("max(%d, %d) = %d\n", a, b, max);
    printf("\n");

    printf("--- Bitwise ---\n");
    printf("0xFF & 0x0F = %d\n", 0xFF & 0x0F);
    printf("0xAA | 0x55 = %d\n", 0xAA | 0x55);
    printf("1 << 10 = %d\n", 1 << 10);
    printf("1024 >> 3 = %d\n", 1024 >> 3);
    printf("\n");

    printf("--- Enums ---\n");
    printf("RED = %d\n", RED);
    printf("GREEN = %d\n", GREEN);
    printf("BLUE = %d\n", BLUE);
    printf("\n");

    printf("--- Structs ---\n");
    struct Point p;
    p.x = 42;
    p.y = 17;
    printf("Point p = {%d, %d}\n", p.x, p.y);
    printf("\n");

    printf("--- Pointers ---\n");
    int x = 100;
    int *ptr = &x;
    printf("x = %d, *ptr = %d\n", x, *ptr);
    *ptr = 200;
    printf("after *ptr = 200: x = %d\n", x);
    printf("\n");

    printf("--- Compound Assignment ---\n");
    int v = 10;
    v += 5;
    printf("v += 5  -> %d\n", v);
    v -= 3;
    printf("v -= 3  -> %d\n", v);
    v *= 2;
    printf("v *= 2  -> %d\n", v);
    v /= 4;
    printf("v /= 4  -> %d\n", v);
    printf("\n");

    printf("--- Increment/Decrement ---\n");
    int n = 5;
    printf("n = %d\n", n);
    printf("++n = %d\n", ++n);
    printf("n++ = %d\n", n++);
    printf("n is now %d\n", n);
    printf("--n = %d\n", --n);
    printf("\n");

    printf("=== All demos complete! ===\n");
    return 0;
}
