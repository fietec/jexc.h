# jecx.h

A simple header-only streaming json lexer written in C.

## Usage
`jexc.h` is an [stb-style](https://github.com/nothings/stb/blob/master/docs/stb_howto.txt) library, which means a single header file and no further dependencies. Define `JEXC_IMPLEMENTATION` to access the function implementations, otherwise `jexc.h` will act as a regular header-file.
```c 
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
```
Output:
```console
buffer:1:1: MapOpen: '{'
buffer:1:4: String: 'first'
buffer:1:11: MapSep: ':'
buffer:1:13: Int: '1'
buffer:1:14: Sep: ','
buffer:1:16: String: 'second'
buffer:1:25: MapSep: ':'
buffer:1:27: Bool: 'false'
buffer:1:32: Sep: ','
buffer:2:1: String: 'third'
buffer:2:8: MapSep: ':'
buffer:2:9: ArrayOpen: '['
buffer:2:10: Float: '1.0'
buffer:2:14: Sep: ','
buffer:2:16: Int: '23'
buffer:2:18: Sep: ','
buffer:2:20: Null: 'null'
buffer:2:24: ArrayClose: ']'
buffer:2:25: MapClose: '}'
```

## Functions
```c 
JexcLexer jexc_init(char *buffer, size_t buffer_size, char *buffer_name); // initilize the lexer
bool jexc_next(JexcLexer *lexer, JexcToken *token); // fetch the next token
bool jexc_expect(lexer, token, (JexcTokenType)...) // fetch the next token and expect one of the given token types (macro)
bool jexc_extract(JexcToken *token, char *buffer, size_t buffer_size); // extract the content of a token into a buffer
void jexc_print(JexcToken token); // print a token
```