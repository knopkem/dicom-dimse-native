#include <napi.h>

using namespace Napi;

class GetAsyncWorker : public AsyncWorker
{
    public:
        GetAsyncWorker(std::string data, Function &callback);

        void Execute();

        void OnOK();

    private:
    std::string _input;
    std::string _output;
};
