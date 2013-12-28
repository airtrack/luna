#ifndef GUARD_H
#define GUARD_H

#include <functional>

// Guard class, using for RAII operations.
// e.g.
//      {
//          Guard g(constructor, destructor);
//          ...
//      }
class Guard
{
public:
    Guard(const std::function<void ()> &enter,
          const std::function<void ()> &leave)
    : leave_(leave)
    {
        enter();
    }

    ~Guard()
    {
        leave_();
    }

    Guard(const Guard &) = delete;
    void operator = (const Guard &) = delete;

private:
    std::function<void ()> leave_;
};

#endif // GUARD_H
