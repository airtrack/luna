function f()
    local f1, f2
    local up
    do
        f1 = function () return function() print(up) end end
        local up = "local up"
        f2 = function () return function() print(up) end end
    end
    up = "f up"
    return f1, f2
end

f1, f2 = f()

cf1 = f1()
cf2 = f2()

cf1()
cf2()

cf1 = f1()
cf2 = f2()

cf1()
cf2()