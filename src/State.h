#ifndef STATE_H
#define STATE_H

namespace luna
{
    class String;

    class State
    {
    public:
        void LoadModule();
        String *GetCurrentModule() const;
    };
} // namespace luna

#endif // STATE_H
