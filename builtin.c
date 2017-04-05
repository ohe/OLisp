#include <math.h>
#include <string.h>

#include "builtin.h"

#define LASSERT(args, cond, err_msg) if (!(cond)) {lval_del(args); return lval_err(err_msg);}

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

lval *builtin_cons(lval *a) {
    LASSERT(a, a->count > 1, "Function `cons` passed too few arguments");
    LASSERT(a, a->count < 3, "Function `cons` passed too many arguments");
    LASSERT(a, a->cell[1]->type == LVAL_QEXPR, "Function `cons` passed incorrect types");

    lval *q = lval_pop(a, 1);
    lval *v = lval_take(a, 0);

    q->count++;
    q->cell = realloc(q->cell, sizeof(lval *) * q->count);
    memmove(&q->cell[1], &q->cell[0], sizeof(lval *) * (q->count - 1));
    q->cell[0] = v;

    return q;
}

lval *builtin_eval(lval *a) {
    LASSERT(a, a->count == 1, "Function `eval` passed too many arguments");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function `eval` passed incorrect types!");

    lval *x = lval_take(a, 0);
    x->type = LVAL_SEXPR;
    return lval_eval(x);
}

lval *builtin_join(lval *a) {
    for (int i=0; i < a->count; i++) {
        LASSERT(a, a->cell[i]->type == LVAL_QEXPR, "Function `join` passed incorrect types!");
    }

    lval *x = lval_pop(a, 0);
    while (a->count) {
        x = lval_join(x, lval_pop(a, 0));
    }
    lval_del(a);
    return x;
}

lval *builtin_head(lval *a) {
    LASSERT(a, a->count == 1, "Function `head` passed too many arguments");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function `head` passed incorrect types!");
    LASSERT(a, a->cell[0]->count != 0, "Function `head` passed {}!");

    lval *v =lval_take(a, 0);
    while (v->count > 1) {
        lval_del(lval_pop(v, 1));
    }
    return v;
}

lval *builtin_init(lval *a) {
    LASSERT(a, a->count == 1, "Function `init` passed too many arguments");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function `init` passed incorrect types!");
    LASSERT(a, a->cell[0]->count != 0, "Function `init` passed {}!");

    lval *v = lval_take(a, 0);
    lval_del(lval_pop(v, v->count - 1));
    return v;
}

lval *builtin_len(lval *a) {
    LASSERT(a, a->count == 1, "Function `len` passed too many arguments");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function `len` passed incorrect types!");
    lval *v = lval_take(a, 0);
    lval *result = lval_num(v->count);
    lval_del(v);
    return result;
}

lval *builtin_list(lval *a) {
    a->type = LVAL_QEXPR;
    return a;
}

lval *builtin_tail(lval *a) {
    LASSERT(a, a->count == 1, "Function `tail` passed too many arguments");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function `tail` passed incorrect types!");
    LASSERT(a, a->cell[0]->count != 0, "Function `tail` passed {}!");

    lval *v =lval_take(a, 0);
    lval_del(lval_pop(v, 0));
    return v;
}

lval *builtin(lval *a, char *func) {
    if (strcmp("cons", func) == 0) { return builtin_cons(a); }
    if (strcmp("eval", func) == 0) { return builtin_eval(a); }
    if (strcmp("head", func) == 0) { return builtin_head(a); }
    if (strcmp("init", func) == 0) { return builtin_init(a); }
    if (strcmp("join", func) == 0) { return builtin_join(a); }
    if (strcmp("len", func) == 0) { return builtin_len(a); }
    if (strcmp("list", func) == 0) { return builtin_list(a); }
    if (strcmp("tail", func) == 0) { return builtin_tail(a); }
    else {
        return builtin_op(a, func);
    }
}