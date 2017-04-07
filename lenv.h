#ifndef OLISP_LENV_H
#define OLISP_LENV_H

#include "lval.h"

lenv *lenv_copy(lenv *e);
void lenv_def(lenv *e, lval *k, lval *v);
void lenv_del(lenv *e);
lval* lenv_get(lenv *e, lval *k);
lenv *lenv_new(void);
void lenv_put(lenv *e, lval *k, lval *v);

#endif //OLISP_LENV_H
