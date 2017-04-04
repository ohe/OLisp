#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>

#include "mpc.h"

enum LVAL_TYPE {LVAL_NUM, LVAL_ERR};
enum ERROR_TYPE {LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM};

typedef struct {
    int type;
    /* Union are more efficient in memory as they create space for its largest element
     * the counterpart is that only one field can be created/accessed at the same time
     * This fits the lval struct because, depending of the context, only num or err will
     * be set
     */
    union {
        long num;
        int err;
    };
} lval;

lval lval_num(long x) {
    lval v;
    v.type = LVAL_NUM;
    v.num = x;
    return v;
}

lval lval_err(int x) {
    lval v;
    v.type = LVAL_ERR;
    v.err = x;
    return v;
}

void lval_print(lval v) {
    switch (v.type) {
        case LVAL_NUM:
            printf("%li", v.num);
            break;
        case LVAL_ERR:
            if (v.err == LERR_DIV_ZERO) printf("Error: Division by zero!");
            if (v.err == LERR_BAD_OP) printf("Error: Invalid Operator!");
            if (v.err == LERR_BAD_NUM) printf("Error: Invalid Number!");
            break;
    }
}
void lval_println(lval v) {
    lval_print(v);
    putchar('\n');
}

lval eval_op(lval x, char *op, lval y) {
    if (x.type == LVAL_ERR) return x;
    if (y.type == LVAL_ERR) return y;
    if (strncmp(op, "+", 1) == 0) return lval_num(x.num + y.num);
    if (strncmp(op, "-", 1) == 0) return lval_num(x.num - y.num);
    if (strncmp(op, "*", 1) == 0) return lval_num(x.num * y.num);
    if (strncmp(op, "/", 1) == 0) return y.num == 0 ? lval_err(LERR_DIV_ZERO): lval_num(x.num / y.num);
    if (strncmp(op, "%", 1) == 0) return lval_num(x.num % y.num);
    if (strncmp(op, "^", 1) == 0) return lval_num(pow(x.num, y.num));
    if (strncmp(op, "add", 1) == 0) return lval_num(x.num + y.num);
    if (strncmp(op, "sub", 1) == 0) return lval_num(x.num - y.num);
    if (strncmp(op, "mul", 1) == 0) return lval_num(x.num * y.num);
    if (strncmp(op, "div", 1) == 0) return y.num == 0 ? lval_err(LERR_DIV_ZERO): lval_num(x.num / y.num);
    if (strncmp(op, "min", 3) == 0) return x.num < y.num? x: y;
    if (strncmp(op, "max", 3) == 0) return x.num > y.num? x: y;

    return lval_err(LERR_BAD_OP);

}

lval eval(mpc_ast_t *t) {
    if(strstr(t->tag, "number")) {
        errno = 0;
        long x = strtol(t->contents, NULL, 10);
        return errno == ERANGE ? lval_err(LERR_BAD_NUM): lval_num(x);
    }

    char *op = t->children[1]->contents;
    lval x = eval(t->children[2]);

    int i = 3;
    if (strstr(t->children[i]->tag, "expr") == NULL && strncmp(op, "-", 1) == 0) {
        return lval_num(-x.num);
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
              "                                                                                              \
               number   : /-?[0-9]+/;                                                                       \
               operator : '+' | '-' | '*' | '/' | '%' | '^' | /min/ | /max/ | /add/ | /sub/ | /mul/ | /div/; \
               expr     : <number> | '(' <operator> <expr>+ ')';                                             \
               olisp    : /^/ <operator><expr>+ /$/;                                                         \
              ",
              Number, Operator, Expr, OLisp);


    puts("OLisp v0");
    puts("Press Ctrl+c to exit.");
    while (1) {
        char *input = readline("()lisp> ");
        add_history(input);

        if (mpc_parse("<stdin>", input, OLisp, &result)) {
            mpc_ast_print(result.output);
            lval_println(eval(result.output));
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