#!/usr/bin/env python3
if __name__ == "__main__":
    print('static const uint64_t masks[] =\n{')
    for i in range(65):
        print('    0x%016X,' % (0xFFFFFFFFFFFFFFFF >> i))
    print('};')
