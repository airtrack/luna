Luna
====
A interpreter of lua-like language written in C++ 11.

Build
-----

	cmake -G "Unix Makefiles"

or

	cmake -G Xcode

API
---

Function|Description
--------|----
print(...)|print values to stdout
puts(string)|print a *string* to stdout
ipairs(table)|returns a iterator to iterate array part of a *table*
pairs(table)|returns a iterator to iterate a *table*(array and hash)
type(value)|returns type of a *value*
getline()|returns a line string which gets from stdin
io.open(path [, mode])|returns a file of *path* by *mode* when open success, otherwise returns nil and error description, *mode* is same with c function *fopen*, default is "r".
io.stdin()|returns a file of stdin
io.stdout()|returns a file of stdout
io.stderr()|returns a file of stderr
file:close()|close file
file:flush()|flush write buffer
file:read(...)|read data from file, arguments could be number(read number bytes, returns as a string), "\*n"(read a number and returns the number), "\*a"(read whole file, returns as a string. returns a empty string when on the end of file), "\*l"(read a line, returns as a string without '\\n'), "*L"(read a line, returns as a string with '\\n'). returns nil when on end of file.
file:seek([whence [, offset]])|sets and gets the file position. *whence* could be "set", "cur", "end", *offset* is a number. If seek success, then returns the file position, otherwise returns nil and error description. Called with no argument, returns current position.
file:setvbuf(mode [, size])|set the buffering mode for the output file. *mode* could be "no"(no buffering), "full"(full buffering), "line"(line buffering), *size* is a number specifies the size of the buffer, in bytes.
file:write(...)|write the value of each argument to file, arguments could be string and number. If success, returns the file, otherwise returns nil and error description.
math.abs(x)|same with c function *abs*
math.acos(x)|same with c function *acos*
math.asin(x)|same with c function *asin*
math.atan(x)|same with c function *atan*
math.atan2(y, x)|same with c function *atan2*
math.ceil(x)|same with c function ceil
math.cos(x)|same with c function *cos*
math.cosh(x)|same with c function *cosh*
math.deg(x)|returns the angle *x*(given in radians) in degrees
math.exp(x)|same with c function *exp*
math.floor(x)|same with c function *floor*
math.frexp(x)|same with c function *frexp*, returns significand of the given number in range of [0.5, 1) and exponent.
math.huge|the c macro HUGE_VAL
math.ldexp(m, e)|same with c function *ldexp*
math.log(x [, base])|same with c function *log*, the default for *base* is *e*.
math.max(x, ...)|returns the maximum value
math.min(x, ...)|returns the minimum value
math.modf(x)|returns the integral part of *x* and the fractional part of *x*.
math.pi|the value of PI
math.pow(x, y)|same with c function *pow*
math.rad(x)|returns the angle(given in degrees) in radians.
math.random([m [, n]])|returns a uniform pseudo-random number. When called with no arguments, returns a real number in the range [0, 1). When called with number *m*, returns a integer in the range [1, m]. When called with number *m* and *n*, returns a integer in the range [m, n].
math.randomseed(x)|set *x* as the seed for the pseudo-random generator.
math.sin(x)|same with c function *sin*
math.sinh(x)|same with c function *sinh*
math.sqrt(x)|same with c function *sqrt*
math.tan(x)|same with c function *tan*
math.tanh(x)|same with c function *tanh*
string.byte(s [, i [, j]])| returns the numerical codes of the characters s[*i*] to s[*j*]. The default value for *i* is 1.
string.char(...)|returns a string with length equal to the number of arguments, in which each character has the numerical code equal to its corresponding argument.
string.len(s)|returns the length of the string *s*.
string.lower(s)|returns a string in which each character is lowercase.
string.upper(s)|returns a string in which each character is uppercase.
string.reverse(s)|returns a reverse string of the string *s*.
string.sub(s, i [, j])|returns the substring of *s*[*i*..*j*].
