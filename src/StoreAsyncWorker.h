#include <napi.h>

using namespace Napi;

class StoreAsyncWorker : public AsyncWorker
{
    public:
        StoreAsyncWorker(std::string data, Function &callback);

        void Execute();

        void OnOK();

    private:
    std::string _input;
    std::string _output;
};
