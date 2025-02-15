#include <stdio.h>
#include "tau.h"

int main(void) {
    const char *expressions[] = {
        "(a b c)",
        "(define (my-add x y) (+ x y))",
        "(printf \"Hello World! \")",
        "(define x \"abcdef\")",
        "(struct point-2d ((x i8) (y i8)))",
        "(vec-with-capacity i8 100)",
        "(vec-with-elems i8 '(1 2 3))",
        "foo",
        "`(list ,(today-date) ,(tomm-date))",
        "`(a b ,@(get-rest))",
        "(let ([x 50] [y 100]) (+ x y))",
        "#f",
        "#t",
        "nil",
        "(define x #f)",
        "(define y #t)",
        "(define x nil)",
        "(* 10 5)",
        "(+ 1 (+ 2 3) (+ 4 5))",
        "(+ 1 2)",
        "(- 20 3)",
        "20",
        "42.5",
        "#f",
        "#t",
        NULL
    };
    

    Buffer *buf __attribute__ ((__cleanup__(buffer_cleanup))) = buffer_create(sizeof(Marker), 1024);
    for (const char **p = expressions; *p != NULL; p++) {
        buffer_clear(buf);
        printf("Expression: %s\n", *p);
        if (read_markers(*p, buf) != RETURN_STATUS_SUCCESS) {
            printf("Error reading markers.\n");
        } else {
            pretty_print_markers(buf, *p);
        }
        eval_buffer(buf, *p);
        printf("\n\n");
    }
    
    return 0;
}
