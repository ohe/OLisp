#ifndef OLISP_LENV_H
#define OLISP_LENV_H

#include "lval.h"

void lenv_put(lenv *e, lval *k, lval *v);
lval* lenv_get(lenv *e, lval *k);
void lenv_del(lenv *e);
lenv *lenv_new(void);

#endif //OLISP_LENV_H
