#pragma once
#include "adapter.h"

#ifdef __cplusplus
extern "C" {
#endif

// A tiny, in-memory mock AST for build validation only

typedef struct DemoNode DemoNode;
struct DemoNode {
    AnalyzerNodeKind kind;
    const char* name;     // for identifiers/function/var names
    Type type;            // for type nodes
    DemoNode* first_child;
    DemoNode* next_sibling;
};

void demo_link(DemoNode* parent, DemoNode* child);

extern const AstAdapter DEMO_ADAPTER;

#ifdef __cplusplus
}
#endif
