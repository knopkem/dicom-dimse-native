#include "DataProcessingAsyncWorker.h"

DataProcessingAsyncWorker::DataProcessingAsyncWorker(Buffer<uint8_t> &data,
                                                     Function &callback) : AsyncWorker(callback),
                                                                           dataRef(ObjectReference::New(data, 1)),
                                                                           dataPtr(data.Data()),
                                                                           dataLength(data.Length())
{
}

void DataProcessingAsyncWorker::Execute()
{
    for (size_t i = 0; i < dataLength; i++)
    {
        uint8_t value = *(dataPtr + i);
        *(dataPtr + i) = value * 2;
    }
}

void DataProcessingAsyncWorker::OnOK()
{
    Callback().Call({});

    dataRef.Unref();
}
