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
end

fun(str1, str2, str, tb);
