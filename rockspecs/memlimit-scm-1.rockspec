package = "memlimit"
version = "scm-1"
source = {
    url = "git://github.com/mah0x211/lua-memlimit.git"
}
description = {
    summary = "memory limit module.",
    homepage = "https://github.com/mah0x211/lua-memlimit",
    license = "MIT/X11",
    maintainer = "Masatoshi Fukunaga"
}
dependencies = {
    "lua >= 5.1",
}
build = {
    type = "builtin",
    modules = {
        memlimit = {
            sources = { "src/memlimit.c" },
        },
    }
}
