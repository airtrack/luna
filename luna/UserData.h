#ifndef USER_DATA_H
#define USER_DATA_H

#include "GC.h"
#include "Table.h"

namespace luna
{
    class UserData : public GCObject
    {
    public:
        typedef void (*Destroyer)(void *);

        UserData() = default;
        virtual ~UserData();

        virtual void Accept(GCObjectVisitor *v) final;

        void Set(void *user_data, Table *metatable)
        {
            user_data_ = user_data;
            metatable_ = metatable;
        }

        void SetDestroyer(Destroyer destroyer)
        {
            destroyer_ = destroyer;
        }

        void MarkDestroyed()
        {
            destroyed_ = true;
        }

        void * GetData() const
        {
            return user_data_;
        }

        Table * GetMetatable() const
        {
            return metatable_;
        }

    private:
        // Point to user data
        void *user_data_ = nullptr;
        // Metatable of user data
        Table *metatable_ = nullptr;
        // User data destroyer, call it when user data destroy
        Destroyer destroyer_ = nullptr;
        // Whether user data destroyed
        bool destroyed_ = false;
    };
} // namespace luna

#endif // USER_DATA_H
