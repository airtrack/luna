a, b, c, d, e, f = 1, "test", true, false, nil
print(a, b, c, d, e, f)

function foo()
    print("in foo:", a, b, c, d, e, f)
    a, b, c, d, e, f = 2, "new", false, true
end

foo(1, 2, 3)
print(a, b, c, d, e, f)