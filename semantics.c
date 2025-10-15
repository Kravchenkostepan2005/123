#include <string.h>
#include "semantics.h"

const char *ifj_sem_error_name(IfjSemErrorCode code) {
    switch (code) {
        case IFJ_SEM_OK: return "SEM_OK";
        case IFJ_SEM_UNDEFINED: return "SEM_UNDEFINED";
        case IFJ_SEM_REDEFINITION: return "SEM_REDEFINITION";
        case IFJ_SEM_BAD_CALL_OR_BUILTIN_PARAM: return "SEM_BAD_CALL_OR_BUILTIN_PARAM";
        case IFJ_SEM_TYPE_COMPAT: return "SEM_TYPE_COMPAT";
        case IFJ_SEM_OTHER: return "SEM_OTHER";
        default: return "SEM_UNKNOWN";
    }
}

const char *ifj_sem_prolog_rule_reason_name(IfjSemPrologRuleReason reason) {
    switch (reason) {
        case IFJ_SEM_PR_OK: return "PR_OK";
        case IFJ_SEM_PR_PROLOG_COUNT: return "PR_PROLOG_COUNT";
        case IFJ_SEM_PR_PROLOG_NOT_FIRST: return "PR_PROLOG_NOT_FIRST";
        case IFJ_SEM_PR_CLASS_COUNT: return "PR_CLASS_COUNT";
        case IFJ_SEM_PR_CLASS_NAME_NOT_PROGRAM: return "PR_CLASS_NAME_NOT_PROGRAM";
        default: return "PR_UNKNOWN";
    }
}

const char *ifj_sem_runtime_error_name(IfjSemRuntimeErrorCode code) {
    switch (code) {
        case IFJ_RT_BAD_BUILTIN_PARAM: return "RT_BAD_BUILTIN_PARAM";
        case IFJ_RT_TYPE_COMPAT: return "RT_TYPE_COMPAT";
        default: return "RT_UNKNOWN";
    }
}

static int count_direct_children_with(Node *node, Nonterminal_type nt) {
    int count = 0;
    if (!node) return 0;
    for (Node *ch = node->left_child; ch != NULL; ch = ch->right_sibling) {
        if (ch->nonterminal == nt) count++;
    }
    return count;
}

static Node* first_direct_child(Node *node, Nonterminal_type nt) {
    if (!node) return NULL;
    for (Node *ch = node->left_child; ch != NULL; ch = ch->right_sibling) {
        if (ch->nonterminal == nt) return ch;
    }
    return NULL;
}

static int is_first_child(Node *parent, Node *child) {
    return parent && parent->left_child == child;
}

static int token_equals(const Token *tok, const char *s) {
    if (!tok || !tok->lexeme || !s) return 0;
    return strcmp(tok->lexeme, s) == 0;
}

SemError check_prolog_and_rules_ex(Node *program_root, FILE *errout, IfjSemPrologRuleReason *reason_out) {
    if (reason_out) *reason_out = IFJ_SEM_PR_OK;
    if (!program_root) {
        if (errout) fprintf(errout, "SEMANTICS: null PROGRAM root\n");
        if (reason_out) *reason_out = IFJ_SEM_PR_PROLOG_COUNT;
        return IFJ_SEM_OTHER;
    }
    if (program_root->nonterminal != PROGRAM) {
        if (errout) fprintf(errout, "SEMANTICS: invalid PROGRAM root\n");
        if (reason_out) *reason_out = IFJ_SEM_PR_PROLOG_COUNT;
        return IFJ_SEM_OTHER;
    }

    // Exactly one PROLOG under PROGRAM
    int prolog_count = count_direct_children_with(program_root, PROLOG);
    if (prolog_count != 1) {
        if (errout) fprintf(errout, "SEMANTICS: Expected exactly one PROLOG, got %d\n", prolog_count);
        if (reason_out) *reason_out = IFJ_SEM_PR_PROLOG_COUNT;
        return IFJ_SEM_OTHER;
    }
    Node *prolog = first_direct_child(program_root, PROLOG);
    if (!is_first_child(program_root, prolog)) {
        if (errout) fprintf(errout, "SEMANTICS: PROLOG must be the first child of PROGRAM\n");
        if (reason_out) *reason_out = IFJ_SEM_PR_PROLOG_NOT_FIRST;
        return IFJ_SEM_OTHER;
    }

    // Exactly one CLASS_NT under PROGRAM
    int class_count = count_direct_children_with(program_root, CLASS_NT);
    if (class_count != 1) {
        if (errout) fprintf(errout, "SEMANTICS: Expected exactly one class skeleton (CLASS_NT), got %d\n", class_count);
        if (reason_out) *reason_out = IFJ_SEM_PR_CLASS_COUNT;
        return IFJ_SEM_OTHER;
    }
    Node *klass = first_direct_child(program_root, CLASS_NT);

    // Optional: ensure class name token is "Program" if present at the CLASS_NT node
    if (klass && klass->token && klass->token->lexeme) {
        if (!token_equals(klass->token, "Program")) {
            if (errout) fprintf(errout, "SEMANTICS: Top-level class must be named 'Program'\n");
            if (reason_out) *reason_out = IFJ_SEM_PR_CLASS_NAME_NOT_PROGRAM;
            return IFJ_SEM_OTHER;
        }
    }

    return IFJ_SEM_OK;
}

SemError check_prolog_and_rules(Node *program_root, FILE *errout) {
    return check_prolog_and_rules_ex(program_root, errout, NULL);
}
