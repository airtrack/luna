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
        typedef std::unordered_set<const Value *, ValueHasher, ValueEqualer> Set;
        typedef std::unique_ptr<Set> SetPtr;
        typedef std::vector<SetPtr> LevelSet;

        friend class Iterator;
        class Iterator
        {
        public:
            Iterator()
                : ns_(0),
                  level_(0)
            {
            }

            explicit Iterator(const NameSet *ns)
                : ns_(ns),
                  level_(0)
            {
                if (ns_ && !ns_->Empty())
                {
                    it_ = ns_->level_set_[level_]->begin();
                    if (IsLevelEnd())
                        ++(*this);
                }
            }

            Iterator& operator ++ ()
            {
                IteratorNext();
                while (!IsEnd() && IsLevelEnd())
                    IteratorNext();

                if (IsEnd())
                    Reset();
                return *this;
            }

            Iterator operator ++ (int)
            {
                Iterator res = *this;
                ++(*this);
                return res;
            }

            const Value * operator * () const
            {
                if (ns_)
                    return *it_;
                return 0;
            }

            friend bool operator == (const Iterator& left, const Iterator& right)
            {
                return left.ns_ == right.ns_ &&
                       left.level_ == right.level_ &&
                       left.it_ == right.it_;
            }

            friend bool operator != (const Iterator& left, const Iterator& right)
            {
                return !(left == right);
            }

        private:
            void Reset()
            {
                ns_ = 0;
                level_ = 0;
                it_ = NameSet::Set::iterator();
            }

            void IteratorNext()
            {
                if (!ns_)
                    return ;

                if (!IsLevelEnd())
                {
                    ++it_;
                    return ;
                }

                ++level_;
                if (level_ >= ns_->level_set_.size())
                    return ;

                it_ = ns_->level_set_[level_]->begin();
            }

            bool IsLevelEnd()
            {
                if (!ns_)
                    return true;
                return it_ == ns_->level_set_[level_]->end();
            }

            bool IsEnd()
            {
                if (!ns_ || ns_->Empty())
                    return true;

                if (!IsLevelEnd())
                    return false;

                return level_ >= ns_->level_set_.size() - 1;
            }

            const NameSet *ns_;
            std::size_t level_;
            NameSet::Set::iterator it_;
        };

        explicit NameSet(bool init_first_level = true)
        {
            if (init_first_level)
                PushLevel();
        }

        Iterator Begin() const
        {
            return Iterator(this);
        }

        Iterator End() const
        {
            return Iterator();
        }

        bool Empty() const
        {
            return level_set_.empty();
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
            if (Empty())
                PushLevel();

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
