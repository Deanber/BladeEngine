#pragma one

namespace RHI {
    class ICommandList;
    class IFence;

    class ICommandQueue
    {
    public:
        virtual ~ICommandQueue() = default;

        virtual void Execute(ICommandList*) = 0;

        virtual void Signal(IFence*) = 0;
        virtual void Wait(IFence*) = 0;
    };
}