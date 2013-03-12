#ifndef VM_H
#define VM_H

namespace luna
{
    class State;

    class VM
    {
    public:
        explicit VM(State *state);

        void Execute();

    private:
        State *state_;
    };
} // namespace luna

#endif // VM_H
