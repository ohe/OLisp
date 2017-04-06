#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>

#include "mpc.h"
#include "lval.h"
#include "builtin.h"


int main(int argc, char **argv) {

    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Symbol = mpc_new("symbol");
    mpc_parser_t *Expr = mpc_new("expr");
    mpc_parser_t *Sexpr = mpc_new("sexpr");
    mpc_parser_t *Qexpr = mpc_new("qexpr");
    mpc_parser_t *OLisp = mpc_new("olisp");

    mpc_result_t result;

    mpca_lang(MPCA_LANG_DEFAULT,
              "                                                                                              \
               number   : /-?[0-9]+/;                                                                        \
               symbol   : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;                                                 \
               expr     : <number> | <symbol> | <sexpr> | <qexpr>;                                           \
               sexpr    : '(' <expr>* ')';                                                                   \
               qexpr    : '{' <expr>* '}';                                                                   \
               olisp    : /^/ <expr>* /$/;                                                                   \
              ",
              Number, Symbol, Expr, Sexpr, Qexpr, OLisp);


    puts("OLisp v0");
    puts("Press Ctrl+c to exit.");

    lenv *e = lenv_new();
    register_builtins(e);

    while (1) {
        char *input = readline("()lisp> ");
        add_history(input);

        if (mpc_parse("<stdin>", input, OLisp, &result)) {
            /* mpc_ast_print(result.output); */
            lval *x = lval_eval(e, lval_read(result.output));
            lval_println(x);
            lval_del(x);
            mpc_ast_delete(result.output);
        } else {
            mpc_err_print(result.error);
            mpc_err_delete(result.error);
        }
        free(input);
    }

    lenv_del(e);
    mpc_cleanup(6, Number, Symbol, Expr, Sexpr, Qexpr, OLisp);
    return 0;
}