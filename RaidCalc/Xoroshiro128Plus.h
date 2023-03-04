#pragma once

#include <cstdint>
#include <intrin.h>

class Xoroshiro128Plus
{
public:
    static const uint64_t XOROSHIRO_CONST0 = 0x0F4B17A579F18960;
    static const uint64_t XOROSHIRO_CONST1 = 0x82A2B175229D6A5B;

    Xoroshiro128Plus(uint64_t s0 = XOROSHIRO_CONST0, uint64_t s1 = XOROSHIRO_CONST1) : s0(s0), s1(s1)
    {

    }

    FORCEINLINE uint64_t next()
    {
        uint64_t _s0 = s0;
        uint64_t _s1 = s1;
        uint64_t result = _s0 + _s1;
        _s1 ^= _s0;
        s0 = _rotl64(_s0, 24) ^ _s1 ^ (_s1 << 16);
        s1 = _rotl64(_s1, 37);
        return result;
    }

	FORCEINLINE uint64_t prev()
    {
        uint64_t _s0 = s0;
        uint64_t _s1 = s1;
        _s1 = _rotl64(_s1, 27);
        _s0 = _s0 ^ _s1 ^ (_s1 << 16);
        _s0 = _rotl64(_s0, 40);
        _s1 ^= _s0;
        uint64_t result = _s0 + _s1;
        s0 = _s0;
        s1 = _s1;
        return result;
    }

	FORCEINLINE uint64_t next_int(uint64_t mod = 0xFFFFFFFF)
    {
        uint64_t mask = get_bitmask(mod);
        uint64_t res;
        do
        {
            res = next() & mask;
        } while (res >= mod);
        return res;
    }

	FORCEINLINE uint64_t get_bitmask(uint64_t x)
    {
		static const uint64_t masks[] =
		{
			0xFFFFFFFFFFFFFFFF,
			0x7FFFFFFFFFFFFFFF,
			0x3FFFFFFFFFFFFFFF,
			0x1FFFFFFFFFFFFFFF,
			0x0FFFFFFFFFFFFFFF,
			0x07FFFFFFFFFFFFFF,
			0x03FFFFFFFFFFFFFF,
			0x01FFFFFFFFFFFFFF,
			0x00FFFFFFFFFFFFFF,
			0x007FFFFFFFFFFFFF,
			0x003FFFFFFFFFFFFF,
			0x001FFFFFFFFFFFFF,
			0x000FFFFFFFFFFFFF,
			0x0007FFFFFFFFFFFF,
			0x0003FFFFFFFFFFFF,
			0x0001FFFFFFFFFFFF,
			0x0000FFFFFFFFFFFF,
			0x00007FFFFFFFFFFF,
			0x00003FFFFFFFFFFF,
			0x00001FFFFFFFFFFF,
			0x00000FFFFFFFFFFF,
			0x000007FFFFFFFFFF,
			0x000003FFFFFFFFFF,
			0x000001FFFFFFFFFF,
			0x000000FFFFFFFFFF,
			0x0000007FFFFFFFFF,
			0x0000003FFFFFFFFF,
			0x0000001FFFFFFFFF,
			0x0000000FFFFFFFFF,
			0x00000007FFFFFFFF,
			0x00000003FFFFFFFF,
			0x00000001FFFFFFFF,
			0x00000000FFFFFFFF,
			0x000000007FFFFFFF,
			0x000000003FFFFFFF,
			0x000000001FFFFFFF,
			0x000000000FFFFFFF,
			0x0000000007FFFFFF,
			0x0000000003FFFFFF,
			0x0000000001FFFFFF,
			0x0000000000FFFFFF,
			0x00000000007FFFFF,
			0x00000000003FFFFF,
			0x00000000001FFFFF,
			0x00000000000FFFFF,
			0x000000000007FFFF,
			0x000000000003FFFF,
			0x000000000001FFFF,
			0x000000000000FFFF,
			0x0000000000007FFF,
			0x0000000000003FFF,
			0x0000000000001FFF,
			0x0000000000000FFF,
			0x00000000000007FF,
			0x00000000000003FF,
			0x00000000000001FF,
			0x00000000000000FF,
			0x000000000000007F,
			0x000000000000003F,
			0x000000000000001F,
			0x000000000000000F,
			0x0000000000000007,
			0x0000000000000003,
			0x0000000000000001,
			0x0000000000000000,
		};
        return masks[__lzcnt64(x - 1)];
    }

	FORCEINLINE float next_float(float range, float bias = 0.0f)
    {
        static const uint32_t inv_64_f = 0x1F800000UL;
        uint64_t n = next();
        return (range * (n * (*(float *)&inv_64_f))) + bias;
    }

	FORCEINLINE uint8_t next_byte()
	{
		return (uint8_t)(next_int(0x81) + next_int(0x80));
	}

private:
    uint64_t s0, s1;
};
