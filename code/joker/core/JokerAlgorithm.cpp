#include "JokerCorePCH.h"
#include "JokerAlgorithm.h"
#include "EAStdC/EAHashCRC.h"

namespace joker::algorithm
{

u16 Crc16(const void* pData, size_t nLength, u16 nInitialValue, bool bFinalize)
{
    return EA::StdC::CRC16(pData, nLength, nInitialValue, bFinalize);
}

u32 Crc24(const void* pData, size_t nLength, u32 nInitialValue, bool bFinalize)
{
    return EA::StdC::CRC24(pData, nLength, nInitialValue, bFinalize);
}

u32 Crc32(const void* pData, size_t nLength, u32 nInitialValue, bool bFinalize)
{
    return EA::StdC::CRC32(pData, nLength, nInitialValue, bFinalize);
}

u64 Crc64(const void* pData, size_t nLength, u64 nInitialValue, bool bFinalize)
{
    return EA::StdC::CRC64(pData, nLength, nInitialValue, bFinalize);
}

}