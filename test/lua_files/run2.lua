function get_multiply_times()
    return 2, 1
end

local a = 10 ^ 3.2
print(a - 10 ^ 3.2)

a = a * 2 * get_multiply_times()
print(a)

a = a / 2
print(a)

print(1 / 0)

print(1.15 % 1.2)
print(1 % 0)

a = a + a
print(a)

a = a - 10 ^ 3.2
print(a)

print(1 .. 1.1)
print(1 .. "str")
print("str" .. 1)
print("str1" .. "str2")

print("str1" > "str2")
print("str1" < "str2")
print("str1" >= "str2")
print("str1" <= "str2")
print(1 > 0)
print(1 < 0)
print(1 >= 0)
print(1 <= 0)
print(1 == "str", 1 == nil, 1 == true, 1 == false, true == false)
print(1 ~= "str", 1 ~= nil, 1 ~= true, 1 ~= false, true ~= false)

b = 0
function ins_b()
    b = b + 1
    return b
end

print(0 and ins_b())
print(b)

print(nil and ins_b())
print(b)

print(false and ins_b())
print(b)

print(0 or ins_b())
print(b)

print(nil or ins_b())
print(b)

print(false or ins_b())
print(b)

print(-b)
print(not b)
print(#"str")