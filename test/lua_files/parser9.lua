local fun = function(abc, ...)
	print(abc, ...)
end

local abc, def = 123, fun(123)