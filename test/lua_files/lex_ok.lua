-- this is comment
--[[
this is long comment
so there are multi-lines
and we test --]
--]]

local str1 = "abc"
local str2 = 'abc'
local str = [[
this is long string,
so there are multi-lines.
]]

local tb = {}
tb[1] = 123.456
tb[2] = str
for _, v in ipairs(tb) do
    print(v)
end

function fun(...)
    return { unpack(arg) }
end

fun(str1, str2, str, tb);

print(tb[1] .. tb[2])

tb.count = #tb

tb[3] = {1, 2, 3, { "1", "2", "3" }}

local v = nil
v = true
v = false

i = 0x1234;
i = 0X5678;
i = 0.1
i = .123e+10
i = .456E-10
i = 1.e+10
i = 1.E+10+1
i = 1.-1

local _, _ = 1, i
i = i ^ 2 + 1 * (2 - 1 * (1 + 1))
i = i / 1 % 100
i = -i
i = not i
