#include "lenv.h"

lenv *lenv_new(void) {
    lenv *e = malloc(sizeof(lenv));
    e->par = NULL;
    e->count = 0;
    e->syms = NULL;
    e->vals = NULL;
    return e;
}

lenv *lenv_copy(lenv *e) {

    lenv *ne = malloc(sizeof(lenv));
    ne->par = e->par;
    ne->count = e->count;
    ne->syms = malloc(sizeof(char *) * ne->count);
    ne->vals = malloc(sizeof(lval *) * ne->count);

    for (int i = 0; i < ne->count; i++) {
        ne->vals[i] = lval_copy(e->vals[i]);
        ne->syms[i] = malloc(strlen(e->syms[i]) + 1);
        strcpy(ne->syms[i], e->syms[i]);
    }
    return ne;
}

void lenv_del(lenv *e) {
    for (int i = 0; i < e->count; i++) {
        free(e->syms[i]);
        lval_del(e->vals[i]);
    }
    free(e->syms);
    free(e->vals);
    free(e);
}

lval* lenv_get(lenv *e, lval *k) {
    for (int i = 0; i < e->count; i ++) {
        if (strcmp(e->syms[i], k->sym) == 0) {
            return lval_copy(e->vals[i]);
        }
    }

    if (e->par) {
        return lenv_get(e->par, k);
    }

    return lval_err("Unbound symbol `%s`!", k->sym);
}

void lenv_def(lenv *e, lval *k, lval *v) {
    while (e->par) {e = e->par;}
    lenv_put(e, k, v);
}

void lenv_put(lenv *e, lval *k, lval *v) {
    for (int i = 0; i < e->count; i ++) {
        if (strcmp(e->syms[i], k->sym) == 0) {
            lval_del(e->vals[i]);
            e->vals[i] = lval_copy(v);
            return;
        }
    }

    e->count++;
    e->vals = realloc(e->vals, sizeof(lval *) * e->count);
    e->syms = realloc(e->syms, sizeof(char *) * e->count);

    e->vals[e->count - 1] = lval_copy(v);
    e->syms[e->count - 1] = malloc(strlen(k->sym) + 1);
    strcpy(e->syms[e->count - 1], k->sym);
}