#ifndef OLISP_BUILTIN_H
#define OLISP_BUILTIN_H

#include "lval.h"

void register_builtins(lenv *e);
lval *builtin_eval(lenv *e, lval *a);
lval *builtin_list(lenv *e, lval *a);

#endif //OLISP_BUILTIN_H
