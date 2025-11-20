#include "parser.h"

int error_handler(Token *token){
    if(token->type == ERROR){
        return token->data.integer;
    }
    return 0;
}

Token *skip_EOL_WHITESPACE(Token *token_buffer,int is_EOL_needed){
    Token *tmp = NULL;
    if(token_buffer == NULL){
        tmp = get_token();
        token_buffer = tmp;
    }
    else{
        tmp =token_buffer;
    }
    if(tmp->type != WHITESPACE && tmp->type != EOL){
        if(is_EOL_needed){
            return NULL;
        }
        return tmp;
    }
    int check_EOL = false;
    while(tmp->type == WHITESPACE || tmp->type == EOL){
        if(tmp->type == EOL){
            check_EOL = true;
        }
        tmp = get_token();
        token_buffer = tmp;
    }
    if(check_EOL == false && is_EOL_needed == true){
        return NULL;
    }
    return tmp;
}
int prolog_rule(Node *root){
    if(insert_child(root,PROLOG,NULL) == COMPILER_ERROR){
        return COMPILER_ERROR;
    }
    Token *token_buffer = NULL;
    Token *tmp = NULL;
    Token_type check_types[] = {EOL_EPSILON,KEYWORD,WHITESPACE,STRING_TOKEN,WHITESPACE,KEYWORD,WHITESPACE,KEYWORD,EOL,-1};
    Keyword_type check_keywords[] = {IMPORT,FOR,IFJ};
    int f = 0;
    for(int i = 0; (int)check_types[i] != -1;i++){
        tmp = read_buffer(&token_buffer);
        if(tmp == NULL){
            tmp = get_token();
            int error = error_handler(tmp);
            if(error != 0)
                return error;
        }
        if(check_types[i] == tmp->type){
            if(check_types[i] == STRING_TOKEN){
                root->left_child->token = tmp;
            }
            if(check_keywords[f] == tmp->data.integer && check_types[i] == KEYWORD){
                f++;
            }
            else if(check_types[i] == KEYWORD){
                return SYNTACTICAL_ERROR;
            }
        }
        else{
            if((check_types[i] == EOL || check_types[i] == EOL_EPSILON) && tmp->type == WHITESPACE){
                i--;
                continue;
            }
            else if( check_types[i] == EOL_EPSILON && tmp->type == EOL){
                i--;
                continue;
            }
            else if( check_types[i] == EOL_EPSILON){
                write_buffer(&token_buffer,tmp);
                continue;
            }
            return SYNTACTICAL_ERROR;
        }
    }
    return 0;
}

void write_buffer(Token **token_buffer, Token *address){
    *token_buffer = address;
}

Token *read_buffer(Token **token_buffer){
    Token *tmp = *token_buffer;
    *token_buffer = NULL;
    return tmp;
}
int class_rule(Node *root){
    Token *token_buffer = NULL;
    Token *tmp = NULL;
    tmp = skip_EOL_WHITESPACE(tmp,false);
    int error = error_handler(tmp);
    if (error != 0)
        return error;
    write_buffer(&token_buffer,tmp);
    if(insert_child(root,CLASS_NT,NULL) == COMPILER_ERROR){
        return COMPILER_ERROR;
    }
    Token_type terminals[] = {KEYWORD,WHITESPACE,IDENTIFIER,WHITESPACE,LEFT_CURLY_BRACKET,EOL,NaT,RIGHT_CURLY_BRACKET,EOL_EPSILON,EOF_TOKEN,-1};
    Nonterminal_type nonterminals[] = { FUNCLIST };
    Keyword_type keywords[] = {CLASS};
    return check_correct_rule(root->right_child,terminals,nonterminals,keywords,&token_buffer);
}

int func_list_rule(Node *root,Token **token_buffer){
    //determining rule
    Token *tmp = read_buffer(token_buffer);
    if(tmp == NULL){
        tmp = get_token();
        int error = error_handler(tmp);
        if(error != 0)
            return error;
    }
    write_buffer(token_buffer,tmp);
    if(error_handler(tmp) != 0){
        return tmp->data.integer;
    }
    if(tmp->type != KEYWORD){
        if((Keyword_type)tmp->data.integer != STATIC){
            return 0;
        }
    }
    Token_type terminals[] = {NaT,NaT,-1};
    Nonterminal_type nonterminals[] = { FUNCDECL,FUNCLIST };
    return check_correct_rule(root,terminals,nonterminals,NULL,token_buffer);
}

int func_decl_rule(Node *root, Token **token_buffer){
    Token_type terminals[] = {KEYWORD,WHITESPACE,IDENTIFIER,WHITESPACE_EPSILON,NaT,WHITESPACE_EPSILON,NaT,EOL,-1};
    Nonterminal_type nonterminals[] = { FUNCHEAD,BLOCK };
    Keyword_type keywords[] = {STATIC};
    return check_correct_rule(root,terminals,nonterminals,keywords,token_buffer);
}

int func_head_rule(Node *root,Token **token_buffer){
    Token *tmp = read_buffer(token_buffer);
    if(tmp == NULL){
        tmp = get_token();
        int error = error_handler(tmp);
        if(error != 0)
            return error;
    }
    write_buffer(token_buffer,tmp);
    if(tmp->type == LEFT_BRACKET){
        Token_type terminals[] = {LEFT_BRACKET,EOL_EPSILON,NaT,WHITESPACE_EPSILON,RIGHT_BRACKET,-1};
        Nonterminal_type nonterminals[] = { PARAMLISTOPT };
        return check_correct_rule(root,terminals,nonterminals,NULL,token_buffer);
    }
    if(tmp->type == EQUAL){
        Token_type terminals[] = {EQUAL,WHITESPACE_EPSILON,LEFT_BRACKET,WHITESPACE_EPSILON,IDENTIFIER,WHITESPACE_EPSILON,RIGHT_BRACKET,-1};
        return check_correct_rule(root,terminals,NULL,NULL,token_buffer);
    }
    return 0;
}

int param_list_opt_rule(Node *root, Token **token_buffer){
    Token *tmp = read_buffer(token_buffer);
    if(tmp == NULL){
        tmp = get_token();
        int error = error_handler(tmp);
        if(error != 0)
            return error;
    }
    write_buffer(token_buffer,tmp);
    if(tmp->type == IDENTIFIER){
        Token_type terminals[] = { NaT,-1 };
        Nonterminal_type nonterminals[] = { PARAMLIST };
        return check_correct_rule(root,terminals,nonterminals,NULL,token_buffer);
    }
    return 0;
}

int param_list_rule(Node *root, Token **token_buffer){
    Token_type terminals[] = { IDENTIFIER,WHITESPACE_EPSILON,NaT,-1 };
    Nonterminal_type nonterminals[] = { PARAMTAIL };
    return check_correct_rule(root,terminals,nonterminals,NULL,token_buffer);
}

int param_tail_rule(Node *root, Token **token_buffer){
    Token *tmp = read_buffer(token_buffer);
    if(tmp == NULL){
        tmp = get_token();
        int error = error_handler(tmp);
        if(error != 0)
            return error;
    }
    write_buffer(token_buffer,tmp);
    if(tmp->type == COMMA){
        Token_type terminals[] = { COMMA,WHITESPACE_EPSILON,IDENTIFIER,WHITESPACE_EPSILON,NaT,-1 };
        Nonterminal_type nonterminals[] = { PARAMTAIL };
        return check_correct_rule(root,terminals,nonterminals,NULL,token_buffer);
    }
    return 0;
}

int block_rule(Node *root,Token **token_buffer){
    Token_type terminals[] = { LEFT_CURLY_BRACKET,EOL,NaT,RIGHT_CURLY_BRACKET,-1 };
    Nonterminal_type nonterminals[] = { SEQUENCEBLOCK };
    return check_correct_rule(root,terminals,nonterminals,NULL,token_buffer);
}

int sequence_block_rule(Node *root,Token **token_buffer){
    Token *tmp = read_buffer(token_buffer);
    if(tmp == NULL){
        tmp = get_token();
        int error = error_handler(tmp);
        if(error != 0)
            return error;
    }
    write_buffer(token_buffer,tmp);
    switch(tmp->type){
        case KEYWORD:
            switch(tmp->data.integer){
                case VAR:
                case IF:
                case WHILE:
                case RETURN:
                case IFJ:
                    break;
                default:
                    return 0;
            }
            break;
        case IDENTIFIER:
        case GLOBAL_IDENTIFIER:
            break;
        default:
            return 0;
    }
    Token_type terminals[] = { NaT,EOL,NaT,-1 };
    Nonterminal_type nonterminals[] = { SEQUENCE,SEQUENCEBLOCK };
    return check_correct_rule(root,terminals,nonterminals,NULL,token_buffer);
}

int sequence_rule(Node *root,Token **token_buffer){
    Token *tmp = read_buffer(token_buffer);
    if(tmp == NULL){
        tmp = get_token();
        int error = error_handler(tmp);
        if(error != 0)
            return error;
    }
    write_buffer(token_buffer,tmp);
    if(tmp->type == KEYWORD){
        if(tmp->data.integer == VAR){
            Token_type terminals[] = { KEYWORD,WHITESPACE,IDENTIFIER,-1 };
            Keyword_type keywords[] = { VAR };
            return check_correct_rule(root,terminals,NULL,keywords,token_buffer);
        }
        else if(tmp->data.integer == IF){
            Token_type terminals[] = { KEYWORD,WHITESPACE_EPSILON,LEFT_BRACKET,WHITESPACE_EPSILON,NaT,WHITESPACE_EPSILON,RIGHT_BRACKET,WHITESPACE_EPSILON,NaT,WHITESPACE_EPSILON,KEYWORD,WHITESPACE_EPSILON,NaT,-1 };
            Nonterminal_type nonterminals[] = { EXPR,BLOCK,BLOCK };
            Keyword_type keywords[] = { IF,ELSE };
            return check_correct_rule(root,terminals,nonterminals,keywords,token_buffer);
        }
        else if(tmp->data.integer == WHILE){
            Token_type terminals[] = { KEYWORD,WHITESPACE_EPSILON,LEFT_BRACKET,WHITESPACE_EPSILON,NaT,WHITESPACE_EPSILON,RIGHT_BRACKET,WHITESPACE_EPSILON,NaT,-1 };
            Keyword_type keywords[] = { WHILE };
            Nonterminal_type nonterminals[] = { EXPR,BLOCK };
            return check_correct_rule(root,terminals,nonterminals,keywords,token_buffer);
        }
        else if(tmp->data.integer == RETURN){
            Token_type terminals[] = { KEYWORD,WHITESPACE,NaT,-1 };
            Keyword_type keywords[] = { RETURN };
            Nonterminal_type nonterminals[] = { EXPR };
            return check_correct_rule(root,terminals,nonterminals,keywords,token_buffer);
        }
        else if((Keyword_type)tmp->data.integer == IFJ ){
            Token_type terminals[] = { KEYWORD,WHITESPACE_EPSILON,DOT,EOL_EPSILON,IDENTIFIER,WHITESPACE_EPSILON,LEFT_BRACKET,WHITESPACE_EPSILON,NaT,WHITESPACE_EPSILON,RIGHT_BRACKET,-1 };
            Keyword_type keywords[] = { IFJ };
            Nonterminal_type nonterminals[] = { ARGLISTOPT };
            return check_correct_rule(root,terminals,nonterminals,keywords,token_buffer);
        }
        else{
                return SYNTACTICAL_ERROR;
        }
    }
    else if(tmp->type == IDENTIFIER){
        Token_type terminals[] = { IDENTIFIER,WHITESPACE_EPSILON,EQUAL,WHITESPACE_EPSILON,NaT,-1 };
        Nonterminal_type nonterminals[] = { EXPR };
        return check_correct_rule(root,terminals,nonterminals,NULL,token_buffer);
    }
    else if(tmp->type == GLOBAL_IDENTIFIER){
        Token_type terminals[] = { GLOBAL_IDENTIFIER,WHITESPACE_EPSILON,EQUAL,WHITESPACE_EPSILON,NaT,-1 };
        Nonterminal_type nonterminals[] = { EXPR };
        return check_correct_rule(root,terminals,nonterminals,NULL,token_buffer);
    }
    else{
        return SYNTACTICAL_ERROR;
    }
}

int arg_list_opt_rule(Node *root,Token **token_buffer){
    Token *tmp = read_buffer(token_buffer);
    if(tmp == NULL){
        tmp = get_token();
        int error = error_handler(tmp);
        if(error != 0)
            return error;
    }
    write_buffer(token_buffer,tmp);
    if(tmp->type != INT && tmp->type != FLOAT && tmp->type != IDENTIFIER && tmp->type != STRING_TOKEN && tmp->type != GLOBAL_IDENTIFIER && tmp->type != KEYWORD){
        return 0;
    }
    if(tmp->type == KEYWORD){
        if((Keyword_type)tmp->data.integer != NULL_LOWERCASE){
            return 0;
        }
    }
    Token_type terminals[] = { NaT,-1 };
    Nonterminal_type nonterminals[] = { ARGLIST };
    return check_correct_rule(root,terminals,nonterminals,NULL,token_buffer);
}

int arg_list_rule(Node *root,Token **token_buffer){
    Token *tmp = read_buffer(token_buffer);
    if(tmp == NULL){
        tmp = get_token();
        int error = error_handler(tmp);
        if(error != 0)
            return error;
    }
    write_buffer(token_buffer,tmp);
    if(tmp->type != INT && tmp->type != FLOAT && tmp->type != IDENTIFIER && tmp->type != STRING_TOKEN && tmp->type != GLOBAL_IDENTIFIER && tmp->type != KEYWORD){
        return SYNTACTICAL_ERROR;
    }
    if(tmp->type == KEYWORD){
        if((Keyword_type)tmp->data.integer != NULL_LOWERCASE){
            return SYNTACTICAL_ERROR;
        }
    }
    Token_type terminals[] = { NaT,WHITESPACE_EPSILON,NaT,-1 };
    Nonterminal_type nonterminals[] = { TERM,ARGTAIL };
    return check_correct_rule(root,terminals,nonterminals,NULL,token_buffer);
}

int arg_tail_rule(Node *root,Token **token_buffer){
    Token *tmp = read_buffer(token_buffer);
    if(tmp == NULL){
        tmp = get_token();
        int error = error_handler(tmp);
        if(error != 0)
            return error;
    }
    write_buffer(token_buffer,tmp);
    if(tmp->type != COMMA)
        return 0;
    Token_type terminals[] = { COMMA,WHITESPACE_EPSILON,NaT,WHITESPACE_EPSILON,NaT,-1 };
    Nonterminal_type nonterminals[] = { TERM,ARGTAIL };
    return check_correct_rule(root,terminals,nonterminals,NULL,token_buffer);
}

int term_rule(Node *root,Token **token_buffer){
    Token *tmp = read_buffer(token_buffer);
    if(tmp == NULL){
        tmp = get_token();
        int error = error_handler(tmp);
        if(error != 0)
            return error;
    }
    write_buffer(token_buffer,tmp);
    if(tmp->type == INT || tmp->type == FLOAT || tmp->type == STRING_TOKEN || tmp->type == IDENTIFIER || tmp->type == GLOBAL_IDENTIFIER){
        Token_type terminals[] = { tmp->type,-1 };
        return check_correct_rule(root,terminals,NULL,NULL,token_buffer);
    }
    else if(tmp->type == KEYWORD){
        if((Keyword_type)tmp->data.integer != NULL_LOWERCASE){
            return SYNTACTICAL_ERROR;
        }
        Token_type terminals[] = { KEYWORD,-1 };
        Keyword_type keywords[] = { NULL_LOWERCASE };
        return check_correct_rule(root,terminals,NULL,keywords,token_buffer);
    }
    return SYNTACTICAL_ERROR;
}

int expr_rule(Node *root, Token **token_buffer){
    Token_type terminals[] = { NaT,-1 };
    Nonterminal_type nonterminals[] = { LOGICOR };
    return check_correct_rule(root,terminals,nonterminals,NULL,token_buffer);
}
int logic_or_rule(Node *root, Token **token_buffer){
    Token_type terminals[] = { NaT,WHITESPACE_EPSILON,NaT,-1 };
    Nonterminal_type nonterminals[] = { LOGICAND,LOGICORTAIL };
    return check_correct_rule(root,terminals,nonterminals,NULL,token_buffer);
}
/* EXTENSION, Scanner doesnt support boolean OPERATORS
int logic_or_tail_rule(Node *root, Token **token_buffer){
    Token *tmp = read_buffer(token_buffer);
    if(tmp == NULL){
        tmp = get_token();
        int error = error_handler(tmp);
        if(error != 0)
            return error;
    }
    write_buffer(token_buffer,tmp);
    if(tmp->type != OR){
        return 0;
    }
    Token_type terminals[] = { OR,WHITESPACE_EPSILON,NaT,WHITESPACE_EPSILON,NaT,-1 };
    Nonterminal_type nonterminals[] = { LOGICAND,LOGICORTAIL };
    return check_correct_rule(root,terminals,nonterminals,NULL,token_buffer);
}
*/
int logic_and_rule(Node *root, Token **token_buffer){
    Token_type terminals[] = { NaT,WHITESPACE_EPSILON,NaT,-1 };
    Nonterminal_type nonterminals[] = { EQUALITY,LOGICANDTAIL };
    return check_correct_rule(root,terminals,nonterminals,NULL,token_buffer);
}
/* EXTENSION, Scanner doesnt support boolean OPERATORS
int logic_or_tail_rule(Node *root, Token **token_buffer){
    Token *tmp = read_buffer(token_buffer);
    if(tmp == NULL){
        tmp = get_token();
        int error = error_handler(tmp);
        if(error != 0)
            return error;
    }
    write_buffer(token_buffer,tmp);
    if(tmp->type != AND){
        return 0;
    }
    Token_type terminals[] = { AND,WHITESPACE_EPSILON,NaT,WHITESPACE_EPSILON,NaT,-1 };
    Nonterminal_type nonterminals[] = { EQUALITY,LOGICANDTAIL };
    return check_correct_rule(root,terminals,nonterminals,NULL,token_buffer);
}
*/
int equality_rule(Node *root, Token **token_buffer){
    Token_type terminals[] = { NaT,WHITESPACE_EPSILON,NaT,-1 };
    Nonterminal_type nonterminals[] = { RELATIONAL,EQUALITYTAIL };
    return check_correct_rule(root,terminals,nonterminals,NULL,token_buffer);
}
int equality_tail_rule(Node *root, Token **token_buffer){
    Token *tmp = read_buffer(token_buffer);
    if(tmp == NULL){
        tmp = get_token();
        int error = error_handler(tmp);
        if(error != 0)
            return error;
    }
    write_buffer(token_buffer,tmp);
    if(tmp->type == COMPARISON || tmp->type == NOT_EQUAL){
        Token_type terminals[] = { tmp->type,WHITESPACE_EPSILON,NaT,WHITESPACE_EPSILON,NaT,-1 };
        Nonterminal_type nonterminals[] = { RELATIONAL,EQUALITYTAIL };
        return check_correct_rule(root,terminals,nonterminals,NULL,token_buffer);
    }
    return 0;
}
int relational_rule(Node *root, Token **token_buffer){
    Token_type terminals[] = { NaT,WHITESPACE_EPSILON,NaT,-1 };
    Nonterminal_type nonterminals[] = { ADDITIVE,RELATIONALTAIL };
    return check_correct_rule(root,terminals,nonterminals,NULL,token_buffer);
}
int relational_tail_rule(Node *root, Token **token_buffer){
    Token *tmp = read_buffer(token_buffer);
    if(tmp == NULL){
        tmp = get_token();
        int error = error_handler(tmp);
        if(error != 0)
            return error;
    }
    write_buffer(token_buffer,tmp);
    if(tmp->type == LESSER_THAN || tmp->type == LESSER_OR_EQUAL || tmp->type == GREATER_THAN || tmp->type == GREATER_OR_EQUAL){
        Token_type terminals[] = { tmp->type,WHITESPACE_EPSILON,NaT,WHITESPACE_EPSILON,NaT,-1 };
        Nonterminal_type nonterminals[] = { ADDITIVE,RELATIONALTAIL };
        return check_correct_rule(root,terminals,nonterminals,NULL,token_buffer);
    }
    return 0;
}
int additive_rule(Node *root, Token **token_buffer){
    Token_type terminals[] = { NaT,WHITESPACE_EPSILON,NaT,-1 };
    Nonterminal_type nonterminals[] = { MULTIPLICATIVE,ADDITIVETAIL };
    return check_correct_rule(root,terminals,nonterminals,NULL,token_buffer);
}
int additive_tail_rule(Node *root, Token **token_buffer){
    Token *tmp = read_buffer(token_buffer);
    if(tmp == NULL){
        tmp = get_token();
        int error = error_handler(tmp);
        if(error != 0)
            return error;
    }
    write_buffer(token_buffer,tmp);
    if(tmp->type == ADDITION || tmp->type == SUBTRACTION){
        Token_type terminals[] = { tmp->type,WHITESPACE_EPSILON,NaT,WHITESPACE_EPSILON,NaT,-1 };
        Nonterminal_type nonterminals[] = { MULTIPLICATIVE,ADDITIVETAIL };
        return check_correct_rule(root,terminals,nonterminals,NULL,token_buffer);
    }
    return 0;
}
int multiplicative_rule(Node *root, Token **token_buffer){
    Token_type terminals[] = { NaT,WHITESPACE_EPSILON,NaT,-1 };
    Nonterminal_type nonterminals[] = { UNARY,MULTIPLICATIVETAIL };
    return check_correct_rule(root,terminals,nonterminals,NULL,token_buffer);
}
int multiplicative_tail_rule(Node *root, Token **token_buffer){
    Token *tmp = read_buffer(token_buffer);
    if(tmp == NULL){
        tmp = get_token();
        int error = error_handler(tmp);
        if(error != 0)
            return error;
    }
    write_buffer(token_buffer,tmp);
    if(tmp->type == MULTIPLICATION || tmp->type == DIVISION){
        Token_type terminals[] = { tmp->type,WHITESPACE_EPSILON,NaT,WHITESPACE_EPSILON,NaT,-1 };
        Nonterminal_type nonterminals[] = { UNARY,MULTIPLICATIVETAIL };
        return check_correct_rule(root,terminals,nonterminals,NULL,token_buffer);
    }
    return 0;
}
int unary_rule(Node *root, Token **token_buffer){
    Token *tmp = read_buffer(token_buffer);
    if(tmp == NULL){
        tmp = get_token();
        int error = error_handler(tmp);
        if(error != 0)
            return error;
    }
    write_buffer(token_buffer,tmp);
    if(tmp->type == ADDITION || tmp->type == SUBTRACTION /* support for !*/){
        Token_type terminals[] = { tmp->type,WHITESPACE_EPSILON,NaT,-1 };
        Nonterminal_type nonterminals[] = { UNARY };
        return check_correct_rule(root,terminals,nonterminals,NULL,token_buffer);
    }
    else if(tmp->type == INT || tmp->type == FLOAT || tmp->type == IDENTIFIER || tmp->type == STRING_TOKEN || tmp->type == GLOBAL_IDENTIFIER || tmp->type == LEFT_BRACKET || tmp->type == KEYWORD || tmp->type == ML_STRING_TOKEN){
        if(tmp->type == KEYWORD){
            if((Keyword_type)tmp->data.integer != IFJ &&(Keyword_type)tmp->data.integer != NULL_LOWERCASE ){
                return SYNTACTICAL_ERROR;
            }
        }
        Token_type terminals[] = { NaT,-1 };
        Nonterminal_type nonterminals[] = { PRIMARY };
        return check_correct_rule(root,terminals,nonterminals,NULL,token_buffer);
    }
    return SYNTACTICAL_ERROR;
}
int primary_rule(Node *root, Token **token_buffer){
    Token *tmp = read_buffer(token_buffer);
    if(tmp == NULL){
        tmp = get_token();
        int error = error_handler(tmp);
        if(error != 0)
            return error;
    }
    write_buffer(token_buffer,tmp);
    if(tmp->type == KEYWORD){
        if((Keyword_type)tmp->data.integer == NULL_LOWERCASE){
            Token_type terminals[] = { KEYWORD,-1 };
            Keyword_type keywords[] = { NULL_LOWERCASE };
            return check_correct_rule(root,terminals,NULL,keywords,token_buffer);
        }
        else if((Keyword_type)tmp->data.integer == IFJ ){
            Token_type terminals[] = { KEYWORD,WHITESPACE_EPSILON,DOT,EOL_EPSILON,IDENTIFIER,WHITESPACE_EPSILON,LEFT_BRACKET,WHITESPACE_EPSILON,NaT,WHITESPACE_EPSILON,RIGHT_BRACKET,-1 };
            Keyword_type keywords[] = { IFJ };
            Nonterminal_type nonterminals[] = { ARGLISTOPT };
            return check_correct_rule(root,terminals,nonterminals,keywords,token_buffer);
        }
        return SYNTACTICAL_ERROR;
    }
    else if(tmp->type == STRING_TOKEN || tmp->type == GLOBAL_IDENTIFIER || tmp->type == FLOAT || tmp->type == INT || tmp->type == ML_STRING_TOKEN){
        Token_type terminals[] = { tmp->type,-1 };
        return check_correct_rule(root,terminals,NULL,NULL,token_buffer);
    }
    else if(tmp->type == LEFT_BRACKET){
        Token_type terminals[] = { LEFT_BRACKET,WHITESPACE_EPSILON,NaT,WHITESPACE_EPSILON,RIGHT_BRACKET,-1 };
        Nonterminal_type nonterminals[] = { EXPR };
        return check_correct_rule(root,terminals,nonterminals,NULL,token_buffer);
    }
    else if(tmp->type == IDENTIFIER){
        Token_type terminals[] = { IDENTIFIER,WHITESPACE_EPSILON,NaT,-1 };
        Nonterminal_type nonterminals[] = { PRIMARYIDENTTAIL };
        return check_correct_rule(root,terminals,nonterminals,NULL,token_buffer);
    }
    return SYNTACTICAL_ERROR;
}

int primary_ident_tail(Node *root,Token **token_buffer){
    Token *tmp = read_buffer(token_buffer);
    if(tmp == NULL){
        tmp = get_token();
        int error = error_handler(tmp);
        if(error != 0)
            return error;
    }
    write_buffer(token_buffer,tmp);
    if(tmp->type == LEFT_BRACKET){
        Token_type terminals[] = { LEFT_BRACKET,WHITESPACE_EPSILON,NaT,WHITESPACE_EPSILON,RIGHT_BRACKET,-1 };
        Nonterminal_type nonterminals[] = { ARGLISTOPT };
        return check_correct_rule(root,terminals,nonterminals,NULL,token_buffer);
    }
    return 0;
}

int call_rule(Node *root,Nonterminal_type nonterminal,Token **token_buffer){
    switch(nonterminal){
        case FUNCLIST:
            return func_list_rule(root,token_buffer);
        case FUNCDECL:
            return func_decl_rule(root,token_buffer);
        case FUNCHEAD:
            return func_head_rule(root,token_buffer);
        case PARAMLISTOPT:
            return param_list_opt_rule(root,token_buffer);
        case PARAMTAIL:
            return param_tail_rule(root,token_buffer);
        case PARAMLIST:
            return param_list_rule(root,token_buffer);
        case BLOCK:
            return block_rule(root,token_buffer);
        case SEQUENCEBLOCK:
            return sequence_block_rule(root,token_buffer);
        case SEQUENCE:
            return sequence_rule(root,token_buffer);
        case ARGLISTOPT:
            return arg_list_opt_rule(root,token_buffer);
        case ARGLIST:
            return arg_list_rule(root,token_buffer);
        case ARGTAIL:
            return arg_tail_rule(root,token_buffer);
        case TERM:
            return term_rule(root,token_buffer);
        case EXPR:
            return expr_rule(root,token_buffer);
        case LOGICOR:
            return logic_or_rule(root,token_buffer);
        case LOGICAND:
            return logic_and_rule(root,token_buffer);
        case LOGICORTAIL:
            return 0;
        case LOGICANDTAIL:
            return 0;
        case EQUALITY:
            return equality_rule(root,token_buffer);
        case EQUALITYTAIL:
            return equality_tail_rule(root,token_buffer);
        case RELATIONAL:
            return relational_rule(root,token_buffer);
        case RELATIONALTAIL:
            return relational_tail_rule(root,token_buffer);
        case ADDITIVE:
            return additive_rule(root,token_buffer);
        case ADDITIVETAIL:
            return additive_tail_rule(root,token_buffer);
        case MULTIPLICATIVE:
            return multiplicative_rule(root,token_buffer);
        case MULTIPLICATIVETAIL:
            return multiplicative_tail_rule(root,token_buffer);
        case UNARY:
            return unary_rule(root,token_buffer);
        case PRIMARY:
            return primary_rule(root,token_buffer);
        case PRIMARYIDENTTAIL:
            return primary_ident_tail(root,token_buffer);
        default:
            return SYNTACTICAL_ERROR;
            break;
    }
}
int check_correct_rule(Node *root,Token_type *terminals, Nonterminal_type *nonterminals,Keyword_type *check_keywords,Token **token_buffer){
    int nonterminal_count = 0;
    int keyword_count = 0;
    Token *tmp = NULL;
    for(int i = 0; (int)terminals[i] != -1; i++){
        if(terminals[i] != NaT){
            tmp = read_buffer(token_buffer);
            if(tmp == NULL)
                tmp = get_token();
            int error = error_handler(tmp);
            if(error != 0){
                return error;
            }
            if(terminals[i] == WHITESPACE_EPSILON && tmp->type == WHITESPACE){
                i--;
                continue;
            }
            else if(terminals[i] == WHITESPACE_EPSILON){
                i++;
            }
            if(terminals[i] == EOL_EPSILON && (tmp->type == WHITESPACE || tmp->type == EOL)){
                i--;
                continue;
            }
            else if(terminals[i] == WHITESPACE_EPSILON || terminals[i] == EOL_EPSILON){
                i++;
            }
            if(terminals[i] == NaT){
                write_buffer(token_buffer,tmp);
                i--;
                continue;
            }
            if(tmp->type != EOL && tmp->type != WHITESPACE)
                insert_child(root,NaN,tmp);
            if(terminals[i] == EOL){
                tmp = skip_EOL_WHITESPACE(tmp,true);
                write_buffer(token_buffer,tmp);
                if(tmp == NULL){
                    return SYNTACTICAL_ERROR;
                }
                int error = error_handler(tmp);
                if(error != 0){
                    return error;
                }
                if((int)terminals[i] == -1){
                    return 0;
                }
                continue;
            }
            if(terminals[i] != tmp->type){
                return SYNTACTICAL_ERROR;
            }

            if(check_keywords == NULL){
                continue;
            }
            if(check_keywords[keyword_count] == tmp->data.integer && terminals[i] == KEYWORD){
                keyword_count++;
            }
            else if(terminals[i] == KEYWORD){
                return SYNTACTICAL_ERROR;
            }
        }
        else{
            if(nonterminals == NULL){
                return SYNTACTICAL_ERROR;
            }
            insert_child(root,nonterminals[nonterminal_count],NULL);
            int ret_code = call_rule(root->right_child,nonterminals[nonterminal_count],token_buffer);
            if(ret_code != 0)
                return ret_code;
            nonterminal_count++;
        }
    }
    return 0;
}


int parse_topDown(Node *root){
    root->nonterminal = PROGRAM;
    int error = prolog_rule(root);
    if(error != 0){
        return error;
    }
    error = class_rule(root);
    if(error != 0){
        return error;
    }

    return 0;
}
