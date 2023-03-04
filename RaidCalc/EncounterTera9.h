#pragma once

#include <cstdint>
#include "RaidRewards.h"
#include "PersonalInfo9SV.h"

enum Game
{
	GameScarlet,
	GameViolet,
};

enum class AbilityPermission
{
	Any12H = -1,
	Any12 = 0,
	OnlyFirst = 1,
	OnlySecond = 2,
	OnlyHidden = 4,
};

enum class Shiny
{
	Random,
	Never,
	Always,
	AlwaysStar,
	AlwaysSquare,
	FixedValue,
};

enum class GemType : int8_t
{
	Default = 0,
	Random = 1,
	Normal = 2,
	Fighting = 3,
	Flying = 4,
	Poison = 5,
	Ground = 6,
	Rock = 7,
	Bug = 8,
	Ghost = 9,
	Steel = 10,
	Fire = 11,
	Water = 12,
	Grass = 13,
	Electric = 14,
	Psychic = 15,
	Ice = 16,
	Dragon = 17,
	Dark = 18,
	Fairy = 19,
};

enum class EncounterType
{
	Gem,
	Dist,
	Might,
};

enum SizeType : uint8_t
{
	RANDOM = 0,
	XS = 1,
	S = 2,
	M = 3,
	L = 4,
	XL = 5,
	VALUE = 6,
};

class EncounterTera9
{
public:
	static const size_t SizeGem = 0x18;
	static const size_t SizeDist = 0x44;
	static const size_t SizeMight = 0x4E;

	struct RandRateData
	{
		int16_t min;
		int16_t total;
	};

	EncounterTera9(uint8_t *data, EncounterType type) : type(type), fixed_drops(nullptr), lottery_drops(nullptr), lottery_lookup(nullptr)
	{
		species = *(uint16_t*)data;
		form = data[0x02];
		gender = (int8_t)data[0x03] - 1;
		ability = get_ability(data[0x04]);
		flawless_iv_count = data[0x05];
		shiny = get_shiny(data[0x06]);
		level = data[0x07];
		moves[0] = *(uint16_t*)&data[0x08];
		moves[1] = *(uint16_t*)&data[0x0A];
		moves[2] = *(uint16_t*)&data[0x0C];
		moves[3] = *(uint16_t*)&data[0x0E];
		tera_type = (GemType)data[0x10];
		group = data[0x11];
		stars = data[0x12];
		rand_rate = data[0x13];

		switch (type)
		{
		case EncounterType::Gem:
			rand_rate_min[0] = *(int16_t*)&data[0x14];
			rand_rate_min[1] = *(int16_t*)&data[0x16];
			break;
		case EncounterType::Dist:
		case EncounterType::Might:
			for (int i = 0; i < 4; ++i)
			{
				int offset = i * sizeof(int16_t) * 4;
				rand_rate_event[i][GameScarlet].min = *(int16_t*)&data[0x14 + offset];
				rand_rate_event[i][GameViolet].min = *(int16_t*)&data[0x16 + offset];
				rand_rate_event[i][GameScarlet].total = *(int16_t*)&data[0x18 + offset];
				rand_rate_event[i][GameViolet].total = *(int16_t*)&data[0x1A + offset];
			}
			fixed_table_id = *(uint64_t*)&data[0x34];
			lottery_table_id = *(uint64_t*)&data[0x3C];
			if (type == EncounterType::Might)
			{
				nature = data[0x44];
				memcpy(iv, data + 0x45, sizeof(iv));
				iv_fixed = !!data[0x4B];
				scale_type = data[0x4C];
				scale = data[0x4D];
			}
			break;
		}
	}

	// Common
	uint16_t species;
	uint8_t form;
	int8_t gender;
	AbilityPermission ability;
	uint8_t flawless_iv_count;
	Shiny shiny;
	uint8_t level;
	uint16_t moves[4];
	GemType tera_type;
	uint8_t group;
	uint8_t stars;
	uint8_t rand_rate;

	// Gem
	int16_t rand_rate_min[2];

	// Dist/Might
	RandRateData rand_rate_event[4][2];
	uint64_t fixed_table_id;
	uint64_t lottery_table_id;

	// Might
	uint8_t nature;
	int8_t iv[6];
	bool iv_fixed;
	uint8_t scale_type;
	uint8_t scale;

	EncounterType type;
	const RaidFixedRewards* fixed_drops;
	const RaidLotteryRewards* lottery_drops;
	const uint8_t* lottery_lookup;
	PersonalInfo9SV* personal_info;
	int32_t pv_index;

private:
	AbilityPermission get_ability(uint8_t value)
	{
		switch (value)
		{
		case 0: return AbilityPermission::Any12;
		case 1: return AbilityPermission::Any12H;
		case 2: return AbilityPermission::OnlyFirst;
		case 3: return AbilityPermission::OnlySecond;
		case 4: return AbilityPermission::OnlyHidden;
		default: return AbilityPermission::Any12;
		}
	}

	Shiny get_shiny(uint8_t value)
	{
		switch (value)
		{
		case 0: return Shiny::Random;
		case 1: return Shiny::Never;
		case 2: return Shiny::Always;
		default: return Shiny::Random;
		}
	}
};
