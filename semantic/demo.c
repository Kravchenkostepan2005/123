#include <stdio.h>
#include <string.h>

#include "analyzer.h"
#include "demo_adapter.h"

int main(void) {
    // Build a tiny AST:
    // program
    //  └─ func main(params=(), ret=int) { block }
    //       └─ return 0

    DemoNode program = { .kind = AN_NODE_PROGRAM };

    DemoNode params = { .kind = AN_NODE_PARAMLIST };
    DemoNode rettype = { .kind = AN_NODE_TYPE, .type = make_type(TYPE_INT) };
    DemoNode body = { .kind = AN_NODE_BLOCK };
    DemoNode ret_expr = { .kind = AN_NODE_EXPR, .type = make_type(TYPE_INT) };
    DemoNode ret_stmt = { .kind = AN_NODE_RETURN };
    demo_link(&ret_stmt, &ret_expr);

    DemoNode mainf = { .kind = AN_NODE_FUNCDECL, .name = "main" };
    demo_link(&mainf, &params);
    demo_link(&mainf, &rettype);
    demo_link(&mainf, &body);
    demo_link(&body, &ret_stmt);

    demo_link(&program, &mainf);

    AnalyzerConfig cfg = { .require_main = true, .main_return_type = make_type(TYPE_INT), .main_param_count = 0 };

    char err[512];
    int rc = analyze_program(&program, &DEMO_ADAPTER, &cfg, err, sizeof(err));

    if (rc == SEM_OK) {
        printf("Semantic analysis succeeded.\n");
        return 0;
    }
    printf("Semantic analysis failed (code %d):\n%s", rc, err);
    return 1;
}
