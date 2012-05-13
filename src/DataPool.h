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
#include <functional>

namespace lua
{
    class DataPool
    {
    public:
        typedef std::function<void ()> StatFunc;

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

        void Sweep();
        void SetOnAlloc(const StatFunc& on_alloc);
        void SetOnDealloc(const StatFunc& on_dealloc);

    private:
        template<typename ElemType>
        struct PoolElement
        {
            typedef PoolElement<ElemType> ThisType;

            ElemType elem;
            ThisType *next;

            template<typename T>
            explicit PoolElement(const T& t, ThisType *n)
                : elem(t), next(n) { }

            template<typename T>
            explicit PoolElement(std::unique_ptr<T>&& t, ThisType *n)
                : elem(std::move(t)), next(n) { }

            template<typename T, typename V>
            PoolElement(const T& t, const V& v, ThisType *n)
                : elem(t, v), next(n) { }
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
            on_alloc_();
            PoolElement<ElemType> *elem = new PoolElement<ElemType>(param, pool);
            pool = elem;
            return &elem->elem;
        }

        template<typename ElemType>
        void SweepPool(PoolElement<ElemType> *& pool)
        {
            PoolElement<ElemType> *previous = 0;
            PoolElement<ElemType> *current = pool;
            while (current)
            {
                PoolElement<ElemType> *sweep = 0;
                if (current->elem.IsSelfMarked())
                {
                    current->elem.UnmarkSelf();
                    if (previous)
                        previous = previous->next;
                    else
                        previous = current;
                }
                else
                {
                    sweep = current;
                    if (previous)
                        previous->next = current->next;
                    else
                        pool = current->next;
                }

                current = current->next;

                // Sweep the elem
                if (sweep)
                {
                    on_dealloc_();
                    delete sweep;
                }
            }
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

        StatFunc on_alloc_;
        StatFunc on_dealloc_;
    };
} // namespace lua

#endif // DATA_POOL_H
