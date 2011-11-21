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

print("hello world")

local is_true = true
if is_true then
    is_true = false
end

tb[3] = {1, 2, 3, { "1", "2", "3" }}

for i = 1, #tb[3] do
    print(tb[3][i])
end

local i = 1
while tb[i] do
    print(tb[i])
    i = i + 1
end

repeat
    line = io.read()
    if line == "break" then
        break
    end
until line ~= ""
