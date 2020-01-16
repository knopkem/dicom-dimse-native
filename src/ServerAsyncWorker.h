#include <napi.h>

using namespace Napi;

class ServerAsyncWorker : public AsyncWorker
{
    public:
        ServerAsyncWorker(std::string data, Function &callback);

        void Execute();

        void OnOK();

    private:
    std::string _input;
    std::string _output;
};
