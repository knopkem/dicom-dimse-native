#include <napi.h>

using namespace Napi;

class DataProcessingAsyncWorker : public AsyncWorker
{
    public:
        DataProcessingAsyncWorker(Buffer<uint8_t> &data,
                                  Function &callback);

        void Execute();

        void OnOK();

    private:
        ObjectReference dataRef;
        uint8_t *dataPtr;
        size_t dataLength;
};
