debugger = {}
debugger.funcbreakpoints = {}
debugger.linebreakpoints = {}
debugger.luacode = {}
debugger.command = {}
debugger.idgenerator = 0

debugger.generatebreakpointid = function()
    debugger.idgenerator = debugger.idgenerator + 1
    return debugger.idgenerator
end

debugger.addfuncbreak = function(fun)
    assert(type(fun) == "function", "must be a function")

    local id = debugger.generatebreakpointid()
    debugger.funcbreakpoints[fun] = id
    debugger.setnormalmode()

    print(string.format("id: %d, function break point added: ", id), fun)
end

debugger.delfuncbreak = function(fun)
    assert(type(fun) == "function", "must be a function")

    if debugger.funcbreakpoints[fun] then
        local id = debugger.funcbreakpoints[fun]
        debugger.funcbreakpoints[fun] = nil
        print(string.format("id: %d, function break point deleted", id), fun)
    else
        print("no break point:", fun)
    end
end

debugger.printfuncbreak = function()
    for fun, id in pairs(debugger.funcbreakpoints) do
        print(string.format("id: %d ", id), fun)
    end
end

debugger.addlinebreak = function(file, line)
    if not debugger.linebreakpoints[file] then
        debugger.linebreakpoints[file] = {}
    end

    local id = debugger.generatebreakpointid()
    debugger.linebreakpoints[file][line] = id
    debugger.setnormalmode()

    print(string.format("id: %d, line break point [%s]:%d added", id, string.sub(file, 2), line))
end

debugger.dellinebreak = function(file, line)
    if debugger.linebreakpoints[file] and debugger.linebreakpoints[file][line] then
        local id = debugger.linebreakpoints[file][line]
        debugger.linebreakpoints[file][line] = nil
        print(string.format("id: %d, line break point [%s]:%d deleted", id, file, line))
        return
    end

    print(string.format("no break point: [%s]:%d", file, line))
end

debugger.printlinebreak = function()
    for file, lines in pairs(debugger.linebreakpoints) do
        for line, id in pairs(lines) do
            print(string.format("id: %d [%s]:%d", id, file, line))
        end
    end
end

debugger.clearbreakpoint = function()
    debugger.funcbreakpoints = {}
    debugger.linebreakpoints = {}
end

debugger.loadluacode = function(filename)
    local file, errormsg = io.open(filename)
    if not file then
        print(errormsg)
        return
    end

    local code = {}
    for line in file:lines() do
        table.insert(code, line)
    end

    debugger.luacode[filename] = code
    file:close()
end

debugger.printluacode = function(file, line, ll)
    if not debugger.luacode[file] then
        debugger.loadluacode(file)
    end

    local code = debugger.luacode[file]
    if not code then
        return
    end

    local beginline = (line - ll) < 1 and 1 or (line - ll)
    local endline = (line + ll) > #code and #code or (line + ll)

    for i = beginline, endline do
        print(i, code[i])
    end
end

debugger.setnormalmode = function()
    debug.sethook(debugger.normalmodehook, "cl")
end

debugger.setstepinmode = function()
    debug.sethook(debugger.stepinmodehook, "l")
end

debugger.setstepovermode = function()
    debug.sethook(debugger.stepovermodehook, "crl")
end

debugger.setnextlinemode = function()
    debug.sethook(debugger.nextlinemodehook, "crl")
end

debugger.checkbreakline = function(stacklevel, onbreak)
    local info = debug.getinfo(stacklevel, "Sl")
    for file, lines in pairs(debugger.linebreakpoints) do
        if file == info.source then
            for line, _ in pairs(lines) do
                if line == info.currentline then
                    if onbreak then onbreak() end
                    debugger.breakthepoint(stacklevel + 1)
                end
            end
        end
    end
end

debugger.checkbreakfunc = function(stacklevel, onbreak)
    local info = debug.getinfo(stacklevel, "f")
    for fun, _ in pairs(debugger.funcbreakpoints) do
        if fun == info.func then
            if onbreak then onbreak() end
            -- enter step in mode
            debugger.setstepinmode()
        end
    end
end

debugger.normalmodehook = function(event, line)
    if event == "line" then
        debugger.checkbreakline(3)
    elseif event == "call" then
        debugger.checkbreakfunc(3)
    end
end

debugger.stepinmodehook = function(event, line)
    debugger.breakthepoint(3)
end

debugger.stepovermodehook = function(event, line)
    local onbreak = function()
        debugger.stepoverbreak = nil
        debugger.callfunctimes = nil
    end

    if event == "line" then
        debugger.checkbreakline(3, onbreak)

        if debugger.stepoverbreak and debugger.callfunctimes == 0 then
            onbreak()
            debugger.breakthepoint(3)
        end
    elseif event == "call" then
        debugger.checkbreakfunc(3, onbreak)

        if debugger.stepoverbreak then
            debugger.callfunctimes = debugger.callfunctimes + 1
        end
    elseif event == "return" then
        if debugger.stepoverbreak then
            debugger.callfunctimes = debugger.callfunctimes - 1
        end
    end
end

debugger.nextlinemodehook = function(event, line)
    local onbreak = function()
        debugger.nextlinebreak = nil
        debugger.callfunctimes = nil
    end

    if event == "line" then
        debugger.checkbreakline(3, onbreak)

        if debugger.nextlinebreak and debugger.callfunctimes <= 1 then
            onbreak()
            debugger.breakthepoint(3)
        end
    elseif event == "call" then
        debugger.checkbreakfunc(3, onbreak)

        if debugger.nextlinebreak then
            debugger.callfunctimes = debugger.callfunctimes + 1
        end
    elseif event == "return" then
        if debugger.nextlinebreak then
            debugger.callfunctimes = debugger.callfunctimes - 1
        end
    end
end

debugger.breakthepoint = function(stacklevel, notprintcode)
    if not notprintcode then
        -- print currentline code
        local info = debug.getinfo(stacklevel, "Sl")
        debugger.printluacode(string.sub(info.source, 2), info.currentline, 0)
    end

    -- get command input
    print("debugger >")
    local l = io.read("*l")
    local command = debugger.parsecommand(l)
    debugger.execute(command, stacklevel + 1)
end

debugger.parsecommand = function(commandline)
    local t = {}
    for w in string.gmatch(commandline, "[^ ]+") do
        table.insert(t, w)
    end

    return t
end

debugger.execute = function(command, stacklevel)
    local executor = debugger.command[command[1]]
    if not executor then
        debugger.errorcmd()
        debugger.breakthepoint(stacklevel + 1)
    else
        executor(command, stacklevel + 1)
    end
end

debugger.breakline = function(command, stacklevel)
    local iscurrentfile = tonumber(command[2]) and true or false
    for i = 2, #command do
        local errorcmd = false
        if iscurrentfile and not tonumber(command[i]) then
            errorcmd = true
        end

        if not iscurrentfile and math.fmod(i, 2) == 1 and not tonumber(command[i]) then
            errorcmd = true
        end

        if errorcmd then
            return debugger.errorcmd()
        end
    end

    if iscurrentfile then
        local info = debug.getinfo(stacklevel, "S")
        for i = 2, #command do
            local line = tonumber(command[i])
            debugger.addlinebreak(info.source, line)
        end
    else
        for i = 2, #command, 2 do
            local file = "@" .. command[i]
            local line = tonumber(command[i + 1])
            debugger.addlinebreak(file, line)
        end
    end
end

debugger.breakfunc = function(command, stacklevel)
    for i = 2, #command do
        local chunk = loadstring(string.format("return %s", command[i]))
        local func = chunk and chunk() or nil

        if type(func) == "function" then
            debugger.addfuncbreak(func)
            print(string.format("break function: %s", command[i]))
        else
            print(string.format("no function %s, can't break it", command[i]))
        end
    end
end

debugger.breakpoint = function(command, stacklevel)
    if #command < 2 then
        debugger.errorcmd()
        return debugger.breakthepoint(stacklevel + 1)
    end

    local isbreakfunc = tonumber(command[2]) and 0 or 1
    isbreakfunc = (command[3] and tonumber(command[3])) and 0 or isbreakfunc

    if isbreakfunc == 1 then
        debugger.breakfunc(command, stacklevel + 1)
    else
        debugger.breakline(command, stacklevel + 1)
    end

    debugger.breakthepoint(stacklevel + 1, true)
end

debugger.continue = function(command, stacklevel)
    debugger.setnormalmode()
end

debugger.nextline = function(command, stacklevel)
    debugger.nextlinebreak = true
    debugger.callfunctimes = 1
    debugger.setnextlinemode()
end

debugger.stepin = function(command, stacklevel)
    debugger.setstepinmode()
end

debugger.stepover = function(command, stacklevel)
    debugger.stepoverbreak = true
    debugger.callfunctimes = 1
    debugger.setstepovermode()
end

debugger.printline = function(command, stacklevel)
    local info = debug.getinfo(stacklevel, "Sl")
    local lines = tonumber(command[2]) or 2
    debugger.printluacode(string.sub(info.source, 2), info.currentline, lines)

    debugger.breakthepoint(stacklevel + 1, true)
end

debugger.getlocalvaluetable = function(stacklevel)
    local j = 1
    local t = {}
    while true do
        local n, v = debug.getlocal(stacklevel, j)
        if not n then break end

        t[n] = v
        j = j + 1
    end

    return t
end

debugger.setlocalvaluetable = function(stacklevel, newlocals)
    local j = 1
    while true do
        local n, v = debug.getlocal(stacklevel, j)
        if not n then break end

        v = newlocals[n]
        assert(debug.setlocal(stacklevel, j, v) == n)
        j = j + 1
    end
end

debugger.getupvaluetable = function(stacklevel)
    local f = debug.getinfo(stacklevel, "f").func
    local j = 1
    local t = {}
    while true do
        local n, v = debug.getupvalue(f, j)
        if not n then break end

        t[n] = v
        j = j + 1
    end

    return t
end

debugger.setupvaluetable = function(stacklevel, newupvalues)
    local f = debug.getinfo(stacklevel, "f").func
    local j = 1
    while true do
        local n, v = debug.getupvalue(f, j)
        if not n then break end

        v = newupvalues[n]
        assert(debug.setupvalue(f, j, v) == n)
        j = j + 1
    end
end

debugger.getfuncenvtable = function(stacklevel)
    local fenv = getfenv(stacklevel)
    local upvaluetable = debugger.getupvaluetable(stacklevel + 1)
    setmetatable(upvaluetable, { __index = function(t, k) return fenv[k] end })

    local localvaluetable = debugger.getlocalvaluetable(stacklevel + 1)
    setmetatable(localvaluetable, { __index = function(t, k) return upvaluetable[k] end,
                                    __newindex = function() assert(false) end })
    return localvaluetable, upvaluetable, fenv
end

debugger.printvar = function(command, stacklevel)
    local getvalue = function(name, stacklevel)
        local chunk = loadstring(string.format("return %s", name))
        if chunk then
            setfenv(chunk, debugger.getfuncenvtable(stacklevel + 1))
            local value = { pcall(chunk) }
            if value[1] then
                table.remove(value, 1)
                if #value ~= 0 then
                    return name, value
                end
            end
        end

        return name, nil
    end

    for i = 2, #command do
        local n, v = getvalue(command[i], stacklevel + 1)
        if type(v) == "table" then
            print(n, unpack(v))
        else
            print(n, v)
        end
    end

    debugger.breakthepoint(stacklevel + 1, true)
end

debugger.traceback = function(command, stacklevel)
    local level = stacklevel
    while true do
        local info = debug.getinfo(level, "Sl")
        if not info then break end

        print(string.format("[%s]:%d", info.short_src, info.currentline))
        level = level + 1
    end

    debugger.breakthepoint(stacklevel + 1, true)
end

debugger.printbreakpoint = function(command, stacklevel)
    debugger.printfuncbreak()
    debugger.printlinebreak()
    debugger.breakthepoint(stacklevel + 1, true)
end

debugger.clearbreak = function(command, stacklevel)
    debugger.clearbreakpoint()
    print("clear all break points ok")
    debugger.breakthepoint(stacklevel + 1, true)
end

debugger.deletebreak = function(command, stacklevel)
    local getfuncbreakpoint = function(breakpointid)
        for func, id in pairs(debugger.funcbreakpoints) do
            if id == breakpointid then
                return func
            end
        end
    end

    local getlinebreakpoint = function(breakpointid)
        for file, lines in pairs(debugger.linebreakpoints) do
            for line, id in pairs(lines) do
                if id == breakpointid then
                    return file, line
                end
            end
        end
    end

    for i = 2, #command do
        local id = tonumber(command[i])
        if id then
            local func = getfuncbreakpoint(id)
            if func then
                debugger.delfuncbreak(func)
            else
                local file, line = getlinebreakpoint(id)
                if file and line then
                    debugger.dellinebreak(file, line)
                end
            end
        end
    end

    debugger.breakthepoint(stacklevel + 1, true)
end

debugger.dosetvalue = function(command, stacklevel)
    local setcommand = string.format("%s = %s", command[2], command[3])
    local chunk = loadstring(setcommand)

    if chunk then
        local localvaluetable, upvaluetable = debugger.getfuncenvtable(stacklevel + 1)
        setfenv(chunk, localvaluetable)
        if pcall(chunk) then
            debugger.setlocalvaluetable(stacklevel + 1, localvaluetable)
            debugger.setupvaluetable(stacklevel + 1, upvaluetable)
            return setcommand .. " ok"
        end
    end

    return setcommand .. " failed"
end

debugger.setvalue = function(command, stacklevel)
    if #command < 3 then
        debugger.errorcmd()
        return debugger.breakthepoint(stacklevel + 1)
    end

    local msg = debugger.dosetvalue(command, stacklevel + 1)
    print(msg)
    debugger.breakthepoint(stacklevel + 1, true)
end

debugger.help = function(command, stacklevel)
    local str = [[
        b   --   break line. eg. b file line ...
                 current file: eg. b line line ...
                 function: eg. b func func ...
        c   --   continue
        n   --   next line
        s   --   step in
        o   --   step over
        l   --   print context lua code. eg. l [lines]
        p   --   print var value. eg. p var1 var2 ...
        t   --   traceback
        pb  --   print all break points
        cb  --   clear all break points
        db  --   delete break point. eg. db id1 id2 ...
        set --   set value. eg. set var value
        h   --   for help
    ]]

    print(str)
    debugger.breakthepoint(stacklevel + 1, true)
end

debugger.errorcmd = function()
    print("error command, h for help")
end

debugger.command["b"] = debugger.breakpoint
debugger.command["c"] = debugger.continue
debugger.command["n"] = debugger.nextline
debugger.command["s"] = debugger.stepin
debugger.command["o"] = debugger.stepover
debugger.command["l"] = debugger.printline
debugger.command["p"] = debugger.printvar
debugger.command["t"] = debugger.traceback
debugger.command["pb"] = debugger.printbreakpoint
debugger.command["cb"] = debugger.clearbreak
debugger.command["db"] = debugger.deletebreak
debugger.command["set"] = debugger.setvalue
debugger.command["h"] = debugger.help
