/*
** BoredOS OS library for Lua 5.5.0 
** Replaces the standard loslib.c with BoredOS-native implementations.
*/

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "sysinclude/time.h"

static int os_clock(lua_State *L) {
    lua_pushnumber(L, (lua_Number)clock() / (lua_Number)CLOCKS_PER_SEC);
    return 1;
}

static int os_time(lua_State *L) {
    (void)L;
    lua_pushinteger(L, 0);
    return 1;
}

static int os_difftime(lua_State *L) {
    lua_pushnumber(L, (lua_Number)(luaL_checkinteger(L, 1) - luaL_optinteger(L, 2, 0)));
    return 1;
}

static int os_date(lua_State *L) {
    (void)L;
    lua_pushliteral(L, "unknown");
    return 1;
}

static int os_exit(lua_State *L) {
    int status = (int)luaL_optinteger(L, 1, 0);
    sys_exit(status);
    return 0;  /* never reached */
}

static int os_remove(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    int result = sys_delete(path);
    if (result == 0) {
        lua_pushboolean(L, 1);
        return 1;
    }
    lua_pushnil(L);
    lua_pushfstring(L, "cannot remove '%s'", path);
    return 2;
}

static int os_rename(lua_State *L) {
    (void)L;
    lua_pushnil(L);
    lua_pushliteral(L, "rename not supported on BoredOS");
    return 2;
}

static int os_tmpname(lua_State *L) {
    (void)L;
    lua_pushnil(L);
    lua_pushliteral(L, "tmpname not supported on BoredOS");
    return 2;
}

static int os_getenv(lua_State *L) {
    (void)L;
    lua_pushnil(L);
    return 1;
}

static int os_execute(lua_State *L) {
    (void)L;
    lua_pushnil(L);
    lua_pushliteral(L, "exit");
    lua_pushinteger(L, -1);
    return 3;
}

static int os_setlocale(lua_State *L) {
    (void)L;
    lua_pushliteral(L, "C");
    return 1;
}

static const luaL_Reg oslib[] = {
    {"clock",     os_clock},
    {"time",      os_time},
    {"difftime",  os_difftime},
    {"date",      os_date},
    {"exit",      os_exit},
    {"remove",    os_remove},
    {"rename",    os_rename},
    {"tmpname",   os_tmpname},
    {"getenv",    os_getenv},
    {"execute",   os_execute},
    {"setlocale", os_setlocale},
    {NULL, NULL}
};

LUAMOD_API int luaopen_os(lua_State *L) {
    luaL_newlib(L, oslib);
    return 1;
}
