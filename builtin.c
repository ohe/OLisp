#include <math.h>
#include <string.h>

#include "builtin.h"

#define LASSERT(args, cond, fmt, ...) if (!(cond)) {lval_del(args); return lval_err(fmt, ##__VA_ARGS__);}

char *ltype_name(int t) {
    switch(t) {
        case LVAL_FUN: return "Function";
        case LVAL_NUM: return "Number";
        case LVAL_ERR: return "Error";
        case LVAL_SYM: return "Symbol";
        case LVAL_SEXPR: return "S-expr";
        case LVAL_QEXPR: return "Q-expr";
        default: return "Unknown";
    }
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

lval *builtin_op(lenv *e, lval *v, char *op) {

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

lval *builtin_cons(lenv *e, lval *a) {
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

lval *builtin_def(lenv *e, lval *a) {
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function `def` passed incorrect type!");
    lval *syms = a->cell[0];

    /* Ensure all elements of first list are symbols */
    for (int i = 0; i < syms->count; i ++) {
        LASSERT(a, syms->cell[i]->type == LVAL_SYM, "Function `def` cannot define non-symbol!");
    }

    /* Check correct number of symbols and values */
    LASSERT(a, syms->count == a->count - 1, "Function `def` connot define incorrect number of values to symbols");

    for (int i = 0; i < syms->count; i ++) {
        lenv_put(e, syms->cell[i], a->cell[i + 1]);
    }

    lval_del(a);
    return lval_sexpr();
}

lval *builtin_eval(lenv *e, lval *a) {
    LASSERT(a, a->count == 1, "Function `eval` passed too many arguments");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function `eval` passed incorrect types!");

    lval *x = lval_take(a, 0);
    x->type = LVAL_SEXPR;
    return lval_eval(e, x);
}

lval *builtin_join(lenv *e, lval *a) {
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

lval *builtin_head(lenv *e, lval *a) {
    LASSERT(a, a->count == 1, "Function `head` passed too many arguments. Got %i, Expected %i", a->count, 1);
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
            "Function `head` passed incorrect type for argument 0. Got %s, expected %s",
            ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));
    LASSERT(a, a->cell[0]->count != 0, "Function `head` passed {}!");

    lval *v =lval_take(a, 0);
    while (v->count > 1) {
        lval_del(lval_pop(v, 1));
    }
    return v;
}

lval *builtin_init(lenv *e, lval *a) {
    LASSERT(a, a->count == 1, "Function `init` passed too many arguments");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function `init` passed incorrect types!");
    LASSERT(a, a->cell[0]->count != 0, "Function `init` passed {}!");

    lval *v = lval_take(a, 0);
    lval_del(lval_pop(v, v->count - 1));
    return v;
}

lval *builtin_len(lenv *e, lval *a) {
    LASSERT(a, a->count == 1, "Function `len` passed too many arguments");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function `len` passed incorrect types!");
    lval *v = lval_take(a, 0);
    lval *result = lval_num(v->count);
    lval_del(v);
    return result;
}

lval *builtin_list(lenv *e, lval *a) {
    a->type = LVAL_QEXPR;
    return a;
}

lval *builtin_tail(lenv *e, lval *a) {
    LASSERT(a, a->count == 1, "Function `tail` passed too many arguments");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function `tail` passed incorrect types!");
    LASSERT(a, a->cell[0]->count != 0, "Function `tail` passed {}!");

    lval *v =lval_take(a, 0);
    lval_del(lval_pop(v, 0));
    return v;
}

lval *builtin_add(lenv *e, lval *a) {
    return builtin_op(e, a, "+");
}
lval *builtin_sub(lenv *e, lval *a) {
    return builtin_op(e, a, "-");
}
lval *builtin_mul(lenv *e, lval *a) {
    return builtin_op(e, a, "*");
}
lval *builtin_div(lenv *e, lval *a) {
    return builtin_op(e, a, "/");
}
lval *builtin_pow(lenv *e, lval *a) {
    return builtin_op(e, a, "");
}
lval *builtin_mod(lenv *e, lval *a) {
    return builtin_op(e, a, "%");
}
lval *builtin_min(lenv *e, lval *a) {
    return builtin_op(e, a, "min");
}
lval *builtin_max(lenv *e, lval *a) {
    return builtin_op(e, a, "max");
}

void lenv_add_builtin(lenv *e, char *name, lbuiltin func) {
    lval *k = lval_sym(name);
    lval *v = lval_func(func);
    lenv_put(e, k, v);
    lval_del(k);
    lval_del(v);
}

void register_builtins(lenv *e) {
    lenv_add_builtin(e, "cons", builtin_cons);
    lenv_add_builtin(e, "def", builtin_def);
    lenv_add_builtin(e, "eval", builtin_eval);
    lenv_add_builtin(e, "head", builtin_head);
    lenv_add_builtin(e, "init", builtin_init);
    lenv_add_builtin(e, "join", builtin_join);
    lenv_add_builtin(e, "len", builtin_len);
    lenv_add_builtin(e, "list", builtin_list);
    lenv_add_builtin(e, "tail", builtin_tail);
    lenv_add_builtin(e, "+", builtin_add);
    lenv_add_builtin(e, "-", builtin_sub);
    lenv_add_builtin(e, "*", builtin_mul);
    lenv_add_builtin(e, "/", builtin_div);
    lenv_add_builtin(e, "^", builtin_pow);
    lenv_add_builtin(e, "%", builtin_mod);
    lenv_add_builtin(e, "min", builtin_min);
    lenv_add_builtin(e, "max", builtin_max);
}