#ifndef DATA_POOL_H
#define DATA_POOL_H

namespace lua
{
    class Nil;
    class Bool;
    class Number;
    class String;
    class Table;
    class Function;

    class DataPool
    {
    public:
        Nil * GetNil();
        Bool * GetTrue();
        Bool * GetFalse();
        Number * GetNumber();
        String * GetString();
        Table * GetTable();
        Function * GetFunction();

    private:
    };
} // namespace lua

#endif // DATA_POOL_H
