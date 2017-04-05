#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>
#include <stdbool.h>

#include "mpc.h"

enum LVAL_TYPE {LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR};

typedef struct lval lval;

struct lval {
    int type;
    long num;
    /* Error and Symbol types have some string data */
    char *err;
    char *sym;
    /* Count and Pointer to a list of `lval*` */
    int count;
    lval **cell;
};

lval *lval_num(long x) {
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->num = x;
    return v;
}

lval *lval_err(char *message) {
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_ERR;
    v->err = malloc(strlen(message) +1);
    strcpy(v->err, message);
    return v;
}

lval *lval_sym(char *symbol) {
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_SYM;
    v->sym = malloc(strlen(symbol) + 1);
    strcpy(v->sym, symbol);
    return v;
}

lval *lval_sexpr(void) {
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

void lval_del(lval *v) {
    switch (v->type) {
        case LVAL_NUM:
            break;
        case LVAL_ERR:
            free(v->err);
            break;
        case LVAL_SYM:
            free(v->sym);
            break;
        case LVAL_SEXPR:
            for (int i=0; i < v->count; i++) {
                lval_del(v->cell[i]);
            }
            free(v->cell);
            break;
    }
    free(v);
}

lval *lval_read_num(mpc_ast_t *t) {
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno == ERANGE ? lval_err("Invalid Number"): lval_num(x);
}

/* To avoid implicit declaration issues */
void lval_print(lval *v);
lval *lval_eval_sexpr(lval *v);

void lval_expr_print(lval *v, char open, char close) {
    putchar(open);
    for(int i = 0; i < v->count; i++) {
        lval_print(v->cell[i]);
        if (i != v->count -1) {
            putchar(' ');
        }
    }
    putchar(close);
}

void lval_print(lval *v) {
    switch(v->type) {
        case LVAL_NUM: printf("%li", v->num); break;
        case LVAL_ERR: printf("Error: %s", v->err); break;
        case LVAL_SYM: printf("%s", v->sym); break;
        case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
    }
}

void lval_println(lval *v) {
    lval_print(v);
    putchar('\n');
}

lval *lval_eval(lval *v) {
    if (v->type == LVAL_SEXPR) {
        return lval_eval_sexpr(v);
    }
    return v;
}

lval * lval_pop(lval *v, int i) {
    lval *x = v->cell[i];
    memmove(&v->cell[i], &v->cell[i + 1], sizeof(lval*) * (v->count -i -1));
    v->count--;
    v->cell = realloc(v->cell, sizeof(lval *) * v->count);
    return x;
}

lval *lval_take(lval *v, int i) {
    lval *x = lval_pop(v, i);
    lval_del(v);
    return x;
}

lval *eval_op(lval *x, char *op, lval*y) {
    if (strncmp(op, "+", 1) == 0 || strncmp(op, "add", 3) == 0) x->num += y->num;
    if (strncmp(op, "-", 1) == 0 || strncmp(op, "sub", 3) == 0) x->num -= y->num;
    if (strncmp(op, "*", 1) == 0 || strncmp(op, "mul", 3) == 0) x->num *= y->num;
    if (strncmp(op, "/", 1) == 0 || strncmp(op, "div", 3) == 0) {
        if (y->num == 0) {
            lval_del(x);
            x = lval_err("Division by Zero!");
        } else {
            x->num /= y->num;
        }
    }
    if (strncmp(op, "%", 1) == 0) x->num %= y->num;
    if (strncmp(op, "^", 1) == 0) x-> num = pow(x->num, y->num);
    if (strncmp(op, "min", 3) == 0) {
        if (x->num > y->num) {
            x->num = y->num;
        }
    }
    if (strncmp(op, "max", 3) == 0) {
        if (x->num < y->num) {
            x->num = y->num;
        }
    }
    return x;
}

lval *builtin_op(lval *v, char *op) {

    for (int i = 0; i < v->count; i++) {
        if (v->cell[i]->type != LVAL_NUM) {
            lval_del(v);
            return lval_err("Cannot Operate on non-number");
        }
    }

    lval *x = lval_pop(v, 0);

    if (strcmp(op, "-") == 0 && v->count == 0) {
        x->num = -x->num;
    }
    while (v->count > 0) {
        lval *y = lval_pop(v, 0);
        x = eval_op(x, op, y);
        lval_del(y);
    }
    lval_del(v);
    return x;
}

lval * lval_eval_sexpr(lval *v) {
    /* Evaluate Children */
    for (int i = 0; i < v->count; i++) {
        v->cell[i] = lval_eval(v->cell[i]);
    }

    /* Error Checking */
    for (int i = 0; i < v->count; i++) {
        if (v->cell[i]->type == LVAL_ERR) return lval_take(v, i);
    }

    /* Empty Expression */
    if (v->count == 0) return v;

    /* Single Expression */
    if (v->count == 1) return lval_take(v, 0);

    /* Ensure First Element is Symbol */
    lval *first = lval_pop(v, 0);
    if (first->type != LVAL_SYM) {
        lval_del(first);
        lval_del(v);
        return lval_err("S-Expression does not start with symbol'");
    }

    /* Call Builtin with operator */
    lval *result = builtin_op(v, first->sym);
    lval_del(first);
    return result;
}

bool control_chars(mpc_ast_t *t, char *chars) {
    bool rc = false;
    for (int i=0; i < strlen(chars); i ++) {
        if (t->contents[0] == chars[i]) {
            rc = true;
            break;
        }
    }
    return rc;
}

lval *lval_add(lval *v, lval *x) {
    v->count++;
    v->cell = realloc(v->cell, sizeof(lval *) * v->count);
    v->cell[v->count - 1] = x;
    return v;
}

lval* lval_read(mpc_ast_t *t) {

    if (strstr(t->tag, "number")) return lval_read_num(t);
    if (strstr(t->tag, "symbol")) return lval_sym(t->contents);

    lval *x = NULL;
    if (strcmp(t->tag, ">") != 0 || strcmp(t->tag, "sexpr") != 0) x = lval_sexpr();

    for (int i = 0; i < t->children_num; i++) {
        if (!control_chars(t->children[i], "(){}") && strcmp(t->children[i]->tag, "regex") != 0) {
            x = lval_add(x, lval_read(t->children[i]));
        }
    }
    return x;
}

int main(int argc, char **argv) {

    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Symbol = mpc_new("symbol");
    mpc_parser_t *Expr = mpc_new("expr");
    mpc_parser_t *Sexpr = mpc_new("sexpr");
    mpc_parser_t *OLisp = mpc_new("olisp");

    mpc_result_t result;

    mpca_lang(MPCA_LANG_DEFAULT,
              "                                                                                              \
               number   : /-?[0-9]+/;                                                                        \
               symbol   : '+' | '-' | '*' | '/' | '%' | '^' | /min/ | /max/ | /add/ | /sub/ | /mul/ | /div/; \
               expr     : <number> | <symbol> | <sexpr>;                                                     \
               sexpr    : '(' <expr>* ')';                                                                   \
               olisp    : /^/ <expr>* /$/;                                                                   \
              ",
              Number, Symbol, Expr, Sexpr, OLisp);


    puts("OLisp v0");
    puts("Press Ctrl+c to exit.");
    while (1) {
        char *input = readline("()lisp> ");
        add_history(input);

        if (mpc_parse("<stdin>", input, OLisp, &result)) {
            /* mpc_ast_print(result.output); */
            lval *x = lval_eval(lval_read(result.output));
            lval_println(x);
            lval_del(x);
            mpc_ast_delete(result.output);
        } else {
            mpc_err_print(result.error);
            mpc_err_delete(result.error);
        }
        free(input);
    }

    mpc_cleanup(5, Number, Symbol, Expr, Sexpr, OLisp);
    return 0;
}