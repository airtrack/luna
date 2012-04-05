#ifndef NAME_SET_H
#define NAME_SET_H

#include "types/Value.h"
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <vector>

namespace lua
{
    class NameSet
    {
    public:
        std::size_t GetNameLevel(const Value *name) const
        {
            std::size_t level = GetLevelCount();

            for (; level > 0; --level)
            {
                auto it = level_set_[level - 1]->find(name);
                if (it != level_set_[level - 1]->end())
                    return level;
            }

            return 0;
        }

        bool Insert(const Value *name)
        {
            auto result = level_set_.back()->insert(name);
            return result.second;
        }

        std::size_t GetLevelCount() const
        {
            return level_set_.size();
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

    class UpValueNameSet
    {
    public:
        typedef std::unordered_map<const Value *, std::size_t,
            ValueHasher, ValueEqualer> UpValueNameLevelMap;
        typedef UpValueNameLevelMap::const_iterator iterator;

        bool Empty() const
        {
            return up_value_level_map_.empty();
        }

        iterator Begin() const
        {
            return up_value_level_map_.begin();
        }

        iterator End() const
        {
            return up_value_level_map_.end();
        }

        void Insert(const Value *up_value, std::size_t level)
        {
            up_value_level_map_.insert(std::make_pair(up_value, level));
        }

    private:
        UpValueNameLevelMap up_value_level_map_;
    };
} // namespace lua

#endif // NAME_SET_H
