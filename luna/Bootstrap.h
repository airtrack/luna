#ifndef BOOTSTRAP_H
#define BOOTSTRAP_H

#include "OpCode.h"

namespace luna
{
    class State;

    // This class provide bootstrap program to start run by VM
    class Bootstrap
    {
    public:
        explicit Bootstrap(State *state);

        // Prepare bootstrap
        void Prepare();

    private:
        State *state_;
        Instruction boot_;
    };
} // namespace luna

#endif // BOOTSTRAP_H
