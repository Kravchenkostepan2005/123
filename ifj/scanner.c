#include "scanner.h"
                                                // TODO: most states, check if nulling was successful in token_reset and adjusting uses in switch
                                                // tomas - symbols, jan - identifiers etc.


const char *keyword_table[] = {
    "Ifj", "import", "for", "class", "if", "else", "is", "null", "return", "var", "while", "static", "true", "false", "Num", "String", "Null", /*"Ifj", */ NULL
};
const char *inbuilt_functions_table[] = {
    "Ifj.read_str", "Ifj.read_num", "Ifj.write", "Ifj.floor", "Ifj.str", "Ifj.length", "Ifj.substring", "Ifj.strcmp", "Ifj.ord", "Ifj.chr", NULL
};
                                                // tomas - symbols, numbers, jan - identifiers etc.
                                                // jan - strings, global identifiers

//Function expects char pointer to be NULL or already assigned
//Input is current token and character to be assigned
//Output is either 0 or 99 (COMPILER_ERROR)
int token_assign_char(Token *current_token, char data_to_add){
    if(current_token->data.characters == NULL){
        char *data = malloc(2*sizeof(char));
        if(data == NULL){
            current_token->type = EMPTY;                                // To avoid freeing NULL when a token_reset is called in response to this
            return COMPILER_ERROR;
        }
        data[0] = data_to_add;
        data[1] = '\0';
        current_token->data.characters = data;
    } else {
        int number_of_elements = 0;
        while(current_token->data.characters[number_of_elements] != '\0'){
            number_of_elements++;
        }
        char *data = realloc(current_token->data.characters,sizeof(char)*(number_of_elements + 2));
        if(data == NULL){
            current_token->type = EMPTY;                                // To avoid freeing NULL when a token_reset is called in response to this
            return COMPILER_ERROR;
        }
        current_token->data.characters = data;
        current_token->data.characters[number_of_elements] = data_to_add;
        current_token->data.characters[number_of_elements + 1] = '\0';
    }
    return 0;
}

int token_remove_last_char(Token *current_token){                                // returns 0 when successful, -1 when used on an empty or unallocated string, and 99 when it fails

    if(current_token->data.characters == NULL){
        return -1;
    }
    int number_of_elements = 0;
    while(current_token->data.characters[number_of_elements] != '\0'){
            number_of_elements++;
    }
    if(number_of_elements == 0){
        return -1;
    } else {
        current_token->data.characters[number_of_elements-1] = '\0';
        char *data = realloc(current_token->data.characters,sizeof(char)*(number_of_elements));
        if(data == NULL){
            free(current_token->data.characters);
            current_token->data.characters = NULL;
            current_token->type = EMPTY;                                // To avoid freeing NULL when a token_reset is called in response to this
            return COMPILER_ERROR;
        }
        current_token->data.characters = data;
    }

    return 0;
}

void set_error(Token *token){
    token->type = ERROR;
    token->data.integer = LEXICAL_ERROR;
}

void save_int(Token *token,int isHexadecimal){
    char *tmp = token->data.characters;
    if(isHexadecimal == 0){
        token->data.integer = atoi(tmp);
    }
    else{
        token->data.integer = (int)strtoul(tmp,NULL,16);
    }
    free(tmp);
}
void save_float(Token *token){
    char *tmp = token->data.characters;
    token->data.floating_point = atof(tmp);
    free(tmp);
    if(token->data.floating_point == 0){
        set_error(token);
        token->data.integer = COMPILER_ERROR;
    }
}

int token_reset(Token *token){                                    // also use when setting token->data. TODO check if nulling was successful and adjusting uses in switch
    if (token->type == IDENTIFIER || token->type == GLOBAL_IDENTIFIER || token->type == STRING_TOKEN || token->type == ML_STRING_TOKEN || token->type == UNDECLARED) {
        free(token->data.characters);
    }
    token->type = EMPTY;
    token->data.characters = NULL;
    return 0;
}

void state_machine(Token *token) {
    char hex_buffer[3] = {0};
    int counter_hex = 0;
    int counter_quote = 0;
    int counter_space = 0;
    int state = START;
    int next_state = START;

    while (true) {
        int c = fgetc(stdin);
        state = next_state;
        switch (state){
            case START:
                    //Should put into switch later when doing cleanup
                if (c == EOF) {                            // Handles EOF
                    token_reset(token);
                    token->type = EOF_TOKEN;
                    token->data.characters = NULL;
                    return;
                }
                if (c == '*') {                            // Handles multiplication signs
                    token->type = MULTIPLICATION;
                    next_state = CHECK_COMMENT;
                }
                if (c == '/'){
                    token->type = DIVISION;
                    next_state = CHECK_COMMENT;
                }
                if (c == '>'){
                    token->type = GREATER_THAN;
                    next_state = EXTENSION;
                }
                if (c == '<'){
                    token->type = LESSER_THAN;
                    next_state = EXTENSION;
                }
                if (c == '='){
                    token->type = EQUAL;
                    next_state = EXTENSION;
                }
                if (c == '!'){
                    token->type = UNDECLARED;
                    next_state = EXTENSION;
                }
                if (c == '+'){
                    token->type = ADDITION;
                    return;
                }
                if (c == '-'){
                    token->type = SUBTRACTION;
                    return;
                }
                if (c == '{'){
                    token->type = LEFT_CURLY_BRACKET;
                    return;
                }
                if(c == '}'){
                    token->type = RIGHT_CURLY_BRACKET;
                    return;
                }
                if(c == '('){
                    token->type = LEFT_BRACKET;
                    return;
                }
                if(c == ')'){
                    token->type = RIGHT_BRACKET;
                    return;
                }
                if (c == '.'){
                    token->type = DOT;
                    return;
                }
                if (c == ','){
                    token->type = COMMA;
                    return;
                }
                if(c >= '0' && c <= '9'){
                    token->type = INT;
                    if(token_assign_char(token,c) == COMPILER_ERROR){
                        set_error(token);
                        token->data.integer = COMPILER_ERROR;
                        return;
                    }
                    if(c == '0'){
                        token->type = UNDECLARED;
                    }
                    next_state = INTEGER_CHECK;
                }

                if (isalpha(c)){
                    token->type = UNDECLARED;
                    if(token_assign_char(token,c) == COMPILER_ERROR){
                        set_error(token);
                        token->data.integer = COMPILER_ERROR;
                        return;
                    }
                    next_state = IDENTIFIER_OR_KEYWORD;
                }

                if (c == '"') {
                    next_state = QUOTES_STATE;
                    break;
                }

                if (c == '\n') {                        // handles newlines
                    token->type = EOL;
                    return;
                }
                if(isspace(c)){
                    next_state = SPACE_SKIP;
                    token->type = WHITESPACE;
                }

                if (c == '_') {
                    next_state = UNDERSCORE_STATE;
                    token->type = UNDECLARED;
                    if(token_assign_char(token,c) == COMPILER_ERROR){
                        set_error(token);
                        token->data.integer = COMPILER_ERROR;
                        return;
                    }
                }

                break;

            case CHECK_COMMENT:
                if(token->type == DIVISION){
                    if(c == '*'){
                        token->type = ML_COMMENT_START;
                        next_state = END_COMMENT;
                        break;
                    }
                    if(c == '/'){
                        token->type = COMMENT;
                        next_state = END_COMMENT;
                        break;
                    }
                } else if (token->type == ML_COMMENT_END){
                    if(c == '/'){
                        token->type = WHITESPACE;
                        return;
                    } else if (c == EOF){
                        token_reset(token);
                        ungetc(c,stdin);
                        set_error(token);
                        return;
                    }
                    token->type = ML_COMMENT_START;
                    next_state = END_COMMENT;
                    break;
                }
                ungetc(c,stdin);
                return;

            case END_COMMENT:
                if(token->type == COMMENT){
                    if(c == '\n'){
                        token->type = EOL;
                        return;
                    } else if (c == EOF){
                        token->type = EOF_TOKEN;
                        return;
                    }
                } else if (token->type == ML_COMMENT_START){
                    if(c == '*'){
                        token->type = ML_COMMENT_END;
                        next_state = CHECK_COMMENT;
                        break;
                    } else if (c == EOF){
                        token_reset(token);
                        ungetc(c,stdin);
                        set_error(token);
                        return;
                    }
                }
                break;

            case EXTENSION:
                if (c == '='){
                    if(token->type == LESSER_THAN){
                        token->type = LESSER_OR_EQUAL;
                    } else if (token->type == GREATER_THAN){
                        token->type = GREATER_OR_EQUAL;
                    } else if (token->type == EQUAL){
                        token->type = COMPARISON;
                    } else if (token->type == UNDECLARED){
                        token->type = NOT_EQUAL;
                    }
                    return;
                }
                if(token->type == UNDECLARED){
                    token_reset(token);
                    ungetc(c,stdin);
                    set_error(token);
                    return;
                }
                ungetc(c,stdin);
                return;

            case INTEGER_CHECK:
                if(token->type == UNDECLARED){
                    if(c == 'x'){
                        next_state = INTEGER_HEXADECIMAL;
                        break;
                    }
                    token->type = INT;
                }
                if(c == '.'){
                    next_state = FLOAT_CHECK_DECIMAL;
                    token->type = FLOAT;
                    if(token_assign_char(token,c) == COMPILER_ERROR){
                        token_reset(token);
                        set_error(token);
                        token->data.integer = COMPILER_ERROR;
                        return;
                    }
                    break;
                } else if (c == 'E' || c == 'e'){
                    next_state = FLOAT_CHECK_EXPONENT;
                    if(token_assign_char(token,c) == COMPILER_ERROR){
                        token_reset(token);
                        set_error(token);
                        token->data.integer = COMPILER_ERROR;
                        return;
                    }
                    break;
                }
                if(c >= '0' && c <= '9'){
                    if(token_assign_char(token,c) == COMPILER_ERROR){
                        token_reset(token);
                        set_error(token);
                        token->data.integer = COMPILER_ERROR;
                        return;
                    }
                    break;
                }
                ungetc(c,stdin);
                save_int(token,0);
                return;
            case INTEGER_HEXADECIMAL:
                if(token->type == UNDECLARED){
                    if((c < 'a' || c > 'f') && (c < 'A' || c > 'F') && (c < '0' || c > '9')){
                        token_reset(token);
                        set_error(token);
                        return;
                    }
                    token->type = INT;
                }
                if((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || (c >= '0' && c <= '9')){
                    if(token_assign_char(token,c) == COMPILER_ERROR){
                        token_reset(token);
                        set_error(token);
                        token->data.integer = COMPILER_ERROR;
                        return;
                    }
                    break;
                }
                ungetc(c,stdin);
                save_int(token,1);
                return;
            case FLOAT_CHECK_DECIMAL:
                if(c >= '0' && c <= '9'){
                    if(token_assign_char(token,c) == COMPILER_ERROR){
                        token_reset(token);
                        set_error(token);
                        token->data.integer = COMPILER_ERROR;
                        return;
                    }
                    break;
                } else if (c == 'E' || c == 'e'){
                    next_state = FLOAT_CHECK_EXPONENT;
                    token->type = INT;
                    if(token_assign_char(token,c) == COMPILER_ERROR){
                        token_reset(token);
                        set_error(token);
                        token->data.integer = COMPILER_ERROR;
                        return;
                    }
                    break;
                }
                ungetc(c,stdin);
                save_float(token);
                return;
            case FLOAT_CHECK_EXPONENT:
                if(token->type == INT){
                    if(c == '+' || c == '-'){
                        token->type = UNDECLARED;
                    } else if (c >= '0' && c <= '9'){
                        token->type = FLOAT;
                    } else {
                        token_reset(token);
                        set_error(token);
                        return;
                    }
                    if(token_assign_char(token,c) == COMPILER_ERROR){
                        token_reset(token);
                        set_error(token);
                        token->data.integer = COMPILER_ERROR;
                        return;
                    }
                    break;
                }
                if(token->type == UNDECLARED){
                    if(c < '0' || c > '9'){
                        token_reset(token);
                        set_error(token);
                        return;
                    }
                }
                if(c >= '0' && c <= '9'){
                    if(token_assign_char(token,c) == COMPILER_ERROR){
                        token_reset(token);
                        set_error(token);
                        token->data.integer = COMPILER_ERROR;
                        return;
                    }
                    break;
                }
                ungetc(c,stdin);
                save_float(token);
                return;
            case SPACE_SKIP:
                if(!isspace(c) || c == '\n'){
                    ungetc(c,stdin);
                    return;
                }
                break;
            case IDENTIFIER_OR_KEYWORD:
                if (isalnum(c) || c == '_'){
                    if(token_assign_char(token,c) == COMPILER_ERROR){
                        token_reset(token);
                        set_error(token);
                        token->data.integer = COMPILER_ERROR;
                        return;
                    }
                    next_state = IDENTIFIER_OR_KEYWORD;            // technically redundant, but as insurance.
                } else {
                    ungetc(c, stdin);
                    for (int i = 0; keyword_table[i] != NULL; i++){
                        if (strcmp(token->data.characters, keyword_table[i]) == 0){
                            token_reset(token);
                            token->type = KEYWORD;
                            token->data.integer = i;
                            return;
                        }
                    }
                    token->type = IDENTIFIER;
                    return;
                }

                break;

            case QUOTES_STATE:
                if (c == '"'){
                    next_state = TWO_QUOTES_STATE;
                } else if (c == '\\'){
                    next_state = ESCAPE_SEQUENCE;
                } else if (c == '\n'){
                    token_reset(token);
                    set_error(token);
                    return;
                } else {
                    token->type = STRING_TOKEN;
                    next_state = STRING_STATE;
                    if(token_assign_char(token,c) == COMPILER_ERROR){
                        token_reset(token);
                        set_error(token);
                        token->data.integer = COMPILER_ERROR;
                        return;
                    }
                }
                break;

            case TWO_QUOTES_STATE:
                if (c == '"'){
                    next_state = ML_STRING_WHITESPACE_SKIP_AFTER_QUOTES;
                    counter_space = 0;
                } else {
                    token_reset(token);
                    token->type = STRING_TOKEN;
                    token->data.characters = malloc(1);
                    if (token->data.characters == NULL) {                // Not using token reset because we already know the pointer is NULL
                        set_error(token);
                        token->data.integer = COMPILER_ERROR;
                        return;
                    }
                    token->data.characters[0] = '\0';
                    ungetc(c, stdin);
                    return;
                }
                break;

            case STRING_STATE:
                if (c == '"'){
                    token->type = STRING_TOKEN;
                    return;
                } else if (c == '\\'){
                    next_state = ESCAPE_SEQUENCE;
                } else if (c == '\n' || c == EOF){
                    token_reset(token);
                    set_error(token);
                    return;
                } else {
                    next_state = STRING_STATE;
                    token->type = STRING_TOKEN;
                    if(token_assign_char(token,c) == COMPILER_ERROR){
                        token_reset(token);
                        set_error(token);
                        token->data.integer = COMPILER_ERROR;
                        return;
                    }
                }
                break;

            case ESCAPE_SEQUENCE:
                switch (c){
                    case '\"':
                        next_state = STRING_STATE;
                        token->type = STRING_TOKEN;
                        if(token_assign_char(token,'\"') == COMPILER_ERROR){
                            token_reset(token);
                            set_error(token);
                            token->data.integer = COMPILER_ERROR;
                            return;
                        }
                        break;
                    case '\\':
                        next_state = STRING_STATE;
                        token->type = STRING_TOKEN;
                        if(token_assign_char(token,'\\') == COMPILER_ERROR){
                            token_reset(token);
                            set_error(token);
                            token->data.integer = COMPILER_ERROR;
                            return;
                        }
                        break;
                    case 'n':
                        next_state = STRING_STATE;
                        token->type = STRING_TOKEN;
                        if(token_assign_char(token,'\n') == COMPILER_ERROR){
                            token_reset(token);
                            set_error(token);
                            token->data.integer = COMPILER_ERROR;
                            return;
                        }
                        break;
                    case 'r':
                        next_state = STRING_STATE;
                        token->type = STRING_TOKEN;
                        if(token_assign_char(token,'\r') == COMPILER_ERROR){
                            token_reset(token);
                            set_error(token);
                            token->data.integer = COMPILER_ERROR;
                            return;
                        }
                        break;
                    case 't':
                        next_state = STRING_STATE;
                        token->type = STRING_TOKEN;
                        if(token_assign_char(token,'\t') == COMPILER_ERROR){
                            token_reset(token);
                            set_error(token);
                            token->data.integer = COMPILER_ERROR;
                            return;
                        }
                        break;

                    case 'x':
                        counter_hex = 0;
                        next_state = ESCAPE_HEXADECIMAL;
                        break;
                    default:
                        token_reset(token);
                        set_error(token);
                        return;
                }
                break;

            case ESCAPE_HEXADECIMAL:
                if ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || (c >= '0' && c <= '9')){
                    hex_buffer[counter_hex++] = c;
                    next_state = ESCAPE_HEXADECIMAL;
                } else {
                    token_reset(token);
                    set_error(token);
                    return;
                }

                if (counter_hex == 2) {
                    counter_hex = 0;
                    int hex_to_char = (int)strtoul(hex_buffer,NULL,16);
                    next_state = STRING_STATE;
                    token->type = STRING_TOKEN;
                    if(token_assign_char(token, hex_to_char) == COMPILER_ERROR){
                        token_reset(token);
                        set_error(token);
                        token->data.integer = COMPILER_ERROR;
                        return;
                    }
                    hex_buffer[0] = 0;                // resetting hex_buffer for next time
                    hex_buffer[1] = 0;
                    hex_buffer[2] = '\0';
                    break;
                }

                if (counter_hex > 2) {                    // means that the '\0' in hex_buffer was overwritten
                    token_reset(token);
                    set_error(token);
                    token->data.integer = COMPILER_ERROR;
                    return;
                }
                break;

            case ML_STRING_STATE:
                if (c == '"'){
                    next_state = ML_STRING_QUOTE;
                    counter_quote = 1;
                } else if (c == '\n'){
                    next_state = ML_STRING_WHITESPACE_SKIP;
                    counter_space = 0;
                    if(token_assign_char(token,c) == COMPILER_ERROR){
                        token_reset(token);
                        set_error(token);
                        token->data.integer = COMPILER_ERROR;
                        return;
                    }
                } else if (c == EOF){
                    token_reset(token);
                    set_error(token);
                    return;
                } else {
                    next_state = ML_STRING_STATE;
                    token->type = ML_STRING_TOKEN;
                    if(token_assign_char(token,c) == COMPILER_ERROR){
                        token_reset(token);
                        set_error(token);
                        token->data.integer = COMPILER_ERROR;
                        return;
                    }
                }
                break;

            case ML_STRING_QUOTE:
                if (c == '\"') {
                    counter_quote++;
                } else {
                    for (; counter_quote > 0; counter_quote--){                        // puts one " for every " in a row it counted.
                        token->type = ML_STRING_TOKEN;
                        if(token_assign_char(token,'\"') == COMPILER_ERROR){
                            token_reset(token);
                            set_error(token);
                            token->data.integer = COMPILER_ERROR;
                            return;
                        }
                    }
                    ungetc(c, stdin);
                    next_state = ML_STRING_STATE;
                }
                if (counter_quote >= 3){
                    token->type = ML_STRING_TOKEN;
                    return;
                }
                break;

            case ML_STRING_WHITESPACE_SKIP_AFTER_QUOTES:
                if (c == '\n') {
                    for (; counter_space > 0; counter_space--){                        // removes trailing whitespace
                        token_remove_last_char(token);
                    }
                    counter_space = 0;    // should be anyway, but just in case
                    next_state = ML_STRING_WHITESPACE_SKIP;
                } else if (isspace(c)){
                    if(token_assign_char(token,c) == COMPILER_ERROR){
                            token_reset(token);
                            set_error(token);
                            token->data.integer = COMPILER_ERROR;
                            return;
                    }
                    counter_space++;
                } else if (c == '\"') {
                    next_state = ML_STRING_WHITESPACE_SKIP_QUOTE;
                    counter_quote = 1;
                } else {
                    ungetc(c, stdin);
                    next_state = ML_STRING_STATE;
                }
                break;
                break;

            case ML_STRING_WHITESPACE_SKIP:
                if (c == '\n') {
                    counter_space = 0;
                    if(token_assign_char(token,'\n') == COMPILER_ERROR){
                            token_reset(token);
                            set_error(token);
                            token->data.integer = COMPILER_ERROR;
                            return;
                    }
                    next_state = ML_STRING_WHITESPACE_SKIP;
                } else if (isspace(c)){
                    if(token_assign_char(token,c) == COMPILER_ERROR){
                            token_reset(token);
                            set_error(token);
                            token->data.integer = COMPILER_ERROR;
                            return;
                    }
                    counter_space++;
                } else if (c == '\"') {
                    next_state = ML_STRING_WHITESPACE_SKIP_QUOTE;
                    counter_quote = 1;
                } else {
                    ungetc(c, stdin);
                    next_state = ML_STRING_STATE;
                }
                break;

            case ML_STRING_WHITESPACE_SKIP_QUOTE:
                if (c == '\"') {
                    counter_quote++;
                } else {
                    for (; counter_quote > 0; counter_quote--){                        // puts one " for every " in a row it counted.
                        token->type = ML_STRING_TOKEN;
                        if(token_assign_char(token,'\"') == COMPILER_ERROR){
                            token_reset(token);
                            set_error(token);
                            token->data.integer = COMPILER_ERROR;
                            return;
                        }
                    }
                    ungetc(c, stdin);
                    next_state = ML_STRING_STATE;
                }
                if (counter_quote >= 3){
                    token->type = ML_STRING_TOKEN;
                    for (; counter_space > 0; counter_space--){                        // removes trailing whitespace
                        token_remove_last_char(token);
                    }
                    token_remove_last_char(token);                                // removes last \n
                    return;
                }
                break;

            case UNDERSCORE_STATE:
                if (c == '_'){
                    next_state = GLOBAL_IDENTIFIER_STATE;
                    token->type = GLOBAL_IDENTIFIER;
                    if(token_assign_char(token,c) == COMPILER_ERROR){
                        set_error(token);
                        token->data.integer = COMPILER_ERROR;
                        return;
                    }
                } else {
                    token_reset(token);
                    set_error(token);
                    return;
                }
                break;

            case GLOBAL_IDENTIFIER_STATE:
                if (isalnum(c) || c == '_'){
                    if(token_assign_char(token,c) == COMPILER_ERROR){
                        token_reset(token);
                        set_error(token);
                        token->data.integer = COMPILER_ERROR;
                        return;
                    }
                    next_state = GLOBAL_IDENTIFIER_STATE;
                } else {
                    ungetc(c, stdin);
                    token->type = GLOBAL_IDENTIFIER;
                    return;
                }
                break;

            case ERROR_STATE:
            default:
                token_reset(token);
                set_error(token);
                token->data.integer = COMPILER_ERROR;
                return;
        }
        if (state == ERROR_STATE) {
            break;
        }
    }

    return;
}

Token *get_token(void){
    Token *token = malloc(sizeof(Token));
    if (token == NULL) {
        return NULL;
    }
    token_reset(token);
    state_machine(token);
    return token;
}

void free_token(Token *token){
    if (token == NULL){
    return;
    }
    token_reset(token);
    free(token);
    return;
}
