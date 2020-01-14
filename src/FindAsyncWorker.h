#include <napi.h>

using namespace Napi;

class FindAsyncWorker : public AsyncWorker
{
    public:
        FindAsyncWorker(std::string data, Function &callback);

        void Execute();

        void OnOK();

    private:
    std::string _input;
    std::string _output;
};
