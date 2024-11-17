#include "lexer.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>

static int has_at_least(const char* str, const int n) {
    return tokenizer->pos + n < strlen(str);
}

static int is_eof() {
    return !has_at_least(tokenizer->input, 0);
}

static int is_valid_ident_start() {
    char c = tokenizer->input[tokenizer->pos];
    return isalpha(c) || c == '_';
}

static int is_valid_ident() {
    char c = tokenizer->input[tokenizer->pos];
    return isalpha(c) || isdigit(c) || c == '_';
}

static int compare(const char* str1, const char* str2) {
    return strcmp(str1, str2) == 0;
}

static char* safe_malloc(size_t size) {
    char* ptr = (char*)malloc(size);
    if (!ptr) {
        fprintf(stderr, "Memory allocation failed\n");
        //exit(1);
    }
    return ptr;
}

static char* safe_realloc(char* ptr, size_t size) {
    ptr = (char*)realloc(ptr, size);
    if (!ptr) {
        fprintf(stderr, "Memory allocation failed\n");
        //exit(1);
    }
    return ptr;
}

char* token_to_str(const token_type_t type) {
    switch (type) {
        case LOCAL:    return "local";
        case IF:       return "if";
        case IN:       return "in";
        case NIL:      return "nil";
        case NOT:      return "not";
        case REPEAT:   return "repeat";
        case OR:       return "or";
        case THEN:     return "then";
        case TRUE:     return "true";
        case WHILE:    return "while";
        case UNTIL:    return "until";
        case RETURN:   return "return";
        case AND:      return "and";
        case GOTO:     return "goto";
        case FUNCTION: return "function";
        case END:      return "end";
        case FALSE:    return "false";
        case FOR:      return "for";
        case ELSE:     return "else";
        case ELSEIF:   return "elseif";
        case DO:       return "do";
        case BREAK:    return "break";

        case IDENT:                   return "ident";
        case NUMBER:                  return "number";
        case STRING_LITERAL:          return "string_literal";
        case UNCLOSED_STRING_LITERAL: return "unclosed_string_literal";

        case ASSIGN:      return "assign";
        case GREATER_EQ:  return "greater_eq";
        case LESS_EQ:     return "less_eq";
        case EQ:          return "eq";
        case NOT_EQ:      return "not_eq";
        case GREATER:     return "greater";
        case LESS:        return "less";
        case SHIFT_LEFT:  return "shift_left";
        case SHIFT_RIGHT: return "shift_right";

        case ADD: return "add";
        case SUB: return "sub";
        case MUL: return "mul";
        case DIV: return "div";
        case MOD: return "mod";
        case POW: return "pow";
        case LEN: return "len";

        case SHORT_COMMENT: return "short_comment";
        case LONG_COMMENT:  return "long_comment";

        case LEFT_PAREN:    return "left_paren";
        case RIGHT_PAREN:   return "right_paren";
        case LEFT_BRACKET:  return "left_bracket";
        case RIGHT_BRACKET: return "right_bracket";
        case LEFT_CURLY:    return "left_curly";
        case RIGHT_CURLY:   return "right_curly";

        case CONCAT:       return "concat";
        case DOTS:         return "dots";
        case DOUBLE_COLON: return "double_colon";
        case COLON:        return "colon";
        case COMMA:        return "comma";
        case SEMICOLON:    return "semicolon";
        case ATTR:         return "attr";
        case WHITESPACE:   return "whitespace";
        case UNIDENTIFIED: return "unidentified";
        case END_OF_FILE:  return "end_of_file";
    }
    return NULL;
}

static token_type_t keyword_to_token_type(const char* keyword) {
    if (compare(keyword, "and")) {
        return AND;
    } else if (compare(keyword, "goto")) {
        return GOTO;
    } else if (compare(keyword, "function")) {
        return FUNCTION;
    } else if (compare(keyword, "end")) {
        return END;
    } else if (compare(keyword, "false")) {
        return FALSE;
    } else if (compare(keyword, "for")) {
        return FOR;
    } else if (compare(keyword, "else")) {
        return ELSE;
    } else if (compare(keyword, "elseif")) {
        return ELSEIF;
    } else if (compare(keyword, "do")) {
        return DO;
    } else if (compare(keyword, "break")) {
        return BREAK;
    } else if (compare(keyword, "if")) {
        return IF;
    } else if (compare(keyword, "in")) {
        return IN;
    } else if (compare(keyword, "local")) {
        return LOCAL;
    } else if (compare(keyword, "nil")) {
        return NIL;
    } else if (compare(keyword, "not")) {
        return NOT;
    } else if (compare(keyword, "or")) {
        return OR;
    } else if (compare(keyword, "repeat")) {
        return REPEAT;
    } else if (compare(keyword, "return")) {
        return RETURN;
    } else if (compare(keyword, "then")) {
        return THEN;
    } else if (compare(keyword, "true")) {
        return TRUE;
    } else if (compare(keyword, "until")) {
        return UNTIL;
    } else if (compare(keyword, "while")) {
        return WHILE;
    }
    return IDENT;
}

token_t* create_token(token_type_t type, const char* value) {
    token_t* token = safe_malloc(sizeof(token_t));
    token->type = type;
    if (value) {
        token->value = strdup(value);
    } else {
        token->value = NULL;
    }
    return token;
}



// 数字处理
static token_t* read_digit() {
    size_t start = tokenizer->pos;

    // 读取整数部分
    while (isdigit(tokenizer->input[tokenizer->pos]) && !is_eof()) {
        tokenizer->pos++;
    }

    // 处理小数点
    if (tokenizer->input[tokenizer->pos] == '.') {
        tokenizer->pos++;
        if (tokenizer->input[tokenizer->pos] == '.') {
            tokenizer->pos--; // 退回一个位置,让后续处理识别拼接符
        } else {
            while (isdigit(tokenizer->input[tokenizer->pos]) && !is_eof()) {
                tokenizer->pos++;
            }
        }
    }

    // 处理科学计数法
    if (tokenizer->input[tokenizer->pos] == 'e' || tokenizer->input[tokenizer->pos] == 'E') {
        tokenizer->pos++;
        if (tokenizer->input[tokenizer->pos] == '+' || tokenizer->input[tokenizer->pos] == '-') {
            tokenizer->pos++;
        }
        if (!isdigit(tokenizer->input[tokenizer->pos])) {
            fprintf(stderr, "错误: 科学计数法的指数部分必须是数字\n");
            //exit(1);
        }
        while (isdigit(tokenizer->input[tokenizer->pos]) && !is_eof()) {
            tokenizer->pos++;
        }
    }

    size_t length = tokenizer->pos - start;
    char* number = (char*)malloc(length + 1);
    strncpy(number, tokenizer->input + start, length);
    number[length] = '\0';

    // 处理 `..` 的情况
    if (strcmp(number, "..") == 0) {
        free(number);
        return create_token(CONCAT, "..");
    }

    return create_token(NUMBER, number);
}


// 标识符处理
static token_t* read_ident() {
    size_t start = tokenizer->pos;

    // 读取标识符
    while (is_valid_ident() && !is_eof()) {
        tokenizer->pos++;
    }

    size_t length = tokenizer->pos - start;
    char* ident = (char*)malloc(length + 1);
    strncpy(ident, tokenizer->input + start, length);
    ident[length] = '\0';

    // 检查标识符前是否有点号,跳过空格
    if (start > 0) {
        size_t check_pos = start - 1;
        while (check_pos > 0 && tokenizer->input[check_pos] == ' ') {
            check_pos--;
        }
        if (tokenizer->input[check_pos] == '.' && (check_pos == 0 || tokenizer->input[check_pos - 1] != '.')) {
            // 单个点号,创建数组元素token
            char* array_ident = (char*)malloc(length + 3);
            snprintf(array_ident, length + 3, "[%s]", ident);
            free(ident);
            return create_token(IDENT, array_ident);
        }
    }

    token_type_t type = keyword_to_token_type(ident);
    if (type == IDENT) {
        // 跳过标识符后可能存在的空格
        while (tokenizer->input[tokenizer->pos] == ' ') {
            tokenizer->pos++;
        }

        // 检查标识符后是否有点号
        if (tokenizer->input[tokenizer->pos] == '.') {
            tokenizer->pos++; // 跳过单个点号

            // 再次跳过点号后可能存在的空格
            while (tokenizer->input[tokenizer->pos] == ' ') {
                tokenizer->pos++;
            }

            // 如果是双点号,则还原到第一个点号位置
            if (tokenizer->input[tokenizer->pos] == '.') {
                tokenizer->pos--; // 回到第一个点号
            }
        }
        return create_token(IDENT, ident);
    }
    free(ident);
    return create_token(type, NULL);
}


static token_t* read_short_string() {
    char* string = safe_malloc(1);
    size_t size = 0;
    char quote = tokenizer->input[tokenizer->pos++];
    const char* input = tokenizer->input;

    while (input[tokenizer->pos] != quote && input[tokenizer->pos] != '\0') {
        string = safe_realloc(string, ++size + 1);
        string[size - 1] = input[tokenizer->pos++];
    }
    tokenizer->pos++;
    string[size] = '\0';

    token_t* token = create_token(STRING_LITERAL, string);
    free(string);
    return token;
}


static int starts_with(const char* str, const char* prefix, const int pos) {
    for (int i = 0; i < strlen(prefix); ++i) {
        if (str[pos + i] != prefix[i]) {
            return 0;
        }
    }
    return 1;
}

static int starts_with_pattern(const char* str, const char* pattern, const int pos, int* level) {
    *level = 0;
    while (str[pos + *level] == '=') {
        (*level)++;
    }
    return starts_with(str, pattern, pos + *level);
}

// 读取长注释,包括起始标记,内容,结束标记
static token_t* read_long_comment() {
    const char* input = tokenizer->input;
    size_t start = tokenizer->pos;
    int level = 0;

    // 跳过初始的 --[
    tokenizer->pos += 2;
    while (input[tokenizer->pos] == '=') {
        level++;
        tokenizer->pos++;
    }

    // 跳过起始的 [[
    tokenizer->pos += 2;

    // 记录注释的起始位置,包括起始标记
    size_t content_start = start;
    int is_closed = 0;

    while (!is_closed && !is_eof()) {
        // 查找结束标记 ]=...=]
        if (input[tokenizer->pos] == ']') {
            int closing_level = 0;
            tokenizer->pos++; // 跳过第一个 ]
            while (input[tokenizer->pos] == '=') {
                closing_level++;
                tokenizer->pos++;
            }
            if (input[tokenizer->pos] == ']') {
                is_closed = 1;
                tokenizer->pos++; // 跳过结尾的 ]
            }
        } else {
            tokenizer->pos++;
        }
    }

    // 提取整个注释内容,包括起始和结束标记
    size_t length = tokenizer->pos - content_start + 2;
    char* full_comment = (char*)malloc(length + 1);
    strncpy(full_comment, input + content_start - 2, length);
    full_comment[length] = '\0';

    token_t* token = create_token(LONG_COMMENT, full_comment);
    free(full_comment);
    return token;
}



static token_t* next_token() {
    while (!is_eof()) {
        char c = tokenizer->input[tokenizer->pos];

        // 保留必要的空格
        if (isspace(c)) {
            size_t start = tokenizer->pos;
            while (isspace(tokenizer->input[tokenizer->pos]) && !is_eof()) {
                tokenizer->pos++;
            }
            size_t length = tokenizer->pos - start;
            if (c == '\n') {
                continue;
            }
            char* whitespace = (char*)malloc(length + 1);
            strncpy(whitespace, tokenizer->input + start, length);
            whitespace[length] = '\0';
            return create_token(WHITESPACE, whitespace);
        } 

        // 数字处理
        else if (isdigit(c) || (c == '.' && isdigit(tokenizer->input[tokenizer->pos + 1]))) {
            return read_digit();
        } 

        // 标识符处理
        else if (is_valid_ident_start()) {
            return read_ident();
        } 

        // 字符串处理
        else if (c == '\"' || c == '\'') {
            tokenizer->pos++;
            size_t start = tokenizer->pos;
            while (tokenizer->input[tokenizer->pos] != c && !is_eof()) {
                tokenizer->pos++;
            }
            if (is_eof()) {
                size_t length = tokenizer->pos - start;
                char* unclosed_string = (char*)malloc(length + 1);
                strncpy(unclosed_string, tokenizer->input + start, length);
                unclosed_string[length] = '\0';
                return create_token(UNCLOSED_STRING_LITERAL, unclosed_string);
            }
            size_t length = tokenizer->pos - start;
            char* string = (char*)malloc(length + 1);
            strncpy(string, tokenizer->input + start, length);
            string[length] = '\0';
            tokenizer->pos++;
            return create_token(STRING_LITERAL, string);
        } 

        else {
            switch (c) {
                case '+': tokenizer->pos++; return create_token(ADD, "+");
                case '*': tokenizer->pos++; return create_token(MUL, "*");
                case '/': tokenizer->pos++; return create_token(DIV, "/");
                case '%': tokenizer->pos++; return create_token(MOD, "%");
                case '^': tokenizer->pos++; return create_token(POW, "^");
                case '#': tokenizer->pos++; return create_token(LEN, "#");
                case '(': tokenizer->pos++; return create_token(LEFT_PAREN, "(");
                case ')': tokenizer->pos++; return create_token(RIGHT_PAREN, ")");
                case '[': tokenizer->pos++; return create_token(LEFT_BRACKET, "[");
                case ']': tokenizer->pos++; return create_token(RIGHT_BRACKET, "]");
                case '{': tokenizer->pos++; return create_token(LEFT_CURLY, "{");
                case '}': tokenizer->pos++; return create_token(RIGHT_CURLY, "}");
                case '.':
                    if (starts_with(tokenizer->input, "...", tokenizer->pos)) {
                        tokenizer->pos += 3;
                        return create_token(DOTS, "...");
                    }
                    if (starts_with(tokenizer->input, "..", tokenizer->pos)) {
                        tokenizer->pos += 2;
                        // 跳过双点号后面的空格
                        while (tokenizer->input[tokenizer->pos] == ' ') {
                            tokenizer->pos++;
                        }
                        return create_token(CONCAT, "..");
                    }
                    tokenizer->pos++;
                    return create_token(ATTR, ".");
                case ':':
                    if (starts_with(tokenizer->input, "::", tokenizer->pos)) {
                        tokenizer->pos += 2;
                        return create_token(DOUBLE_COLON, "::");
                    }
                    tokenizer->pos++;
                    return create_token(COLON, ":");
                case ',':
                    tokenizer->pos++;
                    return create_token(COMMA, ",");
                case ';':
                    tokenizer->pos++;
                    return create_token(SEMICOLON, ";");
                case '=':
                    if (tokenizer->input[tokenizer->pos + 1] == '=') {
                        tokenizer->pos += 2;
                        return create_token(EQ, "==");
                    }
                    tokenizer->pos++;
                    return create_token(ASSIGN, "=");
                case '>':
                    if (tokenizer->input[tokenizer->pos + 1] == '=') {
                        tokenizer->pos += 2;
                        return create_token(GREATER_EQ, ">=");
                    }
                    tokenizer->pos++;
                    return create_token(GREATER, ">");
                case '<':
                    if (tokenizer->input[tokenizer->pos + 1] == '=') {
                        tokenizer->pos += 2;
                        return create_token(LESS_EQ, "<=");
                    }
                    tokenizer->pos++;
                    return create_token(LESS, "<");
                case '~':
                    if (tokenizer->input[tokenizer->pos + 1] == '=') {
                        tokenizer->pos += 2;
                        return create_token(NOT_EQ, "~=");
                    }
                    tokenizer->pos++;
                    return create_token(UNIDENTIFIED, "~");
                case '-':
                if (starts_with(tokenizer->input, "--", tokenizer->pos)) {
                    size_t start = tokenizer->pos;
                    tokenizer->pos += 2;
                    int level = 0;

                    // 处理长注释
                    if (starts_with_pattern(tokenizer->input, "[", tokenizer->pos, &level)) {
                        return read_long_comment();
                    }

                    // 处理短注释
                    while (tokenizer->input[tokenizer->pos] != '\n' && !is_eof()) {
                        tokenizer->pos++;
                    }

                    size_t length = tokenizer->pos - start;
                    char* short_comment = (char*)malloc(length + 1);
                    strncpy(short_comment, tokenizer->input + start, length);
                    short_comment[length] = '\0';

                    return create_token(SHORT_COMMENT, short_comment);
                }
                tokenizer->pos++;
                return create_token(SUB, "-");
                default:
                    tokenizer->pos++;
                    return create_token(UNIDENTIFIED, &c);
            }
        }
    }
    return create_token(END_OF_FILE, NULL);
}


token_t** tokenize(const char* input) {
    tokenizer = (tokenizer_t*)malloc(sizeof(tokenizer_t));
    tokenizer->input = strdup(input);
    tokenizer->pos = 0;

    token_t** tokens = (token_t**)malloc(sizeof(token_t*) * 1024);
    int index = 0;

    token_t* token;
    while ((token = next_token())->type != END_OF_FILE) {
        tokens[index++] = token;
    }
    tokens[index] = token; // END_OF_FILE token
    tokens[index + 1] = NULL;

    return tokens;
}

