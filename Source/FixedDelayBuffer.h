#pragma once
#include <JuceHeader.h>

template<typename T>
class FixedDelayBuffer
{
public:
    FixedDelayBuffer()
    {
        int size = 96000;
        arr.ensureStorageAllocated(size);
        arr.insertMultiple(0, 0, size);

        write = arr.size() - 1;
    }
    T readOldestSample() const noexcept { return arr[read]; }
    T readNewestSample() const noexcept { return arr[write]; }

    Array<T> getBufferOrdered() {
        Array<T> buf;
        for (int i = 0; i < arr.size(); i++) {
            buf.add(arr[read]);
            ++read;
            if (read >= arr.size())
                read = 0;
        }
        return buf;
    }

    T writeSample(T sample)
    {
        ++write;
        if (write >= arr.size())
            write = 0;
        ++read;
        if (read >= arr.size())
            read = 0;
        auto discarded = arr[write];
        arr.setUnchecked(write, sample);
        return discarded;
    }
private:
    Array<T> arr;
    int read = 0;
    int write;
};