#pragma once
#include <stdbool.h>
#include <stddef.h>

#include "adapter.h"
#include "symtable.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "ifj_errors.h"

typedef enum SemanticErrorCode {
    SEM_OK = 0,
    SEM_ERR_UNDECLARED_IDENT = ERR_SEM_UNDEF,
    SEM_ERR_REDECLARATION = ERR_SEM_UNDEF,
    SEM_ERR_TYPE_MISMATCH = ERR_SEM_TYPES,
    SEM_ERR_MISSING_RETURN = ERR_SEM_OTHER,
    SEM_ERR_DUPLICATE_FUNCTION = ERR_SEM_OTHER,
    SEM_ERR_UNDEFINED_FUNCTION = ERR_SEM_UNDEF
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
