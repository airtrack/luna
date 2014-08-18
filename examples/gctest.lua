local chars = {}

local set_chars = function(first, last)
    for c = first, last do
        chars[#chars + 1] = c
    end
end

set_chars(string.byte("a"), string.byte("z"))
set_chars(string.byte("A"), string.byte("Z"))
set_chars(string.byte("0"), string.byte("9"))

local chars_len = #chars

while true do
    local str = ""
    local len = math.random(3, 200)
    for i = 1, len do
        str = str .. string.char(chars[math.random(chars_len)])
    end
end
