#pragma once

#include <cstdint>

class SpeciesConverter
{
public:
	static uint16_t get_internal(uint16_t species);
	static uint16_t get_national(uint16_t dev_id);

private:
	static constexpr int32_t first_unaligned_national = 917;
	static constexpr int32_t first_unaligned_internal = first_unaligned_national;

	static int8_t table_national_to_internal[94];
	static int8_t table_internal_to_national[94];
};
