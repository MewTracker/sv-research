#pragma once

#include <cstdint>
#include <vector>
#include <functional>
#include "Stopwatch.h"
#include "EncounterTera9.h"
#include "PersonalTable9SV.h"
#include "Xoroshiro128Plus.h"
#include "PokemonNames.h"

class SeedFinder
{
public:
	using EncounterVisitor = void(const EncounterTera9& enc);
	static const int8_t AnyForm = -1;
	static const int8_t RareForm = -2;
	static const int8_t CommonForm = -3;

	struct SeedInfo
	{
		uint32_t seed;
		uint32_t drops;
		uint32_t ec;
		uint32_t pid;
		uint16_t species;
		uint16_t ability;
		uint16_t moves[4];
		uint8_t form;
		uint8_t iv[6];
		bool shiny;
		uint8_t gender;
		uint8_t nature;
		uint8_t tera_type;
		uint8_t stars;
		uint8_t height;
		uint8_t weight;
		uint8_t scale;
	};

	struct Reward
	{
		int32_t item_id;
		int32_t count;
	};

	struct BasicParams
	{
		Game game;
		int32_t event_id;
		int32_t event_group;
		int32_t stars;
		int32_t stage;
		int32_t raid_boost;
	};

	struct GroupInfo
	{
		std::vector<int32_t> dist;
		std::vector<int32_t> might;
	};

	SeedFinder();

	bool find_seeds();
	bool is_search_done();
	void set_drop_filter(int item_id, bool value);
	SeedInfo get_seed_info(uint32_t seed) const;
	bool use_filters() const;
	std::vector<Reward> get_all_rewards(uint32_t seed) const;
	BasicParams get_basic_params() const;
	void set_basic_params(const BasicParams& params);

	static bool initialize();
	static int get_star_count(uint32_t seed, int32_t progress, int32_t event_id, Game game);
	static void visit_encounters(int32_t event_id, std::function<EncounterVisitor> visitor);
	static bool is_mighty_event(int32_t event_id);
	static const GroupInfo* get_event_info(int32_t event_id);

	// Config
	uint32_t thread_count;

	// Query - Common
	Game game;
	uint32_t min_seed;
	uint32_t max_seed;
	int32_t event_id;
	int32_t event_group;
	int32_t stars;
	int32_t stage;
	int32_t raid_boost;

	// Query - Pokemon
	int32_t species;
	int8_t form;
	int32_t shiny;
	int32_t tera_type;
	int32_t ability;
	int32_t nature;
	int32_t gender;
	int8_t min_iv[6];
	int8_t max_iv[6];
	uint8_t min_height;
	uint8_t max_height;
	uint8_t min_weight;
	uint8_t max_weight;
	uint8_t min_scale;
	uint8_t max_scale;

	// Query - Items
	bool item_filters_active;
	int32_t drop_threshold;

	// Results
	Stopwatch time_taken;
	std::vector<uint32_t> seeds;

private:
	friend class Benchmarks;
	static const uint16_t ToxtricityId = 849;
	using EncounterLists = std::vector<std::vector<EncounterTera9>>;
	using EncounterListsEvents = std::vector<EncounterTera9>[_countof(event_names)];

	struct ThreadData
	{
		SeedFinder* finder;
		uint64_t range_min;
		uint64_t range_max;
		std::vector<uint32_t> results;
	};

	struct TroxicityNature
	{
		const uint8_t* table;
		size_t size;
	};

	template<bool f_is6>
	FORCEINLINE const EncounterTera9* get_encounter(uint32_t seed) const;

	template<EncounterType f_type, bool f_species, bool f_shiny, bool f_iv, bool f_advanced>
	FORCEINLINE bool check_pokemon(const EncounterTera9* enc, uint32_t seed) const;

	template<EncounterType f_type>
	FORCEINLINE bool check_rewards(const EncounterTera9* enc, uint32_t seed) const;

	template<EncounterType f_type>
	FORCEINLINE uint32_t get_rewards(const EncounterTera9* enc, uint32_t seed) const;

	template<EncounterType f_type, bool f_is6, bool f_species, bool f_shiny, bool f_iv, bool f_advanced, bool f_rewards>
	void worker_thread(ThreadData& data);

	template<EncounterType f_type, bool f_is6, bool f_species, bool f_shiny, bool f_iv, bool f_advanced, bool f_rewards>
	static DWORD WINAPI worker_thread_wrapper(LPVOID Parameter);

	bool use_iv_filters() const;
	bool use_pokemon_filters() const;
	bool use_item_filters() const;
	bool use_advanced_filters() const;
	bool use_size_filters() const;
	void find_seeds_thread();
	void init_value_map(bool map[256], uint8_t min_val, uint8_t max_val);
	bool compute_event_encounter_lookups();
	const EncounterTera9* get_encounter(uint32_t seed) const;
	const EncounterTera9* get_encounter_dist(uint32_t seed) const;
	FORCEINLINE const EncounterTera9* get_encounter_dist_lookup(uint32_t seed) const;

	static void initialize_event_encounters(const char* file_name, size_t encounter_size, EncounterType type, EncounterListsEvents& lists);
	static DWORD WINAPI find_seeds_thread_wrapper(LPVOID Parameter);
	static void compute_fast_lottery_lookups();
	static void compute_fast_encounter_lookups();
	static const RaidFixedRewards* get_fixed_drop_table(uint64_t table_name);
	static const RaidLotteryRewards* get_lottery_drop_table(uint64_t table_name, const uint8_t*& fast_lookup);
	static int32_t get_toxtricity_nature(Xoroshiro128Plus& gen, uint8_t form);
	FORCEINLINE static int16_t get_rate_total_base(int32_t version, size_t star);
	FORCEINLINE static int32_t get_star_count(Xoroshiro128Plus& gen, int32_t progress);
	FORCEINLINE static int32_t get_reward_count(int32_t random, int32_t stars);
	FORCEINLINE static uint32_t get_tera_type(uint32_t seed);
	FORCEINLINE static uint32_t get_tera_type(const EncounterTera9* enc, uint32_t seed);
	FORCEINLINE static uint32_t get_ability(Xoroshiro128Plus& gen, AbilityPermission permission, PersonalInfo9SV& personal_info);
	FORCEINLINE static uint32_t get_nature(Xoroshiro128Plus& gen, int32_t species, uint8_t form);
	FORCEINLINE static uint32_t get_gender(Xoroshiro128Plus& gen, int32_t ratio);

	HANDLE hFinderThread;
	int32_t item_filters_count;
	int32_t event_rand_rate;
	bool size_filters_in_use;
	std::vector<int8_t> target_drops;
	std::vector<int8_t> target_species;
	std::vector<const EncounterTera9*> event_encounter_lookup;
	alignas(16) int8_t min_iv_vec[16];
	alignas(16) int8_t max_iv_vec[16];
	bool height_map[256];
	bool weight_map[256];
	bool scale_map[256];

	static EncounterLists encounters;
	static EncounterListsEvents encounters_dist;
	static EncounterListsEvents encounters_might;
	static std::vector<std::vector<uint8_t>> fast_lottery_lookup;
	static std::vector<std::vector<uint8_t>> fast_encounter_lookup[2];
	static std::vector<uint8_t> fast_encounter_lookup_dist[2];
	static std::vector<uint8_t> fast_encounter_lookup_might[2];
	static GroupInfo event_groups[_countof(event_names)];
};

FORCEINLINE int16_t SeedFinder::get_rate_total_base(int32_t version, size_t star)
{
	static const int16_t rates[2][7] =
	{
		{ 0, 5800, 5300, 7400, 8800, 9100, 6500 },
		{ 0, 5800, 5300, 7400, 8700, 9100, 6500 },
	};
	assert(star > 0 && star < _countof(rates[0]));
	return rates[version][star];
};

FORCEINLINE int32_t SeedFinder::get_star_count(Xoroshiro128Plus& gen, int32_t progress)
{
	static const uint8_t star_count_lookup[5][100] =
	{
		{
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 2, 2, 2, 2, 2, 2, 2, 2, 2,
			2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		},
		{
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 2, 2, 2, 2, 2, 2, 2, 2, 2,
			2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
			2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
			2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
			2, 3, 3, 3, 3, 3, 3, 3, 3, 3,
			3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
			3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		},
		{
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 2, 2, 2, 2, 2, 2, 2, 2, 2,
			2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
			2, 3, 3, 3, 3, 3, 3, 3, 3, 3,
			3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
			3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
			3, 4, 4, 4, 4, 4, 4, 4, 4, 4,
			4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
			4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		},
		{
			3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
			3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
			3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
			3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
			3, 4, 4, 4, 4, 4, 4, 4, 4, 4,
			4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
			4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
			4, 4, 4, 4, 4, 4, 5, 5, 5, 5,
			5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
			5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		},
		{
			3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
			3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
			3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
			3, 4, 4, 4, 4, 4, 4, 4, 4, 4,
			4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
			4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
			4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
			4, 5, 5, 5, 5, 5, 5, 5, 5, 5,
			5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
			5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		},
	};
	return star_count_lookup[progress][gen.next_int(100)];
}

FORCEINLINE int32_t SeedFinder::get_reward_count(int32_t random, int32_t stars)
{
	static const int32_t reward_slots[8][5] =
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
	assert(stars > 0 && stars < _countof(reward_slots));
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
	return reward_slots[stars][random_lookup[random]];
}

FORCEINLINE uint32_t SeedFinder::get_tera_type(uint32_t seed)
{
	return (uint32_t)Xoroshiro128Plus(seed).next_int(18);
}

FORCEINLINE uint32_t SeedFinder::get_tera_type(const EncounterTera9* enc, uint32_t seed)
{
	return enc->tera_type != GemType::Random ? (uint32_t)enc->tera_type - 2 : get_tera_type(seed);
}

FORCEINLINE uint32_t SeedFinder::get_ability(Xoroshiro128Plus& gen, AbilityPermission permission, PersonalInfo9SV& personal_info)
{
	int ability_index;
	switch (permission)
	{
	case AbilityPermission::Any12H:
		ability_index = (int)gen.next_int(3) << 1;
		break;
	case AbilityPermission::Any12:
		ability_index = (int)gen.next_int(2) << 1;
		break;
	default:
		ability_index = (int)permission;
		break;
	}
	return personal_info.ability[ability_index >> 1];
}

FORCEINLINE uint32_t SeedFinder::get_nature(Xoroshiro128Plus& gen, int32_t species, uint8_t form)
{
	return (uint32_t)(species == ToxtricityId ? get_toxtricity_nature(gen, form) : (int32_t)gen.next_int(25));
}

FORCEINLINE uint32_t SeedFinder::get_gender(Xoroshiro128Plus& gen, int32_t ratio)
{
	static const uint8_t fixed_lookup[] =
	{
		0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 2,
	};
	static const uint8_t ratio_lookup_index[] =
	{
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 1,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 3,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		5, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	};
	static const uint8_t ratio_lookup_gender[5][100] =
	{
		{
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		},
		{
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		},
		{
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		},
		{
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		},
		{
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		},
	};
	uint32_t fixed_gender = fixed_lookup[ratio];
	if (fixed_gender <= 2)
		return fixed_gender;
	uint32_t rand100 = (uint32_t)gen.next_int(100);
	return ratio_lookup_gender[ratio_lookup_index[ratio]][rand100];
}

template<bool f_is6>
FORCEINLINE const EncounterTera9* SeedFinder::get_encounter(uint32_t seed) const
{
	Xoroshiro128Plus gen(seed);
	if constexpr (!f_is6)
	{
		if (get_star_count(gen, stage) != stars)
			return nullptr;
	}
	uint64_t total = get_rate_total_base(game, stars);
	uint64_t speciesroll = gen.next_int(total);
	return &encounters[stars][fast_encounter_lookup[game][stars][speciesroll]];
}

FORCEINLINE const EncounterTera9* SeedFinder::get_encounter_dist_lookup(uint32_t seed) const
{
	Xoroshiro128Plus gen(seed);
	gen.next_int(100);
	return event_encounter_lookup[gen.next_int(event_rand_rate)];
}

template<EncounterType f_type, bool f_species, bool f_shiny, bool f_iv, bool f_advanced>
FORCEINLINE bool SeedFinder::check_pokemon(const EncounterTera9* enc, uint32_t seed) const
{
	if constexpr (f_species)
	{
		if (!target_species[enc->pv_index])
			return false;
	}
	Xoroshiro128Plus gen(seed);
	uint32_t EC = (uint32_t)gen.next_int();
	uint32_t TIDSID = (uint32_t)gen.next_int();
	uint32_t PID = (uint32_t)gen.next_int();
	if constexpr (f_shiny)
	{
		bool is_shiny = (((PID >> 16) ^ (PID & 0xFFFF)) >> 4) == (((TIDSID >> 16) ^ (TIDSID & 0xFFFF)) >> 4);
		bool cond_shiny[] = { is_shiny, true, false };
		if (is_shiny != cond_shiny[shiny])
			return false;
	}
	int8_t ivs[16] = { -1, -1, -1, -1, -1, -1 };
	if constexpr (f_iv || f_advanced)
	{
		for (uint8_t i = 0; i < enc->flawless_iv_count; ++i)
		{
			int32_t index;
			do
			{
				index = (int32_t)gen.next_int(6);
			} while (ivs[index] != -1);
			ivs[index] = 31;
		}
		for (size_t i = 0; i < 6; ++i)
		{
			if (ivs[i] == -1)
				ivs[i] = (int8_t)gen.next_int(32);
		}
	}
	if constexpr (f_iv)
	{
		__m128i min_result = _mm_cmplt_epi8(*(__m128i*)ivs, *(__m128i*)min_iv_vec);
		__m128i max_result = _mm_cmpgt_epi8(*(__m128i*)ivs, *(__m128i*)max_iv_vec);
		int result = _mm_testz_si128(min_result, min_result) + _mm_testz_si128(max_result, max_result);
		if (result != 2)
			return false;
	}
	if constexpr (!f_advanced)
		return true;
	if (tera_type)
	{
		int32_t enc_tera_type;
		if constexpr (f_type == EncounterType::Dist)
			enc_tera_type = get_tera_type(enc, seed);
		else
			enc_tera_type = get_tera_type(seed);
		if (enc_tera_type != tera_type - 1)
			return false;
	}
	PersonalInfo9SV *personal_info = enc->personal_info;
	uint32_t current_ability = get_ability(gen, enc->ability, *personal_info);
	if (ability && current_ability != ability)
		return false;
	uint32_t current_gender = get_gender(gen, personal_info->gender);
	if (gender && current_gender != gender - 1)
		return false;
	uint32_t current_nature = get_nature(gen, enc->species, enc->form);
	if (nature && current_nature != nature - 1)
		return false;
	if (!size_filters_in_use)
		return true;
	uint8_t height = gen.next_byte();
	if (!height_map[height])
		return false;
	uint8_t weight = gen.next_byte();
	if (!weight_map[weight])
		return false;
	uint8_t scale = gen.next_byte();
	if (!scale_map[scale])
		return false;
	return true;
}

template<EncounterType f_type>
FORCEINLINE uint32_t SeedFinder::get_rewards(const EncounterTera9* enc, uint32_t seed) const
{
	uint32_t drop_counter = 0;

	auto& fixed_items = enc->fixed_drops->items;
	#define add_fixed_drop(n) drop_counter += fixed_items[n].num & target_drops[fixed_items[n].item_id]
	add_fixed_drop(0);
	add_fixed_drop(1);
	add_fixed_drop(2);
	add_fixed_drop(3);
	add_fixed_drop(4);
	add_fixed_drop(5);
	add_fixed_drop(6);
	if constexpr (f_type != EncounterType::Gem)
	{
		add_fixed_drop(7);
		add_fixed_drop(8);
		add_fixed_drop(9);
		add_fixed_drop(10);
		add_fixed_drop(11);
	}
	#undef add_fixed_drop

	Xoroshiro128Plus gen(seed);
	int32_t rate_total = enc->lottery_drops->rate_total;
	int32_t count = get_reward_count((int32_t)gen.next_int(100), enc->stars) + raid_boost;
	for (int32_t i = 0; i < count; ++i)
	{
		int32_t roll = (int32_t)gen.next_int((uint64_t)rate_total);
		auto& item = enc->lottery_drops->items[enc->lottery_lookup[roll]];
		drop_counter += item.num & target_drops[item.item_id];
	}
	return drop_counter;
}

template<EncounterType f_type>
FORCEINLINE bool SeedFinder::check_rewards(const EncounterTera9* enc, uint32_t seed) const
{
	uint32_t drops = get_rewards<f_type>(enc, seed);
	return drops >= drop_threshold;
}

template<EncounterType f_type, bool f_is6, bool f_species, bool f_shiny, bool f_iv, bool f_advanced, bool f_rewards>
void SeedFinder::worker_thread(ThreadData& data)
{
	for (uint64_t seed = data.range_min; seed < data.range_max; ++seed)
	{
		const EncounterTera9* enc;
		if constexpr (f_type == EncounterType::Gem)
		{
			enc = get_encounter<f_is6>(seed);
			if constexpr (!f_is6)
			{
				if (!enc)
					continue;
			}
		}
		else if constexpr (f_type == EncounterType::Dist)
		{
			enc = get_encounter_dist_lookup(seed);
			if (!enc)
				continue;
		}
		else
		{
			enc = &encounters_might[event_id][0];
		}
		if constexpr (f_type != EncounterType::Might && (f_species || f_shiny || f_iv || f_advanced))
		{
			if (!check_pokemon<f_type, f_species, f_shiny, f_iv, f_advanced>(enc, seed))
				continue;
		}
		if constexpr (f_rewards)
		{
			if (!check_rewards<f_type>(enc, seed))
				continue;
		}
		data.results.push_back(seed);
	}
}

template<EncounterType f_type, bool f_is6, bool f_species, bool f_shiny, bool f_iv, bool f_advanced, bool f_rewards>
static DWORD WINAPI SeedFinder::worker_thread_wrapper(LPVOID Parameter)
{
	ThreadData* data = (ThreadData*)Parameter;
	data->finder->worker_thread<f_type, f_is6, f_species, f_shiny, f_iv, f_advanced, f_rewards>(*data);
	return 0;
}
