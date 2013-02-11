#ifndef FUNCTION_H
#define FUNCTION_H

#include "GCObject.h"
#include "OpCode.h"
#include "String.h"
#include <vector>

namespace luna
{
    // Function prototype class, all runtime function(closure) reference this
    // class object. This class contains some static information generated after
    // parse.
    class Function : public GCObject
    {
    public:
        Function();

        // Get function instructions and size
        const Instruction * GetOpCodes() const;
        std::size_t OpCodeSize() const;

    private:
        // function instruction opcodes
        std::vector<Instruction> opcodes_;
        // opcodes' line number
        std::vector<int> opcode_lines_;
        // function define module name
        String *module_;
        // function define line at module
        int line_;
        // count of args
        int args_;
        // has '...' param or not
        bool is_vararg_;
    };
} // namespace luna

#endif // FUNCTION_H
