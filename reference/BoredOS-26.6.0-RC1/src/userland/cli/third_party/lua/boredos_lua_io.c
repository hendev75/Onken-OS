/*
** BoredOS I/O library for Lua 5.5.0
** Replaces the standard liolib.c with BoredOS VFS syscall-based I/O.
*/

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

/* BoredOS file handle userdata */
typedef struct BOSFile {
    int fd;
    int closed;
} BOSFile;

#define BOREDOS_FILEHANDLE "BOSFILE*"

static int io_lines_handle(lua_State *L);
static int io_lines_module(lua_State *L);

static BOSFile *tobosfile(lua_State *L, int idx) {
    return (BOSFile *)luaL_checkudata(L, idx, BOREDOS_FILEHANDLE);
}

static BOSFile *checkbosfile(lua_State *L, int idx) {
    BOSFile *f = tobosfile(L, idx);
    if (f->closed)
        luaL_error(L, "attempt to use a closed file");
    return f;
}

static int io_close_file(lua_State *L) {
    BOSFile *f = tobosfile(L, 1);
    if (!f->closed) {
        sys_close(f->fd);
        f->closed = 1;
    }
    lua_pushboolean(L, 1);
    return 1;
}

static int io_read_file(lua_State *L) {
    BOSFile *f = checkbosfile(L, 1);
    int nargs = lua_gettop(L) - 1;

    if (nargs == 0) {
        /* Default: read a line */
        char buf[1024];
        int total = 0;
        luaL_Buffer b;
        luaL_buffinit(L, &b);
        while (1) {
            char c;
            int n = sys_read(f->fd, &c, 1);
            if (n <= 0) break;
            if (c == '\n') break;
            luaL_addchar(&b, c);
            total++;
        }
        if (total == 0 && sys_tell(f->fd) >= sys_size(f->fd)) {
            lua_pushnil(L);
            return 1;
        }
        luaL_pushresult(&b);
        return 1;
    }

    /* Handle format arguments */
    const char *fmt = luaL_checkstring(L, 2);
    if (fmt[0] == '*') fmt++;  /* skip optional '*' */

    if (fmt[0] == 'l' || fmt[0] == 'L') {
        /* Read a line */
        luaL_Buffer b;
        luaL_buffinit(L, &b);
        int total = 0;
        while (1) {
            char c;
            int n = sys_read(f->fd, &c, 1);
            if (n <= 0) break;
            if (c == '\n') {
                if (fmt[0] == 'L') luaL_addchar(&b, '\n');
                break;
            }
            luaL_addchar(&b, c);
            total++;
        }
        if (total == 0 && sys_tell(f->fd) >= sys_size(f->fd)) {
            lua_pushnil(L);
            return 1;
        }
        luaL_pushresult(&b);
        return 1;
    } else if (fmt[0] == 'n') {
        /* Read a number */
        char buf[64];
        int i = 0;
        while (i < 63) {
            char c;
            int n = sys_read(f->fd, &c, 1);
            if (n <= 0) break;
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                if (i > 0) break;
                continue;
            }
            buf[i++] = c;
        }
        buf[i] = '\0';
        if (i > 0) {
            char *end;
            double val = strtod(buf, &end);
            if (end != buf) {
                lua_pushnumber(L, (lua_Number)val);
                return 1;
            }
        }
        lua_pushnil(L);
        return 1;
    } else if (fmt[0] == 'a') {
        /* Read entire file */
        uint32_t cur = sys_tell(f->fd);
        uint32_t sz = sys_size(f->fd);
        uint32_t remaining = (sz > cur) ? sz - cur : 0;
        if (remaining == 0) {
            lua_pushliteral(L, "");
            return 1;
        }
        luaL_Buffer b;
        luaL_buffinit(L, &b);
        char chunk[512];
        while (remaining > 0) {
            int toread = remaining > 512 ? 512 : (int)remaining;
            int n = sys_read(f->fd, chunk, toread);
            if (n <= 0) break;
            luaL_addlstring(&b, chunk, (size_t)n);
            remaining -= (uint32_t)n;
        }
        luaL_pushresult(&b);
        return 1;
    }

    /* Numeric argument — read N bytes */
    if (lua_isnumber(L, 2)) {
        int count = (int)lua_tointeger(L, 2);
        if (count <= 0) {
            lua_pushliteral(L, "");
            return 1;
        }
        luaL_Buffer b;
        luaL_buffinit(L, &b);
        char chunk[512];
        int remaining = count;
        while (remaining > 0) {
            int toread = remaining > 512 ? 512 : remaining;
            int n = sys_read(f->fd, chunk, toread);
            if (n <= 0) break;
            luaL_addlstring(&b, chunk, (size_t)n);
            remaining -= n;
        }
        if (remaining == count) {
            lua_pushnil(L);
            return 1;
        }
        luaL_pushresult(&b);
        return 1;
    }

    lua_pushnil(L);
    return 1;
}

static int io_write_file(lua_State *L) {
    BOSFile *f = checkbosfile(L, 1);
    int nargs = lua_gettop(L);
    for (int i = 2; i <= nargs; i++) {
        size_t len;
        const char *s = luaL_checklstring(L, i, &len);
        sys_write_fs(f->fd, s, (uint32_t)len);
    }
    lua_pushvalue(L, 1);  /* return the file for chaining */
    return 1;
}

static int io_seek_file(lua_State *L) {
    BOSFile *f = checkbosfile(L, 1);
    static const char *const modenames[] = {"set", "cur", "end", NULL};
    int mode = luaL_checkoption(L, 2, "cur", modenames);
    int offset = (int)luaL_optinteger(L, 3, 0);
    int whence;
    switch (mode) {
        case 0: whence = 0; break;  /* SEEK_SET */
        case 1: whence = 1; break;  /* SEEK_CUR */
        case 2: whence = 2; break;  /* SEEK_END */
        default: whence = 0; break;
    }
    sys_seek(f->fd, offset, whence);
    lua_pushinteger(L, (lua_Integer)sys_tell(f->fd));
    return 1;
}

static int io_flush_file(lua_State *L) {
    checkbosfile(L, 1);
    lua_pushboolean(L, 1);
    return 1;
}

static int io_tostring(lua_State *L) {
    BOSFile *f = tobosfile(L, 1);
    if (f->closed)
        lua_pushliteral(L, "file (closed)");
    else {
        char buf[64];
        snprintf(buf, sizeof(buf), "file (%d)", f->fd);
        lua_pushstring(L, buf);
    }
    return 1;
}

static int io_gc(lua_State *L) {
    BOSFile *f = tobosfile(L, 1);
    if (!f->closed) {
        sys_close(f->fd);
        f->closed = 1;
    }
    return 0;
}

static const luaL_Reg file_methods[] = {
    {"read",   io_read_file},
    {"write",  io_write_file},
    {"seek",   io_seek_file},
    {"close",  io_close_file},
    {"lines",  io_lines_handle},
    {"flush",  io_flush_file},
    {"__tostring", io_tostring},
    {"__gc",   io_gc},
    {NULL, NULL}
};

/* === Module-level functions === */

static BOSFile *newbosfile(lua_State *L, int fd) {
    BOSFile *f = (BOSFile *)lua_newuserdatauv(L, sizeof(BOSFile), 0);
    f->fd = fd;
    f->closed = 0;
    luaL_setmetatable(L, BOREDOS_FILEHANDLE);
    return f;
}

static int io_open(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    const char *mode = luaL_optstring(L, 2, "r");
    int fd = sys_open(path, mode);
    if (fd < 0) {
        lua_pushnil(L);
        lua_pushfstring(L, "cannot open '%s'", path);
        return 2;
    }
    newbosfile(L, fd);
    return 1;
}

static int io_close(lua_State *L) {
    if (lua_isnone(L, 1))
        return 0;  /* no default output to close */
    return io_close_file(L);
}

static int io_type(lua_State *L) {
    void *ud = luaL_testudata(L, 1, BOREDOS_FILEHANDLE);
    if (ud == NULL)
        lua_pushnil(L);
    else if (((BOSFile *)ud)->closed)
        lua_pushliteral(L, "closed file");
    else
        lua_pushliteral(L, "file");
    return 1;
}

static int io_write(lua_State *L) {
    int nargs = lua_gettop(L);
    for (int i = 1; i <= nargs; i++) {
        size_t len;
        const char *s = luaL_tolstring(L, i, &len);
        sys_write(1, s, (int)len);
        lua_pop(L, 1);
    }
    /* Push stdout handle or true */
    lua_pushboolean(L, 1);
    return 1;
}

static int io_read_stdin(lua_State *L) {
    /* Read a line from stdin using BoredOS TTY */
    char buf[1024];
    int n = sys_tty_read_in(buf, sizeof(buf) - 1);
    if (n <= 0) {
        lua_pushnil(L);
        return 1;
    }
    /* Remove trailing newline */
    if (n > 0 && buf[n-1] == '\n') n--;
    lua_pushlstring(L, buf, (size_t)n);
    return 1;
}

static int io_lines_iter(lua_State *L) {
    BOSFile *f = (BOSFile *)lua_touserdata(L, lua_upvalueindex(1));
    if (f->closed) return 0;
    
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    int total = 0;
    while (1) {
        char c;
        int n = sys_read(f->fd, &c, 1);
        if (n <= 0) break;
        if (c == '\n') break;
        if (c != '\r') {
            luaL_addchar(&b, c);
            total++;
        }
    }
    
    if (total == 0 && sys_tell(f->fd) >= sys_size(f->fd)) {
        return 0; /* EOF */
    }
    
    luaL_pushresult(&b);
    return 1;
}

static int io_lines_handle(lua_State *L) {
    checkbosfile(L, 1);
    lua_pushvalue(L, 1);
    lua_pushcclosure(L, io_lines_iter, 1);
    return 1;
}

static int io_lines_module(lua_State *L) {
    if (lua_isstring(L, 1)) {
        const char *path = lua_tostring(L, 1);
        int fd = sys_open(path, "r");
        if (fd < 0) {
            luaL_error(L, "cannot open '%s'", path);
        }
        BOSFile *f = newbosfile(L, fd);
        lua_pushcclosure(L, io_lines_iter, 1);
        return 1;
    }
    luaL_error(L, "io.lines: path expected");
    return 0;
}

static int io_tmpfile(lua_State *L) {
    (void)L;
    lua_pushnil(L);
    lua_pushliteral(L, "tmpfile not supported on BoredOS");
    return 2;
}

static int io_input(lua_State *L) {
    (void)L;
    lua_pushnil(L);
    return 1;
}

static int io_output(lua_State *L) {
    (void)L;
    lua_pushnil(L);
    return 1;
}

static const luaL_Reg iolib[] = {
    {"open",    io_open},
    {"close",   io_close},
    {"type",    io_type},
    {"write",   io_write},
    {"read",    io_read_stdin},
    {"lines",   io_lines_module},
    {"tmpfile",  io_tmpfile},
    {"input",   io_input},
    {"output",  io_output},
    {"flush",   io_flush_file},
    {NULL, NULL}
};

static void createmeta(lua_State *L) {
    luaL_newmetatable(L, BOREDOS_FILEHANDLE);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, file_methods, 0);
    lua_pop(L, 1);
}

LUAMOD_API int luaopen_io(lua_State *L) {
    createmeta(L);
    luaL_newlib(L, iolib);
    return 1;
}
