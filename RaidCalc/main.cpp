#include <cstdio>
#include <cstdint>
#include <cassert>
#include <vector>
#include <memory>
#include <windows.h>
#include "Xoroshiro128Plus.hpp"
#include "EncounterTera9.hpp"
#include "PokemonNames.hpp"

#define THREAD_COUNT	4
#define SEED_COUNT		0x100000000
#define DROP_THRESHOLD	8
#define GAME			GameViolet
#define SHINY_MODE

struct RaidResult
{
	uint32_t seed;
	uint32_t result;
};

struct ThreadData
{
	uint64_t range_min;
	uint64_t range_max;
	std::vector<RaidResult> results;
};

std::vector<std::vector<EncounterTera9>> encounters;
std::vector<std::vector<uint8_t>> fast_encounter_lookup;
std::vector<int8_t> target_drops;

static std::vector<uint8_t> read_file(const char *filename)
{
	FILE *f = nullptr;
	fopen_s(&f, filename, "rb");
	if (!f)
		return std::vector<uint8_t>();
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);
	std::vector<uint8_t> buffer;
	buffer.resize(size);
	size_t bytes_read = fread(buffer.data(), 1, size, f);
	fclose(f);
	if (bytes_read != size)
		return std::vector<uint8_t>();
	return buffer;
}

static const RaidFixedRewards *get_fixed_drop_table(uint64_t table_name)
{
	for (auto &table : fixed_rewards)
		if (table.table_name == table_name)
			return &table;
	return nullptr;
}

static const RaidLotteryRewards *get_lottry_drop_table(uint64_t table_name)
{
	for (auto &table : lottery_rewards)
		if (table.table_name == table_name)
			return &table;
	return nullptr;
}

static int16_t get_rate_total_base(size_t star)
{
	static const int16_t rates[2][7] =
	{
		{ 0, 5800, 5300, 7400, 8800, 9100, 6500 },
		{ 0, 5800, 5300, 7400, 8700, 9100, 6500 },
	};
	assert(star > 0 && star < _countof(rates[0]));
	return rates[GAME][star];
};

static void compute_fast_encounter_lookups()
{
	fast_encounter_lookup.resize(encounters.size());
	for (size_t stars = 1; stars < encounters.size(); ++stars)
	{
		uint64_t total = get_rate_total_base(stars);
		for (uint64_t speciesroll = 0; speciesroll < total; ++speciesroll)
		{
			for (size_t i = 0; i < encounters[stars].size(); ++i)
			{
				EncounterTera9 &enc = encounters[stars][i];
				int16_t minimum = enc.rand_rate_min[GAME];
				if (minimum >= 0 && (uint32_t)((int32_t)speciesroll - minimum) < enc.rand_rate)
				{
					assert(i <= 0xFF);
					fast_encounter_lookup[stars].push_back((uint8_t)i);
					break;
				}
			}
		}
	}
}

static EncounterTera9 *get_encounter(uint32_t seed, int stage)
{
	Xoroshiro128Plus gen(seed);
	int32_t stars = 6;
	uint64_t total = get_rate_total_base(stars);
	uint64_t speciesroll = gen.next_int(total);
	return &encounters[stars][fast_encounter_lookup[stars][speciesroll]];
}

static int32_t get_reward_count(int32_t random, int32_t stars)
{
	static const int32_t slots[8][5] =
	{
		{ 0, 0, 0, 0, 0 },
		{ 4, 5, 6, 7, 8 },
		{ 4, 5, 6, 7, 8 },
		{ 5, 6, 7, 8, 9 },
		{ 5, 6, 7, 8, 9 },
		{ 6, 7, 8, 9, 10 },
		{ 7, 8, 9, 10, 11 },
		{ 7, 8, 9, 10, 11 },
	};
	assert(stars > 0 && stars < _countof(slots));
	static const int8_t random_lookup[] =
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	};
	assert(random < _countof(random_lookup));
	return slots[stars][random_lookup[random]];
}

static uint32_t get_rewards(uint32_t seed, int progress, int raid_boost)
{
	EncounterTera9 *enc = get_encounter(seed, progress);
	uint32_t drop_counter = 0;
	for (auto &item : enc->fixed_drops->items)
	{
		if (item.category == 0 && item.item_id == 0)
			continue;
		drop_counter += target_drops[item.item_id_final];
	}
	int32_t rate_total = enc->lottery_drops->rate_total;
	Xoroshiro128Plus gen(seed);
	int32_t count = get_reward_count((int32_t)gen.next_int(100), enc->stars) + raid_boost;
	for (int32_t i = 0; i < count; ++i)
	{
		int32_t roll = (int32_t)gen.next_int((uint64_t)rate_total);
		for (size_t j = 0; j < _countof(enc->lottery_drops->items); ++j)
		{
			auto &item = enc->lottery_drops->items[j];
			if (roll < item.rate)
			{
				drop_counter += target_drops[item.item_id_final];
				break;
			}
			roll -= item.rate;
		}
	}
	return drop_counter;
}

DWORD WINAPI rewards_thread(LPVOID Parameter)
{
	ThreadData *data = (ThreadData *)Parameter;
	for (uint64_t seed = data->range_min; seed < data->range_max; ++seed)
	{
		uint32_t drops = get_rewards((uint32_t)seed, 4, 0);
		if (drops >= DROP_THRESHOLD)
			data->results.push_back({ (uint32_t)seed, drops });
	}
	return 0;
}

DWORD WINAPI shiny_thread(LPVOID Parameter)
{
	ThreadData *data = (ThreadData *)Parameter;
	for (uint64_t seed = data->range_min; seed < data->range_max; ++seed)
	{
		Xoroshiro128Plus gen(seed);
		uint32_t EC = (uint32_t)gen.next_int();
		uint32_t TIDSID = (uint32_t)gen.next_int();
		uint32_t PID = (uint32_t)gen.next_int();
		bool is_shiny = (((PID >> 16) ^ (PID & 0xFFFF)) >> 4) == (((TIDSID >> 16) ^ (TIDSID & 0xFFFF)) >> 4);
		if (!is_shiny)
			continue;
		EncounterTera9 *enc = get_encounter((uint32_t)seed, 4);
		int32_t ivs[] = { -1, -1, -1, -1, -1, -1 };
		for (uint8_t i = 0; i < enc->flawless_iv_count; ++i)
		{
			int32_t index;
			do
			{
				index = (int32_t)gen.next_int(6);
			} while (ivs[index] != -1);
			ivs[index] = 31;
		}
		for (size_t i = 0; i < _countof(ivs); ++i)
		{
			if (ivs[i] == -1)
				ivs[i] = (int32_t)gen.next_int(32);
		}
		if (ivs[0] + ivs[1] + ivs[2] + ivs[3] + ivs[4] + ivs[5] == 186)
			data->results.push_back({ (uint32_t)seed, enc->species });
	}
	return 0;
}

int main(int argc, char *argv[])
{
	std::vector<uint8_t> encounter_data = read_file("encounter_gem_paldea.pkl");
	std::vector<uint8_t> reward_map = read_file("reward_map");
	size_t encounter_count = encounter_data.size() / 0x18;
	size_t rewards_count = reward_map.size() / 0x10;
	if (encounter_count != rewards_count)
	{
		printf("Malformed data files detected (encounters=%zu rewards=%zu)!\n", encounter_count, rewards_count);
		return 1;
	}
	encounters.resize(7);
	uint64_t *rewards = (uint64_t *)reward_map.data();
	for (size_t i = 0; i < encounter_count; ++i, rewards += 2)
	{
		auto enc = EncounterTera9(encounter_data.data() + i * 0x18);
		enc.fixed_drops = get_fixed_drop_table(rewards[0]);
		enc.lottery_drops = get_lottry_drop_table(rewards[1]);
		assert(enc.stars > 0 && enc.stars < 7);
		encounters[enc.stars].push_back(enc);
	}
	compute_fast_encounter_lookups();
	target_drops.resize(20000 + 1);
	// Herba Mystica
	for (size_t i = 1904; i <= 1908; ++i)
		target_drops[i] = 1;
	// Ability Patches
	//target_drops[1606] = 1;

	static_assert((SEED_COUNT % THREAD_COUNT) == 0, "Thread count must be a power of 2");
	LARGE_INTEGER frequency, start_time, end_time, elapsed_microseconds;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&start_time);
	auto handles = std::make_unique<HANDLE[]>(THREAD_COUNT);
	auto thread_data = std::make_unique<ThreadData[]>(THREAD_COUNT);
	for (uint32_t i = 0; i < THREAD_COUNT; ++i)
	{
		auto &data = thread_data[i];
		data.range_min = (SEED_COUNT / THREAD_COUNT) * i;
		data.range_max = (SEED_COUNT / THREAD_COUNT) * (i + 1);
		#ifdef SHINY_MODE
		handles[i] = CreateThread(NULL, 0, shiny_thread, &data, 0, NULL);
		#else
		handles[i] = CreateThread(NULL, 0, rewards_thread, &data, 0, NULL);
		#endif
		if (!handles[i])
		{
			printf("Failed to create thread #%u!\n", i);
			if (i > 0)
				WaitForMultipleObjects(i, handles.get(), TRUE, INFINITE);
			return 1;
		}
	}
	DWORD result = WaitForMultipleObjects(THREAD_COUNT, handles.get(), TRUE, INFINITE);
	QueryPerformanceCounter(&end_time);
	elapsed_microseconds.QuadPart = end_time.QuadPart - start_time.QuadPart;
	elapsed_microseconds.QuadPart *= 1000000;
	elapsed_microseconds.QuadPart /= frequency.QuadPart;
	bool success = result >= WAIT_OBJECT_0 && result < WAIT_OBJECT_0 + THREAD_COUNT;
	if (!success)
	{
		printf("WaitForMultipleObjects failed with error code %08X\n", result);
		return 1;
	}
	printf("Search finished after %llu.%llums\n", elapsed_microseconds.QuadPart / 1000, elapsed_microseconds.QuadPart % 1000);
	for (uint32_t i = 0; i < THREAD_COUNT; ++i)
	{
		auto &data = thread_data[i];
		for (auto &result : data.results)
		{
			#ifdef SHINY_MODE
			printf("seed=%08X %s\n", result.seed, pokemon_names[result.result]);
			#else
			printf("seed=%08X drops=%u\n", result.seed, result.result);
			#endif
		}
	}
	return 0;
}
