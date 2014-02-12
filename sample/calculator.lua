local function char(s)
    return string.byte(s)
end

local _0 = char('0')
local _9 = char('9')
local space = char(' ')
local tab = char('\t')
local add = char('+')
local minus = char('-')
local mul = char('*')
local div = char('/')
local left_par = char('(')
local right_par = char(')')
local eof = -1

local token_type_eof = -1
local token_type_error = 0
local token_type_op = 1
local token_type_num = 2
local token_eof = { type = token_type_eof }
local error_exp = "error expression"

local function get_lexer(str)
    local i = 1
    local len = #str

    local function next_char()
        if i <= len then
            local c = string.byte(str, i)
            i = i + 1
            return c
        else
            return eof
        end
    end

    local function look_ahead()
        if i <= len then
            return string.byte(str, i)
        else
            return eof
        end
    end

    local function is_digit(c)
        return c >= _0 and c <= _9
    end

    local function is_space(c)
        return c == space or c == tab
    end

    local function is_operator(c)
        return c == add or c == minus or c == mul or
            c == div or c == left_par or c == right_par
    end

    local function lex_number(c)
        local num = c - _0
        while true do
            if is_digit(look_ahead()) then
                num = num * 10 + next_char() - _0
            else
                return num
            end
        end
    end

    return function()
        while true do
            local c = next_char()
            if c == eof then
                return token_eof
            end

            if not is_space(c) then
                if is_digit(c) then
                    return { type = token_type_num, value = lex_number(c) }
                elseif is_operator(c) then
                    return { type = token_type_op, value = c }
                else
                    return { type = token_type_error, error = i .. ": unknown char" }
                end
            end
        end
    end
end

local function get_parser(lexer)
    local look_ahead = token_eof

    local function next()
        if look_ahead.type == token_type_eof then
            return lexer()
        else
            local token = look_ahead
            look_ahead = token_eof
            return token
        end
    end

    local function peek()
        if look_ahead.type == token_type_eof then
            look_ahead = lexer()
        end
        return look_ahead
    end

    local parser = { next = next, look_ahead = peek }

    function parser:factor()
        local neg = false
        if self.look_ahead().type == token_type_op and
            self.look_ahead().value == minus then
            self.next()
            neg = true
        end

        local token = self.next()
        if token.type == token_type_num then
            return neg and -token.value or token.value
        elseif token.type == token_type_op and token.value == left_par then
            local temp = self:parse_exp()
            if type(temp) == "string" then
                return temp
            end

            token = self.next()
            if not (token.type == token_type_op and token.value == right_par) then
                return error_exp
            end
            return neg and -temp or temp
        else
            return error_exp
        end
    end

    function parser:mul_div()
        local result = self:factor()
        if type(result) == "string" then
            return result
        end

        while true do
            if self.look_ahead().type == token_type_op then
                if self.look_ahead().value == mul then
                    self.next()
                    local temp = self:factor()
                    if type(temp) == "string" then return temp end
                    result = result * temp
                elseif self.look_ahead().value == div then
                    self.next()
                    local temp = self:factor()
                    if type(temp) == "string" then return temp end
                    result = result / temp
                else
                    return result
                end
            else
                return result
            end
        end
    end

    function parser:add_minus()
        local result = self:mul_div()
        if type(result) == "string" then
            return result
        end

        while true do
            if self.look_ahead().type == token_type_eof then
                return result
            elseif self.look_ahead().type == token_type_op then
                if self.look_ahead().value == add then
                    self.next()
                    local temp = self:mul_div()
                    if type(temp) == "string" then return temp end
                    result = result + temp
                elseif self.look_ahead().value == minus then
                    self.next()
                    local temp = self:mul_div()
                    if type(temp) == "string" then return temp end
                    result = result - temp
                elseif self.look_ahead().value == right_par then
                    return result
                else
                    return error_exp
                end
            elseif self.look_ahead().type == token_type_error then
                return self.look_ahead().error
            else
                return error_exp
            end
        end
    end

    function parser:parse_exp()
        return self:add_minus()
    end

    function parser:parse()
        local result = self:parse_exp()
        if type(result) == "string" then
            return result
        end

        if self.look_ahead().type ~= token_type_eof then
            return error_exp
        end
        return result
    end

    return parser
end

while true do
    print("calculator >")
    local line = getline()
    local parser = get_parser(get_lexer(line))
    if parser.look_ahead().type ~= token_type_eof then
        print(parser:parse())
    end
end
