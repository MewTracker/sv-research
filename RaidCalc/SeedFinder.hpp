#pragma once

#include <cstdint>
#include <vector>
#include "Stopwatch.hpp"
#include "EncounterTera9.hpp"

class SeedFinder
{
public:
	using EncounterVisitor = void(const EncounterTera9& enc);

	struct SeedInfo
	{
		uint32_t seed;
		int32_t species;
		bool shiny;
		uint32_t ec;
		uint32_t pid;
		uint8_t iv[6];
		int32_t drops;
	};

	SeedFinder();

	bool initialize();
	bool find_seeds();
	bool is_search_done();
	void set_drop_filter(int item_id, bool value);
	SeedInfo get_seed_info(uint32_t seed) const;
	void visit_encounters(std::function<EncounterVisitor> visitor) const;
	bool use_pokemon_filters() const;
	bool use_item_filters() const;
	bool use_filters() const;

	// Config
	uint32_t thread_count;

	// Query - Common
	Game game;
	uint32_t min_seed;
	uint32_t max_seed;
	int32_t stars;

	// Query - Pokemon
	int32_t species;
	int32_t shiny;
	int32_t tera_type;	// TODO: Add support
	int32_t ability;	// TODO: Add support
	int32_t nature;		// TODO: Add support
	int32_t gender;		// TODO: Add support
	int8_t min_iv[6];
	int8_t max_iv[6];

	// Query - Items
	bool item_filters_active;
	int32_t drop_threshold;

	// Results
	Stopwatch time_taken;
	std::vector<uint32_t> seeds;

private:
	static const int StoryProgress = 4;
	static const int RaidBoost = 0;

	struct ThreadData
	{
		SeedFinder* finder;
		uint64_t range_min;
		uint64_t range_max;
		std::vector<uint32_t> results;
	};

	std::vector<uint8_t> read_file(const char* filename) const;
	const RaidFixedRewards* get_fixed_drop_table(uint64_t table_name) const;
	const RaidLotteryRewards* get_lottery_drop_table(uint64_t table_name) const;
	int16_t get_rate_total_base(int32_t version, size_t star) const;
	void compute_fast_encounter_lookups();
	const EncounterTera9* get_encounter(uint32_t seed, int stage) const;
	int32_t get_reward_count(int32_t random, int32_t stars) const;
	uint32_t get_rewards(uint32_t seed, int progress, int raid_boost) const;
	bool check_pokemon(uint32_t seed) const;
	bool check_rewards(uint32_t seed) const;

	void combo_thread(ThreadData& data);
	void rewards_thread(ThreadData& data);
	void pokemon_thread(ThreadData& data);
	void find_seeds_thread();

	static DWORD WINAPI combo_thread_wrapper(LPVOID Parameter);
	static DWORD WINAPI rewards_thread_wrapper(LPVOID Parameter);
	static DWORD WINAPI pokemon_thread_wrapper(LPVOID Parameter);
	static DWORD WINAPI find_seeds_thread_wrapper(LPVOID Parameter);

	int32_t item_filters_count;
	alignas(16) int8_t min_iv_vec[16];
	alignas(16) int8_t max_iv_vec[16];
	std::vector<std::vector<EncounterTera9>> encounters;
	std::vector<std::vector<uint8_t>> fast_encounter_lookup[2];
	std::vector<int8_t> target_drops;
	HANDLE hFinderThread;
};
