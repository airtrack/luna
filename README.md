Linux & OSX: ![Build with clang](https://travis-ci.org/airtrack/luna.svg)

Luna
====
An interpreter of lua-like language written in C++ 11.

Build
-----

	cmake -G "Unix Makefiles"

or

	cmake -G Xcode

API
---

Global function|Description
---------------|-----------
print(...)|Print values to stdout
puts(string)|Print a *string* to stdout
ipairs(table)|Returns a iterator to iterate array part of a *table*
pairs(table)|Returns a iterator to iterate a *table*(array and hash)
type(value)|Returns type of a *value*
getline()|Returns a line string which gets from stdin
require(path)|Load the *path* module

IO table|Description
--------|-----------
io.open(path [, mode])|Returns a file of *path* by *mode* when open success, otherwise returns nil and error description, *mode* is same with c function *fopen*, default is "r".
io.stdin()|Returns a file of stdin
io.stdout()|Returns a file of stdout
io.stderr()|Returns a file of stderr

File table|Description
----------|-----------
file:close()|Close file
file:flush()|Flush write buffer
file:read(...)|Read data from file, arguments could be number(read number bytes, returns as a string), "\*n"(read a number and returns the number), "\*a"(read whole file, returns as a string. Returns a empty string when on the end of file), "\*l"(read a line, returns as a string without '\\n'), "*L"(read a line, returns as a string with '\\n'). Returns nil when on end of file.
file:seek([whence [, offset]])|Sets and gets the file position. *whence* could be "set", "cur", "end", *offset* is a number. If seek success, then returns the file position, otherwise returns nil and error description. Called with no argument, returns current position.
file:setvbuf(mode [, size])|Set the buffering mode for the output file. *mode* could be "no"(no buffering), "full"(full buffering), "line"(line buffering), *size* is a number specifies the size of the buffer, in bytes.
file:write(...)|Write the value of each argument to file, arguments could be string and number. If success, returns the file, otherwise returns nil and error description.

Math table|Description
----------|-----------
math.abs(x)|Same with c function *abs*
math.acos(x)|Same with c function *acos*
math.asin(x)|Same with c function *asin*
math.atan(x)|Same with c function *atan*
math.atan2(y, x)|Same with c function *atan2*
math.ceil(x)|Same with c function ceil
math.cos(x)|Same with c function *cos*
math.cosh(x)|Same with c function *cosh*
math.deg(x)|Returns the angle *x*(given in radians) in degrees.
math.exp(x)|Same with c function *exp*
math.floor(x)|Same with c function *floor*
math.frexp(x)|Same with c function *frexp*, returns significand of the given number in range of [0.5, 1) and exponent.
math.huge|The c macro HUGE_VAL
math.ldexp(m, e)|Same with c function *ldexp*
math.log(x [, base])|Same with c function *log*, the default for *base* is *e*.
math.max(x, ...)|Returns the maximum value
math.min(x, ...)|Returns the minimum value
math.modf(x)|Returns the integral part of *x* and the fractional part of *x*.
math.pi|The value of PI
math.pow(x, y)|Same with c function *pow*
math.rad(x)|Returns the angle(given in degrees) in radians.
math.random([m [, n]])|Returns a uniform pseudo-random number. When called with no arguments, returns a real number in the range [0, 1). When called with number *m*, returns a integer in the range [1, m]. When called with number *m* and *n*, returns a integer in the range [m, n].
math.randomseed(x)|Set *x* as the seed for the pseudo-random generator.
math.sin(x)|Same with c function *sin*
math.sinh(x)|Same with c function *sinh*
math.sqrt(x)|Same with c function *sqrt*
math.tan(x)|Same with c function *tan*
math.tanh(x)|Same with c function *tanh*

String table|Description
------------|-----------
string.byte(s [, i [, j]])|Returns the numerical codes of the characters s[*i*] to s[*j*]. The default value for *i* is 1.
string.char(...)|Returns a string with length equal to the number of arguments, in which each character has the numerical code equal to its corresponding argument.
string.len(s)|Returns the length of the string *s*.
string.lower(s)|Returns a string in which each character is lowercase.
string.upper(s)|Returns a string in which each character is uppercase.
string.reverse(s)|Returns a reverse string of the string *s*.
string.sub(s, i [, j])|Returns the substring of *s*[*i*..*j*].

Table table|Description
-----------|-----------
table.concat(t [, sep [, i [, j]]])|Concatenate *t*[*i*] .. *t*[*j*] to a string, insert *sep* between two elements, the default values for *i* is 1, *j* is #*t*, *sep* is an empty string.
table.insert(t, [pos ,] value)|Insert the *value* at position *pos*, by default, the *value* append to the table *t*. Returns true when insert success.
table.pack(...)|Pack all arguments into a table and returns it.
table.remove(t [, pos])|Remove the element at position *pos*, by default, remove the last element. Returns true when remove success.
table.unpack(t [, i [, j]])|Returns *t*[*i*] .. *t*[*j*] elements of table *t*, the default for *i* is 1, the default for *j* is #*t*.
