local t = { [-1] = nil, ["m"] = 1, 1, 2, n = "str", 3, 4 }

print(t)
print(t.m, t.n)
print(t["m"], t["n"])
print(#t)

print(t[1], t[2], t[3], t[4])
t[1] = t[4]
t[2] = t[3]
print(t[1], t[2], t[3], t[4])

print(t[-1], t[0])
t[-1] = -1
t[0] = "zero"
print(#t)
print(t[-1], t[0])

t[10] = 10
print(#t)
print(t[10])

print({})