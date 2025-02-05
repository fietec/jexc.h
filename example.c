#include <stdio.h>
#define JEXC_IMPLEMENTATION
#include "jexc.h"

int main(void)
{
    char buffer[] = "{  \"first\": 1, \"second\" : false, \n\"third\":[1.0 , 23, null]}";
    JexcLexer lexer = jexc_init(buffer, strlen(buffer), "buffer");
    JexcToken token;
    
    while (jexc_next(&lexer, &token)){
        jexc_print(token);
    }
    return 0;
}