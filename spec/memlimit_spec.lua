local newstate = require('newstate')
local memlimit = require('memlimit')

describe('test memlimit', function()
    it('returns a func table', function()
        assert.is_equal('table', type(memlimit))
    end)

    it('returns a greater than 0', function()
        assert.is_true(memlimit.used() > 0)
        assert.is_true(memlimit.minsize() > 0)
    end)

    it('returns a 0 and nil', function()
        local n, ok = memlimit.maxsize()
        assert.is_equal(0, n)
        assert.is_nil(ok)
    end)

    it('returns a previous maxsize and true', function()
        local mb = 1024 * 1024
        local oldv = memlimit.maxsize()
        local newv = memlimit.minsize() + (10 * mb)
        local retv, ok = memlimit.maxsize(newv)
        assert.is_equal(oldv, retv)
        assert.is_true(ok)
        assert.is_equal(newv, memlimit.maxsize())
    end)

    it('cannot set a maxsize smaller then minsize', function()
        local oldv = memlimit.maxsize()
        local retv, ok = memlimit.maxsize(memlimit.minsize() - 1)
        assert.is_equal(oldv, retv)
        assert.is_false(ok)
        assert.is_equal(oldv, memlimit.maxsize())
    end)

    it('returns a ERRMEM error', function()
        local L = newstate.new()
        local f = function()
            local memlimit = require('memlimit')
            local kb = 1024
            local s = ''

            memlimit.maxsize(memlimit.minsize() + (4 * kb))
            for _ = 1, 100000 do
                s = s .. '+'
            end
        end
        local ok, _, rc = L:dostring(string.dump(f))
        assert.is_false(ok)
        assert.is_equal(newstate.ERRMEM, rc)
    end)
end)
