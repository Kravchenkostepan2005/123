#pragma once
#include <stdbool.h>
#include <stddef.h>

#include "adapter.h"
#include "symtable.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum SemanticErrorCode {
    SEM_OK = 0,
    SEM_ERR_UNDECLARED_IDENT = 1,
    SEM_ERR_REDECLARATION = 2,
    SEM_ERR_TYPE_MISMATCH = 3,
    SEM_ERR_MISSING_RETURN = 4,
    SEM_ERR_DUPLICATE_FUNCTION = 5,
    SEM_ERR_UNDEFINED_FUNCTION = 6
} SemanticErrorCode;

typedef struct AnalyzerConfig {
    bool require_main;
    Type main_return_type;
    int main_param_count;
} AnalyzerConfig;

int analyze_program(const void* root, const AstAdapter* adapter, const AnalyzerConfig* config, char* errbuf, size_t errbuf_size);

#ifdef __cplusplus
}
#endif
