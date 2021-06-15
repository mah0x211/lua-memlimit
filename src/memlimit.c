/**
 *  Copyright (C) 2021 Masatoshi Fukunaga
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 */

#include <lauxlib.h>

#define MODULE_MT "memlimit"

typedef struct {
    lua_State *L;
    int ref;
    size_t used;
    size_t minsize;
    size_t maxsize;
    lua_Alloc fn;
    void *ud;
} memlimit_t;

static int used_lua(lua_State *L) {
    memlimit_t *m = NULL;

    lua_getallocf(L, (void *)&m);
    lua_pushinteger(L, (lua_Integer)m->used);

    return 1;
}

static int minsize_lua(lua_State *L) {
    memlimit_t *m = NULL;

    lua_getallocf(L, (void *)&m);
    lua_pushinteger(L, (lua_Integer)m->minsize);

    return 1;
}

static int maxsize_lua(lua_State *L) {
    int narg = lua_gettop(L);
    memlimit_t *m = NULL;

    lua_getallocf(L, (void *)&m);
    // push current maxsize
    lua_pushinteger(L, (lua_Integer)m->maxsize);

    // set new maxsize
    if (narg) {
        lua_Integer n = luaL_checkinteger(L, 1);
        int ok = n <= 0 || n >= m->minsize;

        if (n <= 0) {
            // no limit
            m->maxsize = 0;
        } else if (ok) {
            m->maxsize = (size_t)n;
        }

        lua_pushboolean(L, ok);
        return 2;
    }

    return 1;
}

static void *alloc_lua(void *ud, void *ptr, size_t osize, size_t nsize) {
    memlimit_t *m = (memlimit_t *)ud;
    size_t newused = m->used - osize;

    // set 0 if underflow
    if (newused > m->used) {
        newused = 0;
    }

    // free osize
    if (nsize == 0) {
        m->used = newused;
        return m->fn(m->ud, ptr, osize, nsize);
    }

    // alloc nsize
    newused += nsize;
    if (m->maxsize == 0 || newused <= m->maxsize) {
        void *blk = m->fn(m->ud, ptr, osize, nsize);
        if (blk) {
            m->used = newused;
        }
        return blk;
    }

    // cannot alloc nsize
    return NULL;
}

static int gc_lua(lua_State *L) {
    memlimit_t *m = (memlimit_t *)lua_touserdata(L, 1);
    lua_setallocf(m->L, m->fn, m->ud);
    luaL_unref(m->L, LUA_REGISTRYINDEX, m->ref);
    return 0;
}

LUALIB_API int luaopen_memlimit(lua_State *L) {
    struct luaL_Reg fns[] = {{"used", used_lua},
                             {"minsize", minsize_lua},
                             {"maxsize", maxsize_lua},
                             {NULL, NULL}};
    struct luaL_Reg *fn = fns;
    memlimit_t *m = lua_newuserdata(L, sizeof(memlimit_t));

    // keep the target state
    m->L = L;
    // create and set metatable
    luaL_newmetatable(L, MODULE_MT);
    lua_pushstring(L, "__gc");
    lua_pushcfunction(L, gc_lua);
    lua_rawset(L, -3);
    lua_setmetatable(L, -2);
    // keep m until the target state is closed
    m->ref = luaL_ref(L, LUA_REGISTRYINDEX);

    // add funcs
    lua_newtable(L);
    while (fn->name) {
        lua_pushstring(L, fn->name);
        lua_pushcfunction(L, fn->func);
        lua_rawset(L, -3);
        fn++;
    }

    // set as custom allocater
    m->used = lua_gc(L, LUA_GCCOUNT, 0) * 1024 + lua_gc(L, LUA_GCCOUNTB, 0);
    m->minsize = m->used;
    m->maxsize = 0;
    m->fn = lua_getallocf(L, &m->ud);
    lua_setallocf(L, alloc_lua, (void *)m);

    return 1;
}
