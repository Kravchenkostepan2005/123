#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "semantic.h"
#include "ast.h"
#include "parser.h"

// Adapter to parser
extern int parse_topDown(Node *root);

static Node *parse_input(void) {
    Node *root = (Node *)calloc(1, sizeof(Node));
    if (!root) return NULL;
    int rc = parse_topDown(root);
    if (rc != 0) {
        free(root);
        return NULL;
    }
    return root;
}

// ---------- Test runner ----------

typedef struct {
    const char *name;
    const char *code;
    int expected_result;
} test_case_t;

static void run_test_from_string(const char *test_name, const char *code, int expected) {
    printf("Testing: %s... ", test_name);

    // Create temporary file
    FILE *tmp = fopen("test_input.tmp", "w");
    if (!tmp) {
        printf("FAIL: cannot create temp file\n");
        return;
    }
    fputs(code, tmp);
    fclose(tmp);

    // Redirect stdin to read from temp file
    freopen("test_input.tmp", "r", stdin);

    // Parse and analyze
    Node *ast = parse_input();
    if (!ast) {
        printf("FAIL: parsing failed\n");
        remove("test_input.tmp");
        return;
    }

    int result = semantic_analyze(ast);

    // Cleanup
    free(ast);
    remove("test_input.tmp");

    if (result == expected) {
        printf("PASS (got %d)\n", result);
    } else {
        printf("FAIL: expected %d, got %d\n", expected, result);
    }
}

static void run_all_tests(void) {
    printf("=== Running Semantic Analyzer Tests ===\n\n");

    // Test cases defined inside function (not as global constants)
    test_case_t test_cases[] = {
        {"Basic program",
         "import \"ifj25\" for Ifj\n"
         "class Program {\n"
         "    static main() {\n"
         "        var x\n"
         "        x = 10\n"
         "        Ifj.write(x)\n"
         "    }\n"
         "}\n", 0},

        {"Global variables",
         "import \"ifj25\" for Ifj\n"
         "class Program {\n"
         "    static main() {\n"
         "        __global = \"hello\"\n"
         "        var local\n"
         "        local = __global\n"
         "        Ifj.write(local)\n"
         "    }\n"
         "}\n", 0},

        {"No main function",
         "import \"ifj25\" for Ifj\n"
         "class Program {\n"
         "    static foo() {\n"
         "        Ifj.write(\"no main\")\n"
         "    }\n"
         "}\n", SEMANTIC_UNDEFINED_ERROR},

        {"Main with parameters",
         "import \"ifj25\" for Ifj\n"
         "class Program {\n"
         "    static main(x) {\n"
         "        Ifj.write(x)\n"
         "    }\n"
         "}\n", SEMANTIC_PARAM_COUNT_ERROR},

        {"Type error",
         "import \"ifj25\" for Ifj\n"
         "class Program {\n"
         "    static main() {\n"
         "        var x\n"
         "        x = \"hello\" - 5\n"
         "        Ifj.write(x)\n"
         "    }\n"
         "}\n", SEMANTIC_TYPE_ERROR},

        {NULL, NULL, 0} // Sentinel
    };

    int passed = 0;
    int total = 0;

    for (test_case_t *test = test_cases; test->name != NULL; test++) {
        run_test_from_string(test->name, test->code, test->expected_result);
        if (test->expected_result == 0) {
            passed++;
        }
        total++;
    }

    printf("\n=== Test Results ===\n");
    printf("Passed: %d/%d\n", passed, total);

    if (passed == total) {
        printf("All tests PASSED! ✓\n");
    } else {
        printf("Some tests FAILED! ✗\n");
    }
}

// ---------- File-based testing ----------

static int run_file_test(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return -1;
    }

    // Redirect stdin to read from file
    freopen(filename, "r", stdin);

    Node *ast = parse_input();
    if (!ast) {
        fclose(file);
        fprintf(stderr, "Error: Parsing failed for %s\n", filename);
        return -1;
    }

    int result = semantic_analyze(ast);

    // Cleanup
    free(ast);
    fclose(file);

    return result;
}

// ---------- Main ----------

int main(int argc, char *argv[]) {
    if (argc == 1) {
        // No arguments - run built-in tests
        run_all_tests();
        return 0;
    }
    else if (argc == 2) {
        // One argument - test specific file
        const char *filename = argv[1];
        int result = run_file_test(filename);

        if (result == -1) {
            return 1; // File or parsing error
        }

        printf("%d\n", result);
        return 0;
    }
    else {
        fprintf(stderr, "Usage: %s [test_file]\n", argv[0]);
        fprintf(stderr, "  If no argument: run built-in test suite\n");
        fprintf(stderr, "  If test_file provided: analyze that file and print result code\n");
        return 1;
    }
}
