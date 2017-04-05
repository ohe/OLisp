#ifndef OLISP_LVAL_H
#define OLISP_LVAL_H

#include "mpc.h"

enum LVAL_TYPE {LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR, LVAL_QEXPR};

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

lval *lval_add(lval *v, lval *x);
void lval_del(lval *v);
lval *lval_err(char *message);
lval *lval_eval(lval *v);
lval *lval_eval_sexpr(lval *v);
lval *lval_join(lval *x, lval *y);
lval *lval_num(long x);
lval *lval_pop(lval *v, int i);
void lval_print(lval *v);
void lval_println(lval *v);
lval* lval_read(mpc_ast_t *t);
lval *lval_take(lval *v, int i);

#endif //OLISP_LVAL_H
