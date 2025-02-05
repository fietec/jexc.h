/* 
    =========================================
    jexc.h <https://github.com/fietec/jexc.h>
    =========================================
    Copyright (c) 2025 Constantijn de Meer

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#ifndef _JEXC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define jexc_is_whitespace(c) ((c == ' ' || c == '\n' || c == '\t'))
#define jexc_check_line(lexer, c) do{if (c == '\n'){(lexer)->loc.row++; (lexer)->loc.column=1;}else{lexer->loc.column++;}}while(0)
#define jexc_inc(lexer) do{lexer->index++; lexer->loc.column++;}while(0)
#define jexc_get_char(lexer) (lexer->buffer[lexer->index])
#define jexc_get_pointer(lexer) (lexer->buffer + lexer->index)
#define jexc_loc_expand(loc) (loc).filename, (loc).row, (loc).column
#define jexc_token_args_array(...) (JexcTokenType[]){__VA_ARGS__}, jexc_args_len(__VA_ARGS__)

#define JEXC_LOC_FMT "%s:%d:%d"
#define jexc_arr_len(arr) ((arr)!= NULL ? sizeof((arr))/sizeof((arr)[0]):0)
#define jexc_args_len(...) sizeof((typeof(__VA_ARGS__)[]){__VA_ARGS__})/sizeof(typeof(__VA_ARGS__)/)

typedef enum{
    JexcToken_MapOpen,
    JexcToken_MapClose,
    JexcToken_ArrayOpen,
    JexcToken_ArrayClose,
    JexcToken_Sep,
    JexcToken_MapSep,
    JexcToken_String,
    JexcToken_Int,
    JexcToken_Float,
    JexcToken_True,
    JexcToken_False,
    JexcToken_Null,
    JexcToken_Invalid,
    JexcToken_End,
    JexcToken__Count
} JexcTokenType;

const char* const JexcTokenTypeNames[] = {
    [JexcToken_MapOpen] = "MapOpen",
    [JexcToken_MapClose] = "MapClose",
    [JexcToken_ArrayOpen] = "ArrayOpen",
    [JexcToken_ArrayClose] = "ArrayClose",
    [JexcToken_Sep] = "Sep",
    [JexcToken_MapSep] = "MapSep",
    [JexcToken_String] = "String",
    [JexcToken_Int] = "Int",
    [JexcToken_Float] = "Float",
    [JexcToken_True] = "Bool",
    [JexcToken_False] = "Bool",
    [JexcToken_Null] = "Null",
    [JexcToken_Invalid] = "Invalid",
    [JexcToken_End] = "--END--"
};

_Static_assert(JexcToken__Count == jexc_arr_len(JexcTokenTypeNames), "JexcTokenType count has changed!");

typedef struct{
    char *filename;
    size_t row;
    size_t column;
} JexcLoc;

typedef struct{
    JexcTokenType type;
    char *t_start;
    char *t_end;
    JexcLoc loc;
} JexcToken;

typedef struct{
    char *buffer;
    size_t buffer_size;
    size_t index;
    JexcLoc loc;
} JexcLexer;

#define jexc_expect(lexer, token, ...) jexc__expect(lexer, token, jexc_token_args_array(__VA_ARGS__))
JexcLexer jexc_init(char *buffer, size_t buffer_size, char *filename);
bool jexc_next(JexcLexer *lexer, JexcToken *token);
bool jexc_extract(JexcToken *token, char *buffer, size_t buffer_size);
void jexc_print(JexcToken token);

void jexc_trim_left(JexcLexer *lexer);
bool jexc_find(JexcLexer *lexer, char c);
void jexc_set_token(JexcToken *token, JexcTokenType type, char *t_start, char *t_end, JexcLoc loc);
bool jexc_is_delimeter(char c);
bool jexc_is_int(char *s, char *e);
bool jexc_is_float(char *s, char *e);

// helper utility functions
char* jexc_read_entire_file(char *file_path);
unsigned long long jexc_file_size(const char* file_path);

bool jexc__expect(JexcLexer *lexer, JexcToken *token, JexcTokenType types[], size_t count);


#endif // _JEXC_H

#ifdef JEXC_IMPLEMENTATION

JexcLexer jexc_init(char *buffer, size_t buffer_size, char *filename)
{
    return (JexcLexer) {.buffer=buffer, .buffer_size=buffer_size, .index=0, .loc=(JexcLoc){.filename=filename, .row=1, .column=1}};
}

bool jexc_next(JexcLexer *lexer, JexcToken *token)
{
    if (lexer == NULL || token == NULL || lexer->index > lexer->buffer_size) return false;
    jexc_trim_left(lexer);
    char *t_start = jexc_get_pointer(lexer);
    JexcLoc t_loc = lexer->loc;
    switch (jexc_get_char(lexer)){
        case '{':{
            jexc_set_token(token, JexcToken_MapOpen, t_start, t_start+1, t_loc);
            break;
        }
        case '}':{
            jexc_set_token(token, JexcToken_MapClose, t_start, t_start+1, t_loc);
            break;
        }
        case '[':{
            jexc_set_token(token, JexcToken_ArrayOpen, t_start, t_start+1, t_loc);
            break;
        }
        case ']':{
            jexc_set_token(token, JexcToken_ArrayClose, t_start, t_start+1, t_loc);
            break;
        }
        case ',':{
            jexc_set_token(token, JexcToken_Sep, t_start, t_start+1, t_loc);
            break;
        }
        case ':':{
            jexc_set_token(token, JexcToken_MapSep, t_start, t_start+1, t_loc);
            break;
        }
        case '"':{
            // lex strings
            jexc_inc(lexer);
            char *s_start = jexc_get_pointer(lexer);
            if (!jexc_find(lexer, '"')){
                fprintf(stderr, "[ERROR] Missing closing delimeter for '\"' at " JEXC_LOC_FMT "\n", jexc_loc_expand(t_loc));
                return false;
            }
            char *s_end = jexc_get_pointer(lexer);
            jexc_set_token(token, JexcToken_String, s_start, s_end, t_loc);
            break;
        }
        case '\0':
        case EOF:{
            jexc_set_token(token, JexcToken_End, t_start, t_start+1, t_loc);
            jexc_inc(lexer);
            return false;
        }
        default:{
            // multi-character literal
            // find end of literal
            char c;
            while (lexer->index < lexer->buffer_size && !jexc_is_delimeter(c = jexc_get_char(lexer))){
                jexc_check_line(lexer, c);
                lexer->index++;
            }
            char *t_end = jexc_get_pointer(lexer);
            size_t t_len = t_end-t_start;
            // check for known literals
            if (memcmp(t_start, "true", t_len) == 0){
                jexc_set_token(token, JexcToken_True, t_start, t_end, t_loc);
                return true;
            }
            if (memcmp(t_start, "false", t_len) == 0){
                jexc_set_token(token, JexcToken_False, t_start, t_end, t_loc);
                return true;
            }
            if (memcmp(t_start, "null", t_len) == 0){
                jexc_set_token(token, JexcToken_Null, t_start, t_end, t_loc);
                return true;
            }
            if (jexc_is_int(t_start, t_end)){
                jexc_set_token(token, JexcToken_Int, t_start, t_end, t_loc);
                return true;
            }
            if (jexc_is_float(t_start, t_end)){
                jexc_set_token(token, JexcToken_Float, t_start, t_end, t_loc);
                return true;
            }
            fprintf(stderr, "[ERROR] Invalid literal \"%.*s\" at "JEXC_LOC_FMT"\n", t_end, t_start, t_loc);
            jexc_set_token(token, JexcToken_Invalid, t_start, t_end, t_loc);
            return false;
        }
    }
    jexc_inc(lexer);
    return true;
}

bool jexc__expect(JexcLexer *lexer, JexcToken *token, JexcTokenType types[], size_t count)
{
    if (lexer == NULL || token == NULL) return false;
    jexc_next(lexer, token);
    for (size_t i=0; i<count; ++i){
        if (token->type == types[i]) return true;
    }
    fprintf(stderr, "[ERROR] "JEXC_LOC_FMT " Expected token of type [", jexc_loc_expand(token->loc));
    size_t i;
    for (i=0; i<count-1; ++i){
        fprintf(stderr, "%s, ", JexcTokenTypeNames[types[i]]);
    }
    fprintf(stderr, "%s] but got %s!\n", JexcTokenTypeNames[types[i]], JexcTokenTypeNames[token->type]);
    return false;  
}

bool jexc_extract(JexcToken *token, char *buffer, size_t buffer_size)
{
    if (token == NULL || buffer == NULL || buffer_size == 0) return false;
    size_t t_len = token->t_end-token->t_start;
    if (t_len >= buffer_size) return false;
    if (token->type == JexcToken_String){
        char temp_buffer[t_len+1];
        memset(temp_buffer, 0, t_len+1);
        char *r = token->t_start;
        char *w = temp_buffer;
        while (r != token->t_end){
            if (*r == '\\'){
                switch(*++r){
                    case '\'': *w = 0x27; break;
					case '"':  *w = 0x22; break;
					case '?':  *w = 0x3f; break;
					case '\\': *w = 0x5c; break;
					case 'a':  *w = 0x07; break;
					case 'b':  *w = 0x08; break;
					case 'f':  *w = 0x0c; break;
					case 'n':  *w = 0x0a; break;
					case 'r':  *w = 0x0d; break;
					case 't':  *w = 0x09; break;
					case 'v':  *w = 0x0b; break;
                    default:{
                        *w++ = '\\';
                        *w = *r;
                    }
                }
            }
            else{
                *w = *r;
            }
            r++;
            w++;
        }
        sprintf(buffer, "%.*s\0", w-temp_buffer+1, temp_buffer);
    }
    else{
        sprintf(buffer, "%.*s\0", t_len, token->t_start);
    }
    return true;
}

void jexc_print(JexcToken token)
{
    printf(JEXC_LOC_FMT": %s: '%.*s'\n", jexc_loc_expand(token.loc), JexcTokenTypeNames[token.type], token.t_end-token.t_start, token.t_start);
}

void jexc_set_token(JexcToken *token, JexcTokenType type, char *t_start, char *t_end, JexcLoc loc)
{
    if (token == NULL) return;
    token->type = type;
    token->t_start = t_start;
    token->t_end = t_end,
    token->loc = loc;
}

bool jexc_find(JexcLexer *lexer, char c)
{
    char rc;
    while (lexer->index < lexer->buffer_size){
        if ((rc = jexc_get_char(lexer)) == c) return true;
        jexc_check_line(lexer, rc);
        lexer->index++;
    }
    return false;
}

void jexc_trim_left(JexcLexer *lexer)
{
    char c;
    while (lexer->index <= lexer->buffer_size && jexc_is_whitespace((c = jexc_get_char(lexer)))){
        jexc_check_line(lexer, c);
        lexer->index++;
    }
}

bool jexc_is_delimeter(char c)
{
    switch (c){
        case '{':
        case '}':
        case '[':
        case ']':
        case ',':
        case ':':
        case ' ':
        case '\n':
        case '\t':{
            return true;
        }
        default: return false;
    }
}

bool jexc_is_int(char *s, char *e)
{
	if (!s || !e || e-s < 1) return false;
	if (*s == '-' || *s == '+') s++;
	while (s < e){
		if (!isdigit(*s++)) return false;
	}
	return true;
}

bool jexc_is_float(char *s, char *e)
{
    char* ep = NULL;
    strtod(s, &ep);
    return (ep && ep == e);
}

#include <sys/stat.h>

unsigned long long jexc_file_size(const char* file_path){
    struct stat file;
    if (stat(file_path, &file) == -1){
        return 0;
    }
    return (unsigned long long) file.st_size;
}

// allocate and populate a string with the file's content
char* jexc_read_entire_file(char *file_path)
{
    if (file_path == NULL) return NULL;
    FILE *file = fopen(file_path, "r");
    if (file == NULL) return NULL;
    unsigned long long size = jexc_file_size(file_path);
    char *content = (char*) calloc(size+1, sizeof(*content));
    if (!content){
        fclose(file);
        return NULL;
    }
    fread(content, 1, size, file);
    fclose(file);
    return content;
}
#endif // JEXC_IMPLEMENTATION