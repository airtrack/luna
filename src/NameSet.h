#ifndef NAME_SET_H
#define NAME_SET_H

#include "types/Value.h"
#include <unordered_set>
#include <memory>
#include <vector>

namespace lua
{
    class NameSet
    {
    public:
        NameSet()
        {
            PushLevel();
        }

        bool Has(const Value *name) const
        {
            for (auto it = level_set_.rbegin(); it != level_set_.rend(); ++it)
            {
                if ((*it)->find(name) != (*it)->end())
                    return true;
            }

            return false;
        }

        bool Insert(const Value *name)
        {
            auto result = level_set_.back()->insert(name);
            return result.second;
        }

        void PushLevel()
        {
            level_set_.push_back(SetPtr(new Set));
        }

        void PopLevel()
        {
            level_set_.pop_back();
        }

    private:
        typedef std::unordered_set<const Value *, ValueHasher, ValueEqualer> Set;
        typedef std::unique_ptr<Set> SetPtr;
        typedef std::vector<SetPtr> LevelSet;
        LevelSet level_set_;
    };

    class NameSetLevelPusher
    {
    public:
        explicit NameSetLevelPusher(NameSet *name_set)
            : name_set_(name_set)
        {
            name_set_->PushLevel();
        }

        ~NameSetLevelPusher()
        {
            name_set_->PopLevel();
        }

    private:
        NameSet *name_set_;
    };
} // namespace lua

#endif // NAME_SET_H
