lua-memlimit
====

memory limit module.


## Installation

```sh
luarocks install memlimit
```


## Functions

### n = memlimit.used()

get the number of bytes of memory in used.

**Returns**

1. `n:number`: the number of bytes of memory in used.


### n = memlimit.minsize()

get the minimum number of bytes of memory that can be set to the maximum size.

**Returns**

1. `n:number`: the minimum number of bytes of memory.


### n [, ok] = memlimit.maxsize( [n] )

gets or sets the maximum number of bytes of available memory.

**Parameters**

- `n:number`: the maximum number of bytes of available memory. if set `n <= 0` to unlimited.

**Returns**

1. `n:number`: the maximum number of bytes of available memory. or, previouse value. 
2. `ok:boolean`: if `true`, the new maximum value has been set.


## Usage

the following code confirms that Lua will exit with a memory error.

```lua
local newstate = require('newstate')
-- you must install newstate module;
-- `luarocks install newstate`
local L = newstate.new()
local f = function()
    local memlimit = require('memlimit')
    local kb = 1024
    local s = ''

    memlimit.maxsize(memlimit.minsize() + (4 * kb))
    print('minsize:', memlimit.minsize())
    print('maxsize:', memlimit.maxsize())
    print('   used:', memlimit.used())
    for _ = 1, 100000 do
        -- allocate memory until the memory error occurs
        s = s .. '+'
    end
end
local ok, err, rc = L:dostring(string.dump(f))
-- rc == newstate.ERRMEM
print(ok, err, rc)
```
