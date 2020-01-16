#include <napi.h>

using namespace Napi;

class MoveAsyncWorker : public AsyncWorker
{
    public:
        MoveAsyncWorker(std::string data, Function &callback);

        void Execute();

        void OnOK();

    private:
    std::string _input;
    std::string _output;
};
