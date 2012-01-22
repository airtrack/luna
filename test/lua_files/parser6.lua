if i > -1e6 then
   i = 1
elseif j then
	j = "str"
else
	k = true
end

if not i then
    i = 1
else
    i = 2
end

if not i and j then
    j = 1
end

if (i < 3) and (i > 1) then
    print(i)
end
