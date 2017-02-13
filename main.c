#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>

#include "mpc.h"


long eval_op(long x, char *op, long y) {
    if (strncmp(op, "+", 1) == 0) return x + y;
    if (strncmp(op, "-", 1) == 0) return x - y;
    if (strncmp(op, "*", 1) == 0) return x * y;
    if (strncmp(op, "/", 1) == 0) return x / y;
    if (strncmp(op, "%", 1) == 0) return x % y;
    if (strncmp(op, "^", 1) == 0) return pow(x, y);
    if (strncmp(op, "min", 3) == 0) return x < y? x: y;
    if (strncmp(op, "max", 3) == 0) return x > y? x: y;
}

long eval(mpc_ast_t *t) {
    if(strstr(t->tag, "number")) {
        return atoi(t->contents);
    }

    char *op = t->children[1]->contents;
    long x = eval(t->children[2]);

    int i = 3;
    if (strstr(t->children[i]->tag, "expr") == NULL && strncmp(op, "-", 1) == 0) {
        return -x;
    } else {
        /* iterate the remaining children and combining */
        while (strstr(t->children[i]->tag, "expr")) {
            x = eval_op(x, op, eval(t->children[i]));
            i ++;
        }
    }
    return x;
}

int main(int argc, char **argv) {

    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Operator = mpc_new("operator");
    mpc_parser_t *Expr = mpc_new("expr");
    mpc_parser_t *OLisp = mpc_new("olisp");

    mpc_result_t result;

    mpca_lang(MPCA_LANG_DEFAULT,
              "                                                                       \
               number   : /-?[0-9]+/;                                                 \
               operator : '+' | '-' | '*' | '/' | '%' | '^' | /min/ | /max/;          \
               expr     : <number> | '(' <operator> <expr>+ ')';                      \
               olisp    : /^/ <operator><expr>+ /$/;                                  \
              ",
              Number, Operator, Expr, OLisp);


    puts("OLisp v0");
    puts("Press Ctrl+c to exit.");
    while (1) {
        char *input = readline("()lisp> ");
        add_history(input);

        if (mpc_parse("<stdin>", input, OLisp, &result)) {
            printf("%li\n", eval(result.output));
            mpc_ast_delete(result.output);
        } else {
            mpc_err_print(result.error);
            mpc_err_delete(result.error);
        }
        free(input);
    }

    mpc_cleanup(4, Number, Operator, Expr, OLisp);
    return 0;
}