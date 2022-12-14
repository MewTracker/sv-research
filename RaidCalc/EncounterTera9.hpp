#pragma once

#include <cstdint>
#include "RaidRewards.hpp"

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

class EncounterTera9
{
public:
	EncounterTera9(uint8_t *data) : fixed_drops(nullptr), lottery_drops(nullptr)
	{
		species = *(uint16_t *)data;
		form = data[0x02];
		gender = (int8_t)data[0x03] - 1;
		ability = get_ability(data[0x04]);
		flawless_iv_count = data[0x05];
		shiny = get_shiny(data[0x06]);
		level = data[0x07];
		moves[0] = *(uint16_t *)&data[0x08];
		moves[1] = *(uint16_t *)&data[0x0A];
		moves[2] = *(uint16_t *)&data[0x0C];
		moves[3] = *(uint16_t *)&data[0x0E];
		tera_type = (GemType)data[0x10];
		index = data[0x11];
		stars = data[0x12];
		rand_rate = data[0x13];
		rand_rate_min[0] = *(int16_t *)&data[0x14];
		rand_rate_min[1] = *(int16_t *)&data[0x16];
	}

	uint16_t species;
	uint8_t form;
	int8_t gender;
	AbilityPermission ability;
	uint8_t flawless_iv_count;
	Shiny shiny;
	uint8_t level;
	uint16_t moves[4];
	GemType tera_type;
	uint8_t index;
	uint8_t stars;
	uint8_t rand_rate;
	int16_t rand_rate_min[2];

	const RaidFixedRewards *fixed_drops;
	const RaidLotteryRewards *lottery_drops;

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
