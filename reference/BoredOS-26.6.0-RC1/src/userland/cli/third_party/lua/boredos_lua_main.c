/*
** BoredOS Lua 5.5.0 main / REPL
** Replaces the standard lua.c with BoredOS-native I/O.
** Uses sys_write() for output and sys_tty_read_in() for input.
*/

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "llimits.h"

#if !defined(LUA_PROMPT)
#define LUA_PROMPT      "> "
#define LUA_PROMPT2     ">> "
#endif

#if !defined(LUA_MAXINPUT)
#define LUA_MAXINPUT    512
#endif

#define LUA_PROGNAME    "lua"

static lua_State *globalL = NULL;
static const char *progname = LUA_PROGNAME;

static void lstop(lua_State *L, lua_Debug *ar) {
    (void)ar;
    lua_sethook(L, NULL, 0, 0);
    luaL_error(L, "interrupted!");
}

static void print_usage(const char *badoption) {
    lua_writestringerror("%s: ", progname);
    lua_writestringerror("unrecognized option '%s'\n", badoption);
    lua_writestringerror(
        "usage: %s [options] [script [args]]\n"
        "Available options are:\n"
        "  -e stat   execute string 'stat'\n"
        "  -i        enter interactive mode after executing 'script'\n"
        "  -l mod    require library 'mod' into global 'mod'\n"
        "  -v        show version information\n"
        "  -E        ignore environment variables\n"
        "  -W        turn warnings on\n"
        "  --        stop handling options\n"
        ,
        progname);
}

static void l_message(const char *pname, const char *msg) {
    if (pname) lua_writestringerror("%s: ", pname);
    lua_writestringerror("%s\n", msg);
}

static int report(lua_State *L, int status) {
    if (status != LUA_OK) {
        const char *msg = lua_tostring(L, -1);
        if (msg == NULL) msg = "(error message not a string)";
        l_message(progname, msg);
        lua_pop(L, 1);
    }
    return status;
}

static int msghandler(lua_State *L) {
    const char *msg = lua_tostring(L, 1);
    if (msg == NULL) {
        if (luaL_callmeta(L, 1, "__tostring") &&
            lua_type(L, -1) == LUA_TSTRING)
            return 1;
        else
            msg = lua_pushfstring(L, "(error object is a %s value)",
                                    luaL_typename(L, 1));
    }
    luaL_traceback(L, L, msg, 1);
    return 1;
}

static int docall(lua_State *L, int narg, int nres) {
    int status;
    int base = lua_gettop(L) - narg;
    lua_pushcfunction(L, msghandler);
    lua_insert(L, base);
    globalL = L;
    status = lua_pcall(L, narg, nres, base);
    lua_remove(L, base);
    return status;
}

static void print_version(void) {
    lua_writestring(LUA_COPYRIGHT, strlen(LUA_COPYRIGHT));
    lua_writeline();
}

static int dochunk(lua_State *L, int status) {
    if (status == LUA_OK) status = docall(L, 0, 0);
    return report(L, status);
}

static int dofile(lua_State *L, const char *name) {
    return dochunk(L, luaL_loadfile(L, name));
}

static int dostring(lua_State *L, const char *s, const char *name) {
    return dochunk(L, luaL_loadbuffer(L, s, strlen(s), name));
}

/* BoredOS-specific readline: uses sys_tty_read_in for line input */
static char *boredos_readline(char *buff, const char *prompt) {
    /* Write the prompt */
    sys_write(1, prompt, (int)strlen(prompt));

    /* Read a line */
    int n = sys_tty_read_in(buff, LUA_MAXINPUT - 1);
    if (n <= 0) return NULL;
    buff[n] = '\0';
    return buff;
}

static const char *get_prompt(lua_State *L, int firstline) {
    if (lua_getglobal(L, firstline ? "_PROMPT" : "_PROMPT2") == LUA_TNIL)
        return (firstline ? LUA_PROMPT : LUA_PROMPT2);
    else {
        const char *p = luaL_tolstring(L, -1, NULL);
        lua_remove(L, -2);
        return p;
    }
}

#define EOFMARK     "<eof>"
#define marklen     (sizeof(EOFMARK)/sizeof(char) - 1)

static int incomplete(lua_State *L, int status) {
    if (status == LUA_ERRSYNTAX) {
        size_t lmsg;
        const char *msg = lua_tolstring(L, -1, &lmsg);
        if (lmsg >= marklen && strcmp(msg + lmsg - marklen, EOFMARK) == 0)
            return 1;
    }
    return 0;
}

static int pushline(lua_State *L, int firstline) {
    char buffer[LUA_MAXINPUT];
    size_t l;
    const char *prmt = get_prompt(L, firstline);
    char *b = boredos_readline(buffer, prmt);
    lua_pop(L, 1);  /* remove prompt */
    if (b == NULL) return 0;
    l = strlen(b);
    if (l > 0 && b[l-1] == '\n')
        b[--l] = '\0';
    lua_pushlstring(L, b, l);
    return 1;
}

static int addreturn(lua_State *L) {
    const char *line = lua_tostring(L, -1);
    const char *retline = lua_pushfstring(L, "return %s;", line);
    int status = luaL_loadbuffer(L, retline, strlen(retline), "=stdin");
    if (status == LUA_OK)
        lua_remove(L, -2);
    else
        lua_pop(L, 2);
    return status;
}

static int multiline(lua_State *L) {
    size_t len;
    const char *line = lua_tolstring(L, 1, &len);
    for (;;) {
        int status = luaL_loadbuffer(L, line, len, "=stdin");
        if (!incomplete(L, status) || !pushline(L, 0))
            return status;
        lua_remove(L, -2);
        lua_pushliteral(L, "\n");
        lua_insert(L, -2);
        lua_concat(L, 3);
        line = lua_tolstring(L, 1, &len);
    }
}

static int loadline(lua_State *L) {
    int status;
    lua_settop(L, 0);
    if (!pushline(L, 1))
        return -1;
    if ((status = addreturn(L)) != LUA_OK)
        status = multiline(L);
    lua_remove(L, 1);
    lua_assert(lua_gettop(L) == 1);
    return status;
}

static void l_print(lua_State *L) {
    int n = lua_gettop(L);
    if (n > 0) {
        luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
        lua_getglobal(L, "print");
        lua_insert(L, 1);
        if (lua_pcall(L, n, 0, 0) != LUA_OK)
            l_message(progname, lua_pushfstring(L,
                "error calling 'print' (%s)", lua_tostring(L, -1)));
    }
}

static void doREPL(lua_State *L) {
    int status;
    const char *oldprogname = progname;
    progname = NULL;
    while ((status = loadline(L)) != -1) {
        if (status == LUA_OK)
            status = docall(L, 0, LUA_MULTRET);
        if (status == LUA_OK) l_print(L);
        else report(L, status);
    }
    lua_settop(L, 0);
    lua_writeline();
    progname = oldprogname;
}

static void createargtable(lua_State *L, char **argv, int argc, int script) {
    int i, narg;
    narg = argc - (script + 1);
    lua_createtable(L, narg, script + 1);
    for (i = 0; i < argc; i++) {
        lua_pushstring(L, argv[i]);
        lua_rawseti(L, -2, i - script);
    }
    lua_setglobal(L, "arg");
}

#define has_error    1
#define has_i        2
#define has_v        4
#define has_e        8
#define has_E        16

static int collectargs(char **argv, int *first) {
    int args = 0;
    int i;
    if (argv[0] != NULL) {
        if (argv[0][0])
            progname = argv[0];
    } else {
        *first = -1;
        return 0;
    }
    for (i = 1; argv[i] != NULL; i++) {
        *first = i;
        if (argv[i][0] != '-')
            return args;
        switch (argv[i][1]) {
            case '-':
                if (argv[i][2] != '\0') return has_error;
                *first = (argv[i+1] != NULL) ? i + 1 : 0;
                return args;
            case '\0':
                return args;
            case 'E':
                if (argv[i][2] != '\0') return has_error;
                args |= has_E;
                break;
            case 'W':
                if (argv[i][2] != '\0') return has_error;
                break;
            case 'i':
                args |= has_i;
                /* fallthrough */
            case 'v':
                if (argv[i][2] != '\0') return has_error;
                args |= has_v;
                break;
            case 'e':
                args |= has_e;
                /* fallthrough */
            case 'l':
                if (argv[i][2] == '\0') {
                    i++;
                    if (argv[i] == NULL || argv[i][0] == '-')
                        return has_error;
                }
                break;
            default:
                return has_error;
        }
    }
    *first = 0;
    return args;
}

static int runargs(lua_State *L, char **argv, int n) {
    int i;
    for (i = 1; i < n; i++) {
        int option = argv[i][1];
        lua_assert(argv[i][0] == '-');
        switch (option) {
            case 'e': case 'l': {
                int status;
                char *extra = argv[i] + 2;
                if (*extra == '\0') extra = argv[++i];
                lua_assert(extra != NULL);
                if (option == 'e')
                    status = dostring(L, extra, "=(command line)");
                else {
                    /* dolibrary */
                    lua_getglobal(L, "require");
                    lua_pushstring(L, extra);
                    status = docall(L, 1, 1);
                    if (status == LUA_OK) {
                        lua_setglobal(L, extra);
                    }
                    else report(L, status);
                }
                if (status != LUA_OK) return 0;
                break;
            }
            case 'W':
                lua_warning(L, "@on", 0);
                break;
        }
    }
    return 1;
}

static int pushargs(lua_State *L) {
    int i, n;
    if (lua_getglobal(L, "arg") != LUA_TTABLE)
        luaL_error(L, "'arg' is not a table");
    n = (int)luaL_len(L, -1);
    luaL_checkstack(L, n + 3, "too many arguments to script");
    for (i = 1; i <= n; i++)
        lua_rawgeti(L, -i, i);
    lua_remove(L, -i);
    return n;
}

static int handle_script(lua_State *L, char **argv) {
    int status;
    const char *fname = argv[0];
    status = luaL_loadfile(L, fname);
    if (status == LUA_OK) {
        int n = pushargs(L);
        status = docall(L, n, LUA_MULTRET);
    }
    return report(L, status);
}

static int pmain(lua_State *L) {
    int argc = (int)lua_tointeger(L, 1);
    char **argv = (char **)lua_touserdata(L, 2);
    int script;
    int args = collectargs(argv, &script);
    int optlim = (script > 0) ? script : argc;
    luaL_checkversion(L);
    if (args == has_error) {
        print_usage(argv[script]);
        return 0;
    }
    if (args & has_v)
        print_version();
    if (args & has_E) {
        lua_pushboolean(L, 1);
        lua_setfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
    }
    luaL_openlibs(L);
    createargtable(L, argv, argc, script);
    lua_gc(L, LUA_GCRESTART);
    lua_gc(L, LUA_GCGEN);
    if (!runargs(L, argv, optlim))
        return 0;
    if (script > 0) {
        if (handle_script(L, argv + script) != LUA_OK)
            return 0;
    }
    if (args & has_i)
        doREPL(L);
    else if (script < 1 && !(args & (has_e | has_v))) {
        print_version();
        doREPL(L);
    }
    lua_pushboolean(L, 1);
    return 1;
}

int main(int argc, char **argv) {
    int status, result;
    lua_State *L = luaL_newstate();
    if (L == NULL) {
        l_message(argv[0], "cannot create state: not enough memory");
        return 1;
    }
    lua_gc(L, LUA_GCSTOP);
    lua_pushcfunction(L, &pmain);
    lua_pushinteger(L, argc);
    lua_pushlightuserdata(L, argv);
    status = lua_pcall(L, 2, 1, 0);
    result = lua_toboolean(L, -1);
    report(L, status);
    lua_close(L);
    return (result && status == LUA_OK) ? 0 : 1;
}
