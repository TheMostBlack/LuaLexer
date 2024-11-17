#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>

#include "lexer.h"

static int lua_tokenize(lua_State* L) {
    const char* input = luaL_checkstring(L, 1);
    token_t** tokens = tokenize(input);

    lua_newtable(L);

    for (int i = 0; tokens[i] != NULL; i++) {
        lua_newtable(L);

        lua_pushstring(L, "type");
        lua_pushstring(L, token_to_str(tokens[i]->type));
        lua_settable(L, -3);

        lua_pushstring(L, "value");
        lua_pushstring(L, tokens[i]->value);
        lua_settable(L, -3);

        lua_rawseti(L, -2, i + 1);

        free(tokens[i]->value);
        free(tokens[i]);
    }
    free(tokens);
    return 1;
}

static const struct luaL_Reg lualexer[] = {
    { "tokenize", lua_tokenize },
    {NULL, NULL}
};

int luaopen_lualexer(lua_State *L) {
    luaL_newlib(L, lualexer);
    return 1;
}