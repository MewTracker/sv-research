#include <windows.h>
#include <xmmintrin.h>
#include <cassert>
#include <memory>
#include "SeedFinder.h"
#include "PokemonNames.h"
#include "Utils.h"

std::vector<std::vector<EncounterTera9>> SeedFinder::encounters;
std::vector<std::vector<uint8_t>> SeedFinder::fast_lottery_lookup;
std::vector<std::vector<uint8_t>> SeedFinder::fast_encounter_lookup[2];

SeedFinder::SeedFinder() :
	hFinderThread(NULL),
	thread_count(1),
	game(GameScarlet),
	min_seed(0),
	max_seed(0),
	stars(0),
	species(0),
	tera_type(0),
	ability(0),
	nature(0),
	gender(0),
	shiny(0),
	item_filters_active(false),
	drop_threshold(0),
	item_filters_count(0)
{
	memset(min_iv, 0, sizeof(min_iv));
	memset(max_iv, 0, sizeof(max_iv));
	memset(min_iv_vec, 0, sizeof(min_iv_vec));
	memset(max_iv_vec, 0, sizeof(max_iv_vec));
	target_drops.resize(20000 + 1);
	target_species.resize(_countof(pokemon_names));
}

bool SeedFinder::initialize()
{
	std::vector<uint8_t> encounter_data = get_resource_file(":/RaidCalc/encounter_gem_paldea.pkl");
	std::vector<uint8_t> reward_map = get_resource_file(":/RaidCalc/reward_map");
	if (encounter_data.empty() || reward_map.empty())
		return false;
	size_t encounter_count = encounter_data.size() / 0x18;
	size_t rewards_count = reward_map.size() / 0x10;
	if (encounter_count != rewards_count)
		return false;
	compute_fast_lottery_lookups();
	encounters.resize(7);
	uint64_t* rewards = (uint64_t*)reward_map.data();
	PersonalTable9SV& table = PersonalTable9SV::instance();
	for (size_t i = 0; i < encounter_count; ++i, rewards += 2)
	{
		auto enc = EncounterTera9(encounter_data.data() + i * 0x18);
		enc.fixed_drops = get_fixed_drop_table(rewards[0]);
		enc.lottery_drops = get_lottery_drop_table(rewards[1], enc.lottery_lookup);
		enc.personal_info = &table.get_form_entry(enc.species, enc.form);
		assert(enc.stars > 0 && enc.stars < 7);
		encounters[enc.stars].push_back(enc);
	}
	compute_fast_encounter_lookups();
	return true;
}

const RaidFixedRewards* SeedFinder::get_fixed_drop_table(uint64_t table_name)
{
	for (auto &table : fixed_rewards)
		if (table.table_name == table_name)
			return &table;
	return nullptr;
}

const RaidLotteryRewards* SeedFinder::get_lottery_drop_table(uint64_t table_name, const uint8_t*& fast_lookup)
{
	for (size_t i = 0; i < _countof(lottery_rewards); ++i)
	{
		auto& table = lottery_rewards[i];
		if (table.table_name == table_name)
		{
			fast_lookup = fast_lottery_lookup[i].data();
			return &table;
		}
	}
	return nullptr;
}

void SeedFinder::compute_fast_lottery_lookups()
{
	fast_lottery_lookup.resize(_countof(lottery_rewards));
	for (size_t i = 0; i < _countof(lottery_rewards); ++i)
	{
		auto& record = lottery_rewards[i];
		for (int32_t roll = 0; roll < record.rate_total; ++roll)
		{
			int32_t tmp_roll = roll;
			for (uint8_t j = 0; j < _countof(record.items); ++j)
			{
				auto& item = record.items[j];
				if (tmp_roll < item.rate)
				{
					fast_lottery_lookup[i].push_back(j);
					break;
				}
				tmp_roll -= item.rate;
			}
		}
	}
}

void SeedFinder::compute_fast_encounter_lookups()
{
	for (int32_t ver = GameScarlet; ver <= GameViolet; ++ver)
	{
		auto& lookup = fast_encounter_lookup[ver];
		lookup.resize(encounters.size());
		for (size_t stars = 1; stars < encounters.size(); ++stars)
		{
			uint64_t total = get_rate_total_base(ver, stars);
			for (uint64_t speciesroll = 0; speciesroll < total; ++speciesroll)
			{
				for (size_t i = 0; i < encounters[stars].size(); ++i)
				{
					EncounterTera9& enc = encounters[stars][i];
					int16_t minimum = enc.rand_rate_min[ver];
					if (minimum >= 0 && (uint32_t)((int32_t)speciesroll - minimum) < enc.rand_rate)
					{
						assert(i <= 0xFF);
						lookup[stars].push_back((uint8_t)i);
						break;
					}
				}
			}
		}
	}
}

const EncounterTera9* SeedFinder::get_encounter(uint32_t seed) const
{
	if (stars == 6)
		return get_encounter<true>(seed);
	return get_encounter<false>(seed);
}

std::vector<SeedFinder::Reward> SeedFinder::get_all_rewards(uint32_t seed) const
{
	std::vector<SeedFinder::Reward> rewards;
	const EncounterTera9* enc = get_encounter(seed);

	auto& fixed_items = enc->fixed_drops->items;
	for (auto& item : enc->fixed_drops->items)
		if (item.item_id)
			rewards.push_back({ item.item_id, item.num });
	Xoroshiro128Plus gen(seed);

	int32_t rate_total = enc->lottery_drops->rate_total;
	int32_t count = get_reward_count((int32_t)gen.next_int(100), enc->stars) + raid_boost;
	for (int32_t i = 0; i < count; ++i)
	{
		int32_t roll = (int32_t)gen.next_int((uint64_t)rate_total);
		auto& item = enc->lottery_drops->items[enc->lottery_lookup[roll]];
		if (item.item_id)
			rewards.push_back({ item.item_id, item.num });
	}

	return rewards;
}

void SeedFinder::find_seeds_thread()
{
	auto handles = std::make_unique<HANDLE[]>(thread_count);
	auto thread_data = std::make_unique<ThreadData[]>(thread_count);
	uint64_t seed_count = max_seed - min_seed + 1ULL;
	uint64_t seed_chunk = seed_count / thread_count;
	bool f_is6 = stars == 6;
	bool f_species = species != 0;
	bool f_shiny = shiny != 0;
	bool f_iv = use_iv_filters();
	bool f_advanced = use_advanced_filters();
	bool f_rewards = use_item_filters();
	static const LPTHREAD_START_ROUTINE workers[] =
	{
		worker_thread_wrapper<false, false, false, false, false, false>,
		worker_thread_wrapper<false, false, false, false, false, true>,
		worker_thread_wrapper<false, false, false, false, true, false>,
		worker_thread_wrapper<false, false, false, false, true, true>,
		worker_thread_wrapper<false, false, false, true, false, false>,
		worker_thread_wrapper<false, false, false, true, false, true>,
		worker_thread_wrapper<false, false, false, true, true, false>,
		worker_thread_wrapper<false, false, false, true, true, true>,
		worker_thread_wrapper<false, false, true, false, false, false>,
		worker_thread_wrapper<false, false, true, false, false, true>,
		worker_thread_wrapper<false, false, true, false, true, false>,
		worker_thread_wrapper<false, false, true, false, true, true>,
		worker_thread_wrapper<false, false, true, true, false, false>,
		worker_thread_wrapper<false, false, true, true, false, true>,
		worker_thread_wrapper<false, false, true, true, true, false>,
		worker_thread_wrapper<false, false, true, true, true, true>,
		worker_thread_wrapper<false, true, false, false, false, false>,
		worker_thread_wrapper<false, true, false, false, false, true>,
		worker_thread_wrapper<false, true, false, false, true, false>,
		worker_thread_wrapper<false, true, false, false, true, true>,
		worker_thread_wrapper<false, true, false, true, false, false>,
		worker_thread_wrapper<false, true, false, true, false, true>,
		worker_thread_wrapper<false, true, false, true, true, false>,
		worker_thread_wrapper<false, true, false, true, true, true>,
		worker_thread_wrapper<false, true, true, false, false, false>,
		worker_thread_wrapper<false, true, true, false, false, true>,
		worker_thread_wrapper<false, true, true, false, true, false>,
		worker_thread_wrapper<false, true, true, false, true, true>,
		worker_thread_wrapper<false, true, true, true, false, false>,
		worker_thread_wrapper<false, true, true, true, false, true>,
		worker_thread_wrapper<false, true, true, true, true, false>,
		worker_thread_wrapper<false, true, true, true, true, true>,
		worker_thread_wrapper<true, false, false, false, false, false>,
		worker_thread_wrapper<true, false, false, false, false, true>,
		worker_thread_wrapper<true, false, false, false, true, false>,
		worker_thread_wrapper<true, false, false, false, true, true>,
		worker_thread_wrapper<true, false, false, true, false, false>,
		worker_thread_wrapper<true, false, false, true, false, true>,
		worker_thread_wrapper<true, false, false, true, true, false>,
		worker_thread_wrapper<true, false, false, true, true, true>,
		worker_thread_wrapper<true, false, true, false, false, false>,
		worker_thread_wrapper<true, false, true, false, false, true>,
		worker_thread_wrapper<true, false, true, false, true, false>,
		worker_thread_wrapper<true, false, true, false, true, true>,
		worker_thread_wrapper<true, false, true, true, false, false>,
		worker_thread_wrapper<true, false, true, true, false, true>,
		worker_thread_wrapper<true, false, true, true, true, false>,
		worker_thread_wrapper<true, false, true, true, true, true>,
		worker_thread_wrapper<true, true, false, false, false, false>,
		worker_thread_wrapper<true, true, false, false, false, true>,
		worker_thread_wrapper<true, true, false, false, true, false>,
		worker_thread_wrapper<true, true, false, false, true, true>,
		worker_thread_wrapper<true, true, false, true, false, false>,
		worker_thread_wrapper<true, true, false, true, false, true>,
		worker_thread_wrapper<true, true, false, true, true, false>,
		worker_thread_wrapper<true, true, false, true, true, true>,
		worker_thread_wrapper<true, true, true, false, false, false>,
		worker_thread_wrapper<true, true, true, false, false, true>,
		worker_thread_wrapper<true, true, true, false, true, false>,
		worker_thread_wrapper<true, true, true, false, true, true>,
		worker_thread_wrapper<true, true, true, true, false, false>,
		worker_thread_wrapper<true, true, true, true, false, true>,
		worker_thread_wrapper<true, true, true, true, true, false>,
		worker_thread_wrapper<true, true, true, true, true, true>,
	};
	int worker_index = ((int)f_is6 << 5) | ((int)f_species << 4) | ((int)f_shiny << 3) | ((int)f_iv << 2) | ((int)f_advanced << 1) | ((int)f_rewards << 0);
	LPTHREAD_START_ROUTINE proc = workers[worker_index];
	time_taken.start();
	for (uint32_t i = 0; i < thread_count; ++i)
	{
		auto& data = thread_data[i];
		data.finder = this;
		data.range_min = min_seed + seed_chunk * i;
		data.range_max = min_seed + seed_chunk * (i + 1);
		if (i == thread_count - 1)
			data.range_max = min_seed + seed_count;
		handles[i] = CreateThread(NULL, 0, proc, &data, 0, NULL);
		if (!handles[i])
		{
			if (i > 0)
			{
				WaitForMultipleObjects(i, handles.get(), TRUE, INFINITE);
				for (int32_t j = 0; j < i; ++j)
					CloseHandle(handles[j]);
			}
			time_taken.stop();
			return;
		}
	}
	DWORD result = WaitForMultipleObjects(thread_count, handles.get(), TRUE, INFINITE);
	time_taken.stop();
	for (uint32_t i = 0; i < thread_count; ++i)
		CloseHandle(handles[i]);
	bool success = result >= WAIT_OBJECT_0 && result < WAIT_OBJECT_0 + thread_count;
	if (!success)
		return;
	for (uint32_t i = 0; i < thread_count; ++i)
	{
		auto& data = thread_data[i];
		seeds.insert(seeds.end(), data.results.begin(), data.results.end());
	}
}

DWORD WINAPI SeedFinder::find_seeds_thread_wrapper(LPVOID Parameter)
{
	SeedFinder* finder = (SeedFinder*)Parameter;
	finder->find_seeds_thread();
	return 0;
}

bool SeedFinder::find_seeds()
{
	seeds.clear();
	memcpy(min_iv_vec, min_iv, sizeof(min_iv));
	memcpy(max_iv_vec, max_iv, sizeof(max_iv));
	if (species != 0)
	{
		memset(target_species.data(), 0, target_species.size());
		target_species[species] = 1;
	}
	else
	{
		memset(target_species.data(), 1, target_species.size());
	}
	hFinderThread = CreateThread(NULL, 0, find_seeds_thread_wrapper, this, 0, NULL);
	if (!hFinderThread)
		return false;
	return true;
}

bool SeedFinder::is_search_done()
{
	if (!hFinderThread)
		return true;
	DWORD wait_result = WaitForSingleObject(hFinderThread, 0);
	if (wait_result != WAIT_OBJECT_0)
		return false;
	CloseHandle(hFinderThread);
	hFinderThread = NULL;
	return true;
}

void SeedFinder::set_drop_filter(int item_id, bool value)
{
	auto& drop_value = target_drops[item_id];
	if (value)
	{
		if (!drop_value)
			++item_filters_count;
		drop_value = -1;
	}
	else
	{
		if (drop_value)
			--item_filters_count;
		drop_value = 0;
	}
}

bool SeedFinder::use_advanced_filters() const
{
	return tera_type != 0 || ability != 0 || nature != 0 || gender != 0;
}

bool SeedFinder::use_iv_filters() const
{
	for (auto iv : min_iv)
		if (iv != 0)
			return true;
	for (auto iv : max_iv)
		if (iv != 31)
			return true;
	return false;
}

bool SeedFinder::use_pokemon_filters() const
{
	if (species != 0 || shiny != 0)
		return true;
	if (use_iv_filters() || use_advanced_filters())
		return true;
	return false;
}

bool SeedFinder::use_item_filters() const
{
	return item_filters_active && item_filters_count > 0;
}

bool SeedFinder::use_filters() const
{
	return use_pokemon_filters() || use_item_filters();
}

SeedFinder::SeedInfo SeedFinder::get_seed_info(uint32_t seed) const
{
	SeedInfo info;
	info.seed = seed;
	info.stars = stars < 6 ? get_star_count(seed, story_progress) : 6;
	info.tera_type = (uint8_t)get_tera_type(seed);
	Xoroshiro128Plus gen(seed);
	info.ec = (uint32_t)gen.next_int();
	uint32_t TIDSID = (uint32_t)gen.next_int();
	info.pid = (uint32_t)gen.next_int();
	info.shiny = (((info.pid >> 16) ^ (info.pid & 0xFFFF)) >> 4) == (((TIDSID >> 16) ^ (TIDSID & 0xFFFF)) >> 4);
	const EncounterTera9* enc = get_encounter(seed);
	info.species = enc->species;
	int8_t ivs[6] = { -1, -1, -1, -1, -1, -1 };
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
			ivs[i] = (int8_t)gen.next_int(32);
	}
	memcpy(info.iv, ivs, sizeof(info.iv));
	info.ability = (uint16_t)get_ability(gen, enc->ability, *enc->personal_info);
	info.gender = (uint8_t)get_gender(gen, enc->personal_info->gender);
	info.nature = (uint8_t)get_nature(gen, info.species, enc->form);
	memcpy(info.moves, enc->moves, sizeof(info.moves));
	if (use_item_filters())
		info.drops = get_rewards(enc, seed);
	else
		info.drops = 0;
	return info;
}

int32_t SeedFinder::get_toxtricity_nature(Xoroshiro128Plus& gen, uint8_t form)
{
	static const uint8_t toxtricity_nature0[] = { 3, 4, 2, 8, 9, 19, 22, 11, 13, 14, 0, 6, 24 };
	static const uint8_t toxtricity_nature1[] = { 1, 5, 7, 10, 12, 15, 16, 17, 18, 20, 21, 23 };
	static const TroxicityNature toxtricity_natures[] =
	{
		{ toxtricity_nature0, _countof(toxtricity_nature0) },
		{ toxtricity_nature1, _countof(toxtricity_nature1) },
	};
	auto& form_record = toxtricity_natures[form];
	return form_record.table[gen.next_int(form_record.size)];
}

void SeedFinder::visit_encounters(std::function<EncounterVisitor> visitor)
{
	for (auto& enc_vector : encounters)
		for (auto& enc : enc_vector)
			visitor(enc);
}

int SeedFinder::get_star_count(uint32_t seed, int32_t progress)
{
	Xoroshiro128Plus gen(seed);
	return get_star_count(gen, progress);
}

SeedFinder::BasicParams SeedFinder::get_basic_params() const
{
	BasicParams params;
	params.game = game;
	params.stars = stars;
	params.story_progress = story_progress;
	params.raid_boost = raid_boost;
	return params;
}

void SeedFinder::set_basic_params(const BasicParams& params)
{
	game = params.game;
	stars = params.stars;
	story_progress = params.story_progress;
	raid_boost = params.raid_boost;
}
