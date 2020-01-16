#include <napi.h>

using namespace Napi;

class EchoAsyncWorker : public AsyncWorker
{
    public:
        EchoAsyncWorker(std::string data, Function &callback);

        void Execute();

        void OnOK();

    private:
    std::string _input;
    std::string _output;
};
