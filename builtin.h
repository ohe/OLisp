#ifndef OLISP_BUILTIN_H
#define OLISP_BUILTIN_H

#include "lval.h"

mpc_parser_t *Comment;
mpc_parser_t *Number;
mpc_parser_t *String;
mpc_parser_t *Symbol;
mpc_parser_t *Expr;
mpc_parser_t *Sexpr;
mpc_parser_t *Qexpr;
mpc_parser_t *OLisp;

void register_builtins(lenv *e);
lval *builtin_eval(lenv *e, lval *a);
lval *builtin_list(lenv *e, lval *a);
lval* builtin_load(lenv* e, lval* a);

#endif //OLISP_BUILTIN_H
