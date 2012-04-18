#ifndef DATA_POOL_H
#define DATA_POOL_H

#include "types/Value.h"
#include "types/Bool.h"
#include "types/Nil.h"
#include "types/Number.h"
#include "types/String.h"
#include "types/Table.h"
#include "types/Function.h"
#include "types/Closure.h"
#include "types/NativeFunction.h"
#include <string>
#include <memory>

namespace lua
{
    class DataPool
    {
    public:
        DataPool();
        ~DataPool();

        Nil * GetNil();
        Bool * GetTrue();
        Bool * GetFalse();
        Bool * GetBool(bool b);
        Number * GetNumber(double number);
        String * GetString(const std::string& str);
        Table * GetTable();
        TableValue * GetTableValue(Value *value);
        Function * GetFunction(std::unique_ptr<UpvalueNameSet> &&upvalue_set);
        Closure * GetClosure(Function *func);
        NativeFunction * GetNativeFunction(const NativeFunction::FuncType& func);

    private:
        template<typename ElemType>
        struct PoolElement
        {
            typedef PoolElement<ElemType> ThisType;

            ElemType elem;
            ThisType *next;

            template<typename T>
            explicit PoolElement(const T& t, ThisType *n) : elem(t), next(n) { }

            template<typename T>
            explicit PoolElement(std::unique_ptr<T>&& t, ThisType *n) : elem(std::move(t)), next(n) { }

            template<typename T, typename V>
            PoolElement(const T& t, const V& v, ThisType *n) : elem(t, v), next(n) { }
        };

        typedef PoolElement<Number> NumberPoolElement;
        typedef PoolElement<String> StringPoolElement;
        typedef PoolElement<Table> TablePoolElement;
        typedef PoolElement<TableValue> TableValuePoolElement;
        typedef PoolElement<Function> FunctionPoolElement;
        typedef PoolElement<Closure> ClosurePoolElement;
        typedef PoolElement<NativeFunction> NativeFunctionPoolElement;

        template<typename ElemType>
        void ReleasePool(PoolElement<ElemType> *& pool)
        {
            while (pool)
            {
                PoolElement<ElemType> *tmp = pool;
                pool = pool->next;
                delete tmp;
            }
        }

        template<typename ElemType, typename ParamType>
        ElemType * NewElem(PoolElement<ElemType> *& pool, const ParamType& param)
        {
            PoolElement<ElemType> *elem = new PoolElement<ElemType>(param, pool);
            pool = elem;
            return &elem->elem;
        }

        Nil nil_;
        Bool true_;
        Bool false_;

        NumberPoolElement *number_pool_;
        StringPoolElement *string_pool_;
        TablePoolElement *table_pool_;
        TableValuePoolElement *table_value_pool_;
        FunctionPoolElement *function_pool_;
        ClosurePoolElement *closure_pool_;
        NativeFunctionPoolElement *native_func_pool_;
    };
} // namespace lua

#endif // DATA_POOL_H
