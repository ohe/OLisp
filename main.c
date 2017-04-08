#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>

#include "mpc.h"
#include "lval.h"
#include "builtin.h"


int main(int argc, char **argv) {

    Comment = mpc_new("comment");
    Number = mpc_new("number");
    String = mpc_new("string");
    Symbol = mpc_new("symbol");
    Expr = mpc_new("expr");
    Sexpr = mpc_new("sexpr");
    Qexpr = mpc_new("qexpr");
    OLisp = mpc_new("olisp");

    mpc_result_t result;

    /*string : /\"(\\\\.|[^\"])*\"/ ; \
     * For managing doubles
     *       double   : /-?[0-9]+\\.[0-9]+/ ;                   \
     */

    mpca_lang(MPCA_LANG_DEFAULT,
              "                                                                                              \
               number   : /-?[0-9]+/;                                                                        \
               symbol   : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;                                                 \
               string   : /\"(\\\\.|[^\"])*\"/ ;                                                             \
               comment  : /;[^\\r\\n]*/  ;                                                                   \
               expr     : <number> | <symbol> | <string> | <comment> | <sexpr> | <qexpr>;                    \
               sexpr    : '(' <expr>* ')';                                                                   \
               qexpr    : '{' <expr>* '}';                                                                   \
               olisp    : /^/ <expr>* /$/;                                                                   \
              ",
              Number, Symbol, String, Comment, Expr, Sexpr, Qexpr, OLisp);

    puts("OLisp v0");
    puts("Press Ctrl+c to exit.");

    lenv *e = lenv_new();
    register_builtins(e);

    if (argc >= 2) {
        for (int i = 1; i <argc; i++) {
            lval *args = lval_add(lval_sexpr(), lval_str(argv[i]));
            lval *x = builtin_load(e, args);
            if (x->type == LVAL_ERR) lval_println(x);
            lval_del(x);
        }
    }

    while (1) {
        char *input = readline("()lisp> ");
        add_history(input);

        if (mpc_parse("<stdin>", input, OLisp, &result)) {
            //mpc_ast_print(result.output);
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
    mpc_cleanup(8, Number, Symbol, String, Comment, Expr, Sexpr, Qexpr, OLisp);
    return 0;
}