for i = 1, 5 do
    print(i)
end

for i = 5, 1, -1 do
    print(i)
end

local function iter(a, i)
    i = i + 1
    local v = a[i]
    if v then
        return i, v
    end
end

local function ipairs(a)
    return iter, a, 0
end

local tb = { "str1", "str2", "str3", "str4", "str5" }

for i, v in ipairs(tb) do
    print(i, v)
end