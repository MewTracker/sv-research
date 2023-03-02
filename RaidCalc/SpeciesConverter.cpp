#include <cstdlib>
#include "SpeciesConverter.h"

int8_t SpeciesConverter::table_national_to_internal[94] =
{
      1,   1,   1,   1,  33,  33,  33,  21,  21,  44,
     44,   7,   7,   7,  29,  31,  31,  31,  68,  68,
     68,   2,   2,  17,  17,  30,  30,  24,  24,  28,
     28,  58,  58,  12, -13, -13, -31, -31, -29, -29,
     43,  43,  43, -31, -31,  -3, -30, -30, -23, -23,
    -14, -24,  -3,  -3, -47, -47, -12, -27, -27, -44,
    -46, -26,  31,  29, -53, -65,  25,  -6,  -3,  -7,
     -4,  -4,  -8,  -4,   1,  -3,  -3,  -6,  -4, -47,
    -47, -47, -23, -23,  -5,  -7,  -9,  -7, -20, -13,
     -9,  -9, -29, -23,
};

int8_t SpeciesConverter::table_internal_to_national[94] =
{
     65,  -1,  -1,  -1,  -1,  31,  31,  47,  47,  29,
     29,  53,  31,  31,  46,  44,  30,  30,  -7,  -7,
     -7,  13,  13,  -2,  -2,  23,  23,  24, -21, -21,
     27,  27,  47,  47,  47,  26,  14, -33, -33, -33,
    -17, -17,   3, -29,  12, -12, -31, -31, -31,   3,
      3, -24, -24, -44, -44, -30, -30, -28, -28,  23,
     23,   6,   7,  29,   8,   3,   4,   4,  20,   4,
     23,   6,   3,   3,   4,  -1,  13,   9,   7,   5,
      7,   9,   9, -43, -43, -43, -68, -68, -68, -58,
    -58, -25, -29, -31,
};

uint16_t SpeciesConverter::get_internal(uint16_t species)
{
    int32_t shift = species - first_unaligned_national;
    if ((uint32_t)shift >= _countof(table_national_to_internal))
        return species;
    return (uint16_t)(species + table_national_to_internal[shift]);
}

uint16_t SpeciesConverter::get_national(uint16_t dev_id)
{
    int32_t shift = dev_id - first_unaligned_internal;
    if ((uint32_t)shift >= _countof(table_internal_to_national))
        return dev_id;
    return (uint16_t)(dev_id + table_internal_to_national[shift]);
}
