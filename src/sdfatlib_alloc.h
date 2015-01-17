#ifndef SDFATLIB_ALLOC_H
#define SDFATLIB_ALLOC_H

#include "alloc.h"

#include <SdFat.h>

// UNDONE: make this dependent upon platform
struct SSdfatlibMemAllocProperties
{
    static const uint8_t smallPageCount = 4, smallPageSize = 32;
    static const uint8_t mediumPageCount = 4, mediumPageSize = 64;
    static const uint8_t bigPageCount = 4;
    static const uint16_t bigPageSize = 1024 * 4;
    static const uint32_t poolSize = DEFAULT_POOLSIZE;
};

template <typename> class CSdfatlibVirtMemAlloc;

template <typename TProperties=SSdfatlibMemAllocProperties>
class CSdfatlibVirtMemAlloc : public CVirtMemAlloc<TProperties, CSdfatlibVirtMemAlloc<TProperties> >
{
    SdFile sdFile;

    void doStart(void)
    {
        // file does not exist yet (can we create it)?
        if (sdFile.open("ramfile.vir", O_CREAT | O_EXCL))
            this->writeZeros(0, this->getPoolSize()); // make it the right size
        else // already exists, check size
        {
            if (!sdFile.open("ramfile", O_CREAT | O_RDWR))
            {
                Serial.println("opening ram file failed");
                while (true)
                    ;
            }

            const uint32_t size = sdFile.fileSize();
            if (size < this->getPoolSize())
                this->writeZeros(size, this->getPoolSize() - size);
        }
    }

    void doSuspend(void) { } // UNDONE
    void doStop(void)
    {
        sdFile.close();
        sdFile.remove();
    }
    void doRead(void *data, TVirtPtrSize offset, TVirtPtrSize size)
    {
//        const uint32_t t = micros();
        sdFile.seekSet(offset);
        sdFile.read(data, size);
//        Serial.print("read: "); Serial.print(size); Serial.print("/"); Serial.println(micros() - t);
    }

    void doWrite(const void *data, TVirtPtrSize offset, TVirtPtrSize size)
    {
//        const uint32_t t = micros();
        sdFile.seekSet(offset);
        sdFile.write(data, size);
//        Serial.print("write: "); Serial.print(size); Serial.print("/"); Serial.println(micros() - t);
    }

public:
    CSdfatlibVirtMemAlloc(void) { }
    ~CSdfatlibVirtMemAlloc(void) { doStop(); }
};

template <typename, typename> class CVirtPtr;
template <typename T> struct TSdfatlibVirtPtr { typedef CVirtPtr<T, CSdfatlibVirtMemAlloc<> > type; };

#endif // SDFATLIB_ALLOC_H
