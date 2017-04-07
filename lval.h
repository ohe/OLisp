#ifndef OLISP_LVAL_H
#define OLISP_LVAL_H

#include "mpc.h"

enum LVAL_TYPE {LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_FUN, LVAL_SEXPR, LVAL_QEXPR};

typedef struct lval lval;
typedef struct lenv lenv;

typedef lval *(*lbuiltin)(lenv *, lval *);

struct lenv {
    lenv *par;
    int count;
    char **syms;
    lval **vals;
};

struct lval {
    int type;
    long num;
    char *err;
    char *sym;

    lbuiltin builtin;
    lenv *env;
    lval *formals;
    lval *body;

    /* Count and Pointer to a list of `lval*` */
    int count;
    lval **cell;
};

#include "lenv.h"

lval *lval_add(lval *v, lval *x);
lval *lval_copy(lval *v);
void lval_del(lval *v);
lval *lval_err(char *fmt, ...);
lval *lval_eval(lenv *e, lval *v);
lval *lval_eval_sexpr(lenv *e, lval *v);
lval *lval_func(lbuiltin func);
lval *lval_join(lval *x, lval *y);
lval *lval_lambda(lval* formals, lval *body);
lval *lval_num(long x);
lval *lval_pop(lval *v, int i);
void lval_print(lval *v);
void lval_println(lval *v);
lval* lval_read(mpc_ast_t *t);
lval *lval_sexpr(void);
lval *lval_sym(char *symbol);
lval *lval_take(lval *v, int i);

#endif //OLISP_LVAL_H
