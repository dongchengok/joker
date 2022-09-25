#pragma once

#define JCLAMP(v, lo, hi) ((v) < (lo) ? (v) : (v) > (hi) ? hi : v)

namespace joker::algorithm
{
    u16 Crc16(const void* pData, size_t nLength, u16 nInitialValue = 0xffff, bool bFinalize = true);
    u32 Crc24(const void* pData, size_t nLength, u32 nInitialValue = 0x00b704ce, bool bFinalize = true);
    u32 Crc32(const void* pData, size_t nLength, u32 nInitialValue = 0xFFFFFFFF, bool bFinalize = true);
    u64 Crc64(const void* pData, size_t nLength, u64 nInitialValue = 0xFFFFFFFFFFFFFFFF, bool bFinalize = true);
}