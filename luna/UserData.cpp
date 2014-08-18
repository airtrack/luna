#include "UserData.h"

namespace luna
{
    UserData::~UserData()
    {
        if (!destroyed_ && destroyer_)
            destroyer_(user_data_);
    }

    void UserData::Accept(GCObjectVisitor *v)
    {
        if (v->Visit(this))
        {
            metatable_->Accept(v);
        }
    }
} // namespace luna
