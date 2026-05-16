/*
** BoredOS package/loadlib for Lua 5.5.0
** Replaces loadlib.c — no dynamic C module loading on BoredOS.
** Only supports loading Lua files from the BoredOS VFS.
** Search paths are read from package.path at runtime, never hardcoded.
*/

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

/*
** Search for a Lua file using the path templates in 'package.path'.
** Path templates use ';' as separator and '?' as the module name placeholder.
** Dots in the module name are replaced with '/' for directory separators.
*/
static int searcher_Lua(lua_State *L) {
    const char *name = luaL_checkstring(L, 1);

    /* Get package.path from upvalue or registry */
    lua_getglobal(L, "package");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_pushfstring(L, "no file found for module '%s' (package not a table)", name);
        return 1;
    }
    lua_getfield(L, -1, "path");
    const char *path = lua_tostring(L, -1);
    if (path == NULL) {
        lua_pop(L, 2);
        lua_pushfstring(L, "no file found for module '%s' (package.path is nil)", name);
        return 1;
    }

    /* Build a "cleaned" module name: replace dots with slashes */
    char modname[256];
    int mi = 0;
    for (const char *n = name; *n && mi < 254; n++) {
        modname[mi++] = (*n == '.') ? '/' : *n;
    }
    modname[mi] = '\0';

    /* Iterate through semicolon-separated path templates */
    luaL_Buffer msg;
    luaL_buffinit(L, &msg);

    const char *p = path;
    while (*p) {
        /* Extract one template (up to ';' or end) */
        const char *tpl_start = p;
        while (*p && *p != ';') p++;
        int tpl_len = (int)(p - tpl_start);
        if (*p == ';') p++;  /* skip separator */

        /* Build full path by replacing '?' with modname */
        char fullpath[512];
        int j = 0;
        for (int i = 0; i < tpl_len && j < 510; i++) {
            if (tpl_start[i] == '?') {
                for (int k = 0; modname[k] && j < 510; k++)
                    fullpath[j++] = modname[k];
            } else {
                fullpath[j++] = tpl_start[i];
            }
        }
        fullpath[j] = '\0';

        /* Check if the file exists on the VFS */
        if (sys_exists(fullpath)) {
            int status = luaL_loadfile(L, fullpath);
            if (status == LUA_OK) {
                lua_pushstring(L, fullpath);  /* 2nd result: file path */
                return 2;
            }
            /* Load error — report it */
            lua_pop(L, 2);  /* pop package table and path string */
            return luaL_error(L, "error loading module '%s' from file '%s':\n\t%s",
                name, fullpath, lua_tostring(L, -1));
        }

        /* Accumulate "not found" messages */
        luaL_addstring(&msg, "\n\tno file '");
        luaL_addstring(&msg, fullpath);
        luaL_addchar(&msg, '\'');
    }

    lua_pop(L, 2);  /* pop package table and path string */
    luaL_pushresult(&msg);
    return 1;
}

/* Stub C module searcher — always fails */
static int searcher_C(lua_State *L) {
    const char *name = luaL_checkstring(L, 1);
    lua_pushfstring(L, "\n\tC modules not supported on BoredOS (module '%s')", name);
    return 1;
}

/* Preload searcher */
static int searcher_preload(lua_State *L) {
    const char *name = luaL_checkstring(L, 1);
    lua_getfield(L, LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
    if (lua_getfield(L, -1, name) == LUA_TNIL) {
        lua_pushfstring(L, "\n\tno field package.preload['%s']", name);
        return 1;
    }
    lua_pushliteral(L, ":preload:");
    return 2;
}

static int ll_require(lua_State *L) {
    const char *name = luaL_checkstring(L, 1);
    lua_settop(L, 1);

    /* Check if already loaded */
    luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
    lua_getfield(L, 2, name);  /* LOADED[name] */
    if (lua_toboolean(L, -1))   /* already loaded? */
        return 1;
    lua_pop(L, 1);

    /* Try searchers in order */
    lua_CFunction searchers[] = {searcher_preload, searcher_Lua, searcher_C, NULL};

    luaL_Buffer msg;
    luaL_buffinit(L, &msg);

    for (int i = 0; searchers[i]; i++) {
        lua_pushcfunction(L, searchers[i]);
        lua_pushstring(L, name);
        lua_call(L, 1, 2);
        if (lua_isfunction(L, -2)) {
            /* Found a loader */
            lua_pushstring(L, name);
            lua_call(L, 2, 1);  /* call loader(name, extra) */
            if (!lua_isnil(L, -1))
                lua_setfield(L, 2, name);  /* LOADED[name] = result */
            if (lua_getfield(L, 2, name) == LUA_TNIL) {
                lua_pushboolean(L, 1);
                lua_copy(L, -1, -2);
                lua_setfield(L, 2, name);  /* LOADED[name] = true */
            }
            return 1;
        }
        /* Accumulate error message from searcher */
        if (lua_isstring(L, -2)) {
            const char *s = lua_tostring(L, -2);
            luaL_addstring(&msg, s);
        }
        lua_settop(L, 2);
    }

    luaL_pushresult(&msg);
    return luaL_error(L, "module '%s' not found:%s", name, lua_tostring(L, -1));
}

static const luaL_Reg pkg_funcs[] = {
    {NULL, NULL}
};

static const luaL_Reg ll_funcs[] = {
    {"require", ll_require},
    {NULL, NULL}
};

LUAMOD_API int luaopen_package(lua_State *L) {
    /* No cleanup needed on BoredOS */
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, "_CLIBS");

    luaL_newlib(L, pkg_funcs);

    /* Default search path — users can change package.path at runtime */
    lua_pushliteral(L, "./?.lua;./?/init.lua");
    lua_setfield(L, -2, "path");

    lua_pushliteral(L, "");
    lua_setfield(L, -2, "cpath");

    lua_pushliteral(L, ";");
    lua_setfield(L, -2, "config");

    /* Create preload table */
    luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
    lua_setfield(L, -2, "preload");

    /* Create loaded table */
    luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
    lua_setfield(L, -2, "loaded");

    /* Set 'require' as global */
    lua_pushglobaltable(L);
    luaL_setfuncs(L, ll_funcs, 0);
    lua_pop(L, 1);

    return 1;
}
