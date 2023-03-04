#include <windows.h>
#include <xmmintrin.h>
#include <cassert>
#include <memory>
#include <set>
#include "SeedFinder.h"
#include "FormUtils.h"
#include "Utils.h"

SeedFinder::EncounterLists SeedFinder::encounters;
SeedFinder::EncounterListsEvents SeedFinder::encounters_dist;
SeedFinder::EncounterListsEvents SeedFinder::encounters_might;
std::vector<std::vector<uint8_t>> SeedFinder::fast_lottery_lookup;
std::vector<std::vector<uint8_t>> SeedFinder::fast_encounter_lookup[2];
std::vector<uint8_t> SeedFinder::fast_encounter_lookup_dist[2];
std::vector<uint8_t> SeedFinder::fast_encounter_lookup_might[2];
SeedFinder::GroupInfo SeedFinder::event_groups[_countof(event_names)];

SeedFinder::SeedFinder() :
	hFinderThread(NULL),
	thread_count(1),
	game(GameScarlet),
	min_seed(0),
	max_seed(0),
	event_id(-1),
	event_group(0),
	stars(0),
	species(0),
	form(AnyForm),
	tera_type(0),
	ability(0),
	nature(0),
	gender(0),
	shiny(0),
	min_height(0),
	max_height(255),
	min_weight(0),
	max_weight(255),
	min_scale(0),
	max_scale(255),
	item_filters_active(false),
	drop_threshold(0),
	item_filters_count(0),
	event_rand_rate(0),
	size_filters_in_use(false)
{
	memset(min_iv, 0, sizeof(min_iv));
	memset(max_iv, 0, sizeof(max_iv));
	memset(min_iv_vec, 0, sizeof(min_iv_vec));
	memset(max_iv_vec, 0, sizeof(max_iv_vec));
	memset(height_map, 0, sizeof(height_map));
	memset(weight_map, 0, sizeof(weight_map));
	memset(scale_map, 0, sizeof(scale_map));
	target_drops.resize(20000 + 1);
	target_species.resize(PersonalTable9SV::instance().size());
}

bool SeedFinder::initialize()
{
	std::vector<uint8_t> encounter_data = get_resource_file(":/resources/encounter_gem_paldea.pkl");
	std::vector<uint8_t> reward_map = get_resource_file(":/resources/reward_map");
	if (encounter_data.empty() || reward_map.empty())
		return false;
	size_t encounter_count = encounter_data.size() / EncounterTera9::SizeGem;
	size_t rewards_count = reward_map.size() / 0x10;
	if (encounter_count != rewards_count)
		return false;
	compute_fast_lottery_lookups();
	encounters.resize(7);
	uint64_t* rewards = (uint64_t*)reward_map.data();
	PersonalTable9SV& table = PersonalTable9SV::instance();
	for (size_t i = 0; i < encounter_count; ++i, rewards += 2)
	{
		auto enc = EncounterTera9(encounter_data.data() + i * EncounterTera9::SizeGem, EncounterType::Gem);
		enc.fixed_drops = get_fixed_drop_table(rewards[0]);
		enc.lottery_drops = get_lottery_drop_table(rewards[1], enc.lottery_lookup);
		enc.pv_index = table.get_form_index(enc.species, enc.form);
		enc.personal_info = &table[enc.pv_index];
		assert(enc.stars > 0 && enc.stars < 7);
		assert(enc.tera_type == GemType::Random);
		encounters[enc.stars].push_back(enc);
	}
	compute_fast_encounter_lookups();
	initialize_event_encounters(":/resources/encounter_dist_paldea.pklex", EncounterTera9::SizeDist, EncounterType::Dist, encounters_dist);
	initialize_event_encounters(":/resources/encounter_might_paldea.pklex", EncounterTera9::SizeMight, EncounterType::Might, encounters_might);
	for (int32_t i = 0; i < _countof(event_names); ++i)
	{
		GroupInfo &info = event_groups[i];
		std::set<int32_t> dist_groups;
		int32_t might_group = -1;
		visit_encounters(i, [&](const EncounterTera9 &enc) {
			if (enc.stars == 7)
			{
				assert(might_group == -1 || might_group == enc.group);
				might_group = enc.group;
			}
			else
			{
				dist_groups.insert(enc.group);
			}
		});
		info.might.push_back(might_group);
		for (auto group : dist_groups)
			info.dist.push_back(group);
	}
	#ifdef _DEBUG
	for (int32_t t_event_id = 0; t_event_id < _countof(event_names); ++t_event_id)
	{
		for (auto t_group : event_groups[t_event_id].dist)
		{
			for (int32_t t_game = GameScarlet; t_game <= GameViolet; ++t_game)
			{
				for (int32_t t_stage = 0; t_stage < 4; ++t_stage)
				{
					for (int32_t t_stars = 1; t_stars <= 6; ++t_stars)
					{
						int16_t total = -1;
						for (auto &enc : encounters_dist[t_event_id])
						{
							if (enc.stars != t_stars || enc.group != t_group)
								continue;
							auto &rand_rate_data = enc.rand_rate_event[t_stage][t_game];
							if (rand_rate_data.total == 0)
								continue;
							if (total == -1)
							{
								total = rand_rate_data.total;
							}
							else if (total != rand_rate_data.total)
							{
								qFatal("Rand rate total discrepancy detected: %d != %d", total, rand_rate_data.total);
							}
						}
					}
				}
			}
		}
	}
	#endif
	return true;
}

void SeedFinder::initialize_event_encounters(const char *file_name, size_t encounter_size, EncounterType type, EncounterListsEvents& lists)
{
	size_t offset = 0;
	std::vector<uint8_t> encounter_data = get_resource_file(file_name);
	PersonalTable9SV& table = PersonalTable9SV::instance();
	for (size_t i = 0; i < _countof(lists); ++i)
	{
		assert(*(uint32_t*)&encounter_data.data()[offset] == i);
		uint32_t encounter_count = *(uint32_t*)&encounter_data.data()[offset + sizeof(uint32_t)];
		offset += sizeof(uint32_t) * 2;
		for (uint32_t j = 0; j < encounter_count; ++j)
		{
			auto enc = EncounterTera9(encounter_data.data() + offset, type);
			enc.fixed_drops = get_fixed_drop_table(enc.fixed_table_id);
			enc.lottery_drops = get_lottery_drop_table(enc.lottery_table_id, enc.lottery_lookup);
			enc.pv_index = table.get_form_index(enc.species, enc.form);
			enc.personal_info = &table[enc.pv_index];
			lists[i].push_back(enc);
			assert(enc.stars > 0 && enc.stars < 8);
			assert(enc.tera_type != GemType::Default);
			offset += encounter_size;
		}
	}
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

bool SeedFinder::compute_event_encounter_lookups()
{
	event_encounter_lookup.clear();
	event_rand_rate = 0;
	for (auto& enc : encounters_dist[event_id])
	{
		if (enc.stars != stars || enc.group != event_group)
			continue;
		auto& rand_rate_data = enc.rand_rate_event[stage][game];
		if (rand_rate_data.total == 0)
			continue;
		event_rand_rate = rand_rate_data.total;
		break;
	}
	if (event_rand_rate == 0)
		return false;
	event_encounter_lookup.reserve(event_rand_rate);
	for (int32_t val = 0; val < event_rand_rate; ++val)
	{
		const EncounterTera9* result = nullptr;
		for (auto &enc : encounters_dist[event_id])
		{
			if (enc.stars != stars || enc.group != event_group)
				continue;
			auto &rand_rate_data = enc.rand_rate_event[stage][game];
			if (rand_rate_data.total == 0)
				continue;
			if ((uint32_t)((int32_t)val - rand_rate_data.min) < enc.rand_rate)
			{
				result = &enc;
				break;
			}
		}
		event_encounter_lookup.push_back(result);
	}
	return true;
}

const EncounterTera9* SeedFinder::get_encounter(uint32_t seed) const
{
	if (event_id < 0)
	{
		if (stars == 6)
			return get_encounter<true>(seed);
		return get_encounter<false>(seed);
	}
	if (stars != 7)
		return get_encounter_dist(seed);
	if (!encounters_might[event_id].empty())
		return &encounters_might[event_id][0];
	return nullptr;
}

const EncounterTera9* SeedFinder::get_encounter_dist(uint32_t seed) const
{
	Xoroshiro128Plus gen(seed);
	gen.next_int(100);
	for (auto& enc : encounters_dist[event_id])
	{
		if (enc.stars != stars || enc.group != event_group)
			continue;
		auto& rand_rate_data = enc.rand_rate_event[stage][game];
		if (rand_rate_data.total == 0)
			continue;
		uint64_t val = Xoroshiro128Plus(gen).next_int(rand_rate_data.total);
		if ((uint32_t)((int32_t)val - rand_rate_data.min) < enc.rand_rate)
			return &enc;
	}
	return nullptr;
}

std::vector<SeedFinder::Reward> SeedFinder::get_all_rewards(uint32_t seed) const
{
	std::vector<SeedFinder::Reward> rewards;
	const EncounterTera9* enc = get_encounter(seed);

	auto& fixed_items = enc->fixed_drops->items;
	for (auto& item : enc->fixed_drops->items)
		if (item.item_id)
			rewards.push_back({ item.item_id, item.num });

	std::vector<SeedFinder::Reward> lottery;
	Xoroshiro128Plus gen(seed);
	int32_t rate_total = enc->lottery_drops->rate_total;
	int32_t count = get_reward_count((int32_t)gen.next_int(100), enc->stars) + raid_boost;
	for (int32_t i = 0; i < count; ++i)
	{
		int32_t roll = (int32_t)gen.next_int((uint64_t)rate_total);
		auto& item = enc->lottery_drops->items[enc->lottery_lookup[roll]];
		if (item.item_id)
			lottery.push_back({ item.item_id, item.num });
	}

	static const int32_t rare_rewards[] = { 4, 645, 1606, 1904, 1905, 1906, 1907, 1908, 795 };
	std::vector<SeedFinder::Reward> rares, commons;
	for (auto reward : lottery)
	{
		bool is_rare = false;
		for (auto rare_id : rare_rewards)
		{
			if (reward.item_id == rare_id)
			{
				is_rare = true;
				break;
			}
		}
		if (is_rare)
			rares.push_back(reward);
		else
			commons.push_back(reward);
	}
	rewards.insert(rewards.begin(), rares.begin(), rares.end());
	rewards.insert(rewards.end(), commons.begin(), commons.end());
	return rewards;
}

void SeedFinder::find_seeds_thread()
{
	auto handles = std::make_unique<HANDLE[]>(thread_count);
	auto thread_data = std::make_unique<ThreadData[]>(thread_count);
	uint64_t seed_count = max_seed - min_seed + 1ULL;
	uint64_t seed_chunk = seed_count / thread_count;
	EncounterType f_type = event_id < 0 ? EncounterType::Gem : EncounterType::Dist;
	if (stars == 7)
		f_type = EncounterType::Might;
	bool f_is6 = stars == 6;
	bool f_species = species != 0;
	bool f_shiny = shiny != 0;
	bool f_iv = use_iv_filters();
	bool f_advanced = use_advanced_filters();
	bool f_rewards = use_item_filters();
	static const LPTHREAD_START_ROUTINE workers_gem[] =
	{
		worker_thread_wrapper<EncounterType::Gem, false, false, false, false, false, false>,
		worker_thread_wrapper<EncounterType::Gem, false, false, false, false, false, true>,
		worker_thread_wrapper<EncounterType::Gem, false, false, false, false, true, false>,
		worker_thread_wrapper<EncounterType::Gem, false, false, false, false, true, true>,
		worker_thread_wrapper<EncounterType::Gem, false, false, false, true, false, false>,
		worker_thread_wrapper<EncounterType::Gem, false, false, false, true, false, true>,
		worker_thread_wrapper<EncounterType::Gem, false, false, false, true, true, false>,
		worker_thread_wrapper<EncounterType::Gem, false, false, false, true, true, true>,
		worker_thread_wrapper<EncounterType::Gem, false, false, true, false, false, false>,
		worker_thread_wrapper<EncounterType::Gem, false, false, true, false, false, true>,
		worker_thread_wrapper<EncounterType::Gem, false, false, true, false, true, false>,
		worker_thread_wrapper<EncounterType::Gem, false, false, true, false, true, true>,
		worker_thread_wrapper<EncounterType::Gem, false, false, true, true, false, false>,
		worker_thread_wrapper<EncounterType::Gem, false, false, true, true, false, true>,
		worker_thread_wrapper<EncounterType::Gem, false, false, true, true, true, false>,
		worker_thread_wrapper<EncounterType::Gem, false, false, true, true, true, true>,
		worker_thread_wrapper<EncounterType::Gem, false, true, false, false, false, false>,
		worker_thread_wrapper<EncounterType::Gem, false, true, false, false, false, true>,
		worker_thread_wrapper<EncounterType::Gem, false, true, false, false, true, false>,
		worker_thread_wrapper<EncounterType::Gem, false, true, false, false, true, true>,
		worker_thread_wrapper<EncounterType::Gem, false, true, false, true, false, false>,
		worker_thread_wrapper<EncounterType::Gem, false, true, false, true, false, true>,
		worker_thread_wrapper<EncounterType::Gem, false, true, false, true, true, false>,
		worker_thread_wrapper<EncounterType::Gem, false, true, false, true, true, true>,
		worker_thread_wrapper<EncounterType::Gem, false, true, true, false, false, false>,
		worker_thread_wrapper<EncounterType::Gem, false, true, true, false, false, true>,
		worker_thread_wrapper<EncounterType::Gem, false, true, true, false, true, false>,
		worker_thread_wrapper<EncounterType::Gem, false, true, true, false, true, true>,
		worker_thread_wrapper<EncounterType::Gem, false, true, true, true, false, false>,
		worker_thread_wrapper<EncounterType::Gem, false, true, true, true, false, true>,
		worker_thread_wrapper<EncounterType::Gem, false, true, true, true, true, false>,
		worker_thread_wrapper<EncounterType::Gem, false, true, true, true, true, true>,
		worker_thread_wrapper<EncounterType::Gem, true, false, false, false, false, false>,
		worker_thread_wrapper<EncounterType::Gem, true, false, false, false, false, true>,
		worker_thread_wrapper<EncounterType::Gem, true, false, false, false, true, false>,
		worker_thread_wrapper<EncounterType::Gem, true, false, false, false, true, true>,
		worker_thread_wrapper<EncounterType::Gem, true, false, false, true, false, false>,
		worker_thread_wrapper<EncounterType::Gem, true, false, false, true, false, true>,
		worker_thread_wrapper<EncounterType::Gem, true, false, false, true, true, false>,
		worker_thread_wrapper<EncounterType::Gem, true, false, false, true, true, true>,
		worker_thread_wrapper<EncounterType::Gem, true, false, true, false, false, false>,
		worker_thread_wrapper<EncounterType::Gem, true, false, true, false, false, true>,
		worker_thread_wrapper<EncounterType::Gem, true, false, true, false, true, false>,
		worker_thread_wrapper<EncounterType::Gem, true, false, true, false, true, true>,
		worker_thread_wrapper<EncounterType::Gem, true, false, true, true, false, false>,
		worker_thread_wrapper<EncounterType::Gem, true, false, true, true, false, true>,
		worker_thread_wrapper<EncounterType::Gem, true, false, true, true, true, false>,
		worker_thread_wrapper<EncounterType::Gem, true, false, true, true, true, true>,
		worker_thread_wrapper<EncounterType::Gem, true, true, false, false, false, false>,
		worker_thread_wrapper<EncounterType::Gem, true, true, false, false, false, true>,
		worker_thread_wrapper<EncounterType::Gem, true, true, false, false, true, false>,
		worker_thread_wrapper<EncounterType::Gem, true, true, false, false, true, true>,
		worker_thread_wrapper<EncounterType::Gem, true, true, false, true, false, false>,
		worker_thread_wrapper<EncounterType::Gem, true, true, false, true, false, true>,
		worker_thread_wrapper<EncounterType::Gem, true, true, false, true, true, false>,
		worker_thread_wrapper<EncounterType::Gem, true, true, false, true, true, true>,
		worker_thread_wrapper<EncounterType::Gem, true, true, true, false, false, false>,
		worker_thread_wrapper<EncounterType::Gem, true, true, true, false, false, true>,
		worker_thread_wrapper<EncounterType::Gem, true, true, true, false, true, false>,
		worker_thread_wrapper<EncounterType::Gem, true, true, true, false, true, true>,
		worker_thread_wrapper<EncounterType::Gem, true, true, true, true, false, false>,
		worker_thread_wrapper<EncounterType::Gem, true, true, true, true, false, true>,
		worker_thread_wrapper<EncounterType::Gem, true, true, true, true, true, false>,
		worker_thread_wrapper<EncounterType::Gem, true, true, true, true, true, true>,
	};
	static const LPTHREAD_START_ROUTINE workers_dist[] =
	{
		worker_thread_wrapper<EncounterType::Dist, false, false, false, false, false, false>,
		worker_thread_wrapper<EncounterType::Dist, false, false, false, false, false, true>,
		worker_thread_wrapper<EncounterType::Dist, false, false, false, false, true, false>,
		worker_thread_wrapper<EncounterType::Dist, false, false, false, false, true, true>,
		worker_thread_wrapper<EncounterType::Dist, false, false, false, true, false, false>,
		worker_thread_wrapper<EncounterType::Dist, false, false, false, true, false, true>,
		worker_thread_wrapper<EncounterType::Dist, false, false, false, true, true, false>,
		worker_thread_wrapper<EncounterType::Dist, false, false, false, true, true, true>,
		worker_thread_wrapper<EncounterType::Dist, false, false, true, false, false, false>,
		worker_thread_wrapper<EncounterType::Dist, false, false, true, false, false, true>,
		worker_thread_wrapper<EncounterType::Dist, false, false, true, false, true, false>,
		worker_thread_wrapper<EncounterType::Dist, false, false, true, false, true, true>,
		worker_thread_wrapper<EncounterType::Dist, false, false, true, true, false, false>,
		worker_thread_wrapper<EncounterType::Dist, false, false, true, true, false, true>,
		worker_thread_wrapper<EncounterType::Dist, false, false, true, true, true, false>,
		worker_thread_wrapper<EncounterType::Dist, false, false, true, true, true, true>,
		worker_thread_wrapper<EncounterType::Dist, false, true, false, false, false, false>,
		worker_thread_wrapper<EncounterType::Dist, false, true, false, false, false, true>,
		worker_thread_wrapper<EncounterType::Dist, false, true, false, false, true, false>,
		worker_thread_wrapper<EncounterType::Dist, false, true, false, false, true, true>,
		worker_thread_wrapper<EncounterType::Dist, false, true, false, true, false, false>,
		worker_thread_wrapper<EncounterType::Dist, false, true, false, true, false, true>,
		worker_thread_wrapper<EncounterType::Dist, false, true, false, true, true, false>,
		worker_thread_wrapper<EncounterType::Dist, false, true, false, true, true, true>,
		worker_thread_wrapper<EncounterType::Dist, false, true, true, false, false, false>,
		worker_thread_wrapper<EncounterType::Dist, false, true, true, false, false, true>,
		worker_thread_wrapper<EncounterType::Dist, false, true, true, false, true, false>,
		worker_thread_wrapper<EncounterType::Dist, false, true, true, false, true, true>,
		worker_thread_wrapper<EncounterType::Dist, false, true, true, true, false, false>,
		worker_thread_wrapper<EncounterType::Dist, false, true, true, true, false, true>,
		worker_thread_wrapper<EncounterType::Dist, false, true, true, true, true, false>,
		worker_thread_wrapper<EncounterType::Dist, false, true, true, true, true, true>,
	};
	LPTHREAD_START_ROUTINE proc = nullptr;
	switch (f_type)
	{
	case EncounterType::Gem:
	{
		int worker_index = ((int)f_is6 << 5) | ((int)f_species << 4) | ((int)f_shiny << 3) | ((int)f_iv << 2) | ((int)f_advanced << 1) | ((int)f_rewards << 0);
		proc = workers_gem[worker_index];
		break;
	}
	case EncounterType::Dist:
	{
		int worker_index = ((int)f_species << 4) | ((int)f_shiny << 3) | ((int)f_iv << 2) | ((int)f_advanced << 1) | ((int)f_rewards << 0);
		proc = workers_dist[worker_index];
		break;
	}
	case EncounterType::Might:
		proc = f_rewards
			? worker_thread_wrapper<EncounterType::Might, false, false, false, false, false, true>
			: worker_thread_wrapper<EncounterType::Might, false, false, false, false, false, false>;
		break;
	}
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
		if (form == RareForm || form == CommonForm)
		{
			bool find_rare = form == RareForm;
			for (auto seed : data.results)
			{
				Xoroshiro128Plus gen(seed);
				uint32_t EC = (uint32_t)gen.next_int();
				bool is_rare = (EC % 100) == 0;
				if (((int)find_rare ^ (int)is_rare) == 0)
					seeds.push_back(seed);
			}
		}
		else
		{
			seeds.insert(seeds.end(), data.results.begin(), data.results.end());
		}
	}
}

DWORD WINAPI SeedFinder::find_seeds_thread_wrapper(LPVOID Parameter)
{
	SeedFinder* finder = (SeedFinder*)Parameter;
	finder->find_seeds_thread();
	return 0;
}

void SeedFinder::init_value_map(bool map[256], uint8_t min_val, uint8_t max_val)
{
	for (size_t i = 0; i < 256; ++i)
		map[i] = i >= min_val && i <= max_val;
}

bool SeedFinder::find_seeds()
{
	seeds.clear();
	memcpy(min_iv_vec, min_iv, sizeof(min_iv));
	memcpy(max_iv_vec, max_iv, sizeof(max_iv));
	init_value_map(height_map, min_height, max_height);
	init_value_map(weight_map, min_weight, max_weight);
	init_value_map(scale_map, min_scale, max_scale);
	size_filters_in_use = use_size_filters();
	if (species != 0)
	{
		memset(target_species.data(), 0, target_species.size());
		auto& table = PersonalTable9SV::instance();
		if (form == AnyForm)
		{
			auto forms = FormUtils::get_forms(species, true);
			if (!forms.empty())
			{
				for (auto species_form : forms)
					target_species[table.get_form_index(species, species_form)] = 1;
			}
			else
			{
				target_species[table.get_form_index(species, 0)] = 1;
			}
		}
		else
		{
			target_species[table.get_form_index(species, form >= 0 ? form : 0)] = 1;
		}
	}
	else
	{
		memset(target_species.data(), 1, target_species.size());
	}
	if (event_id >= 0 && stars != 7)
	{
		if (!compute_event_encounter_lookups())
			return true;
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
	event_encounter_lookup.clear();
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

bool SeedFinder::use_size_filters() const
{
	return min_height != 0 || max_height != 255 || min_weight != 0 || max_weight != 255 || min_scale != 0 || max_scale != 255;
}

bool SeedFinder::use_advanced_filters() const
{
	return tera_type != 0 || ability != 0 || nature != 0 || gender != 0 || use_size_filters();
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
	if (stars == 7)
		return false;
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
	Xoroshiro128Plus gen(seed);
	info.seed = seed;
	info.ec = (uint32_t)gen.next_int();
	uint32_t TIDSID = (uint32_t)gen.next_int();
	info.pid = (uint32_t)gen.next_int();
	const EncounterTera9* enc = get_encounter(seed);
	if (!enc)
	{
		memset(&info, 0, sizeof(info));
		return info;
	}
	if (enc->type == EncounterType::Gem)
		info.stars = stars < 6 ? get_star_count(seed, stage, event_id, game) : 6;
	else
		info.stars = enc->stars;
	info.species = enc->species;
	info.form = enc->form;
	memcpy(info.moves, enc->moves, sizeof(info.moves));
	if (enc->type == EncounterType::Might)
	{
		assert(enc->iv_fixed);
		assert(enc->ability == AbilityPermission::OnlyFirst ||
			   enc->ability == AbilityPermission::OnlySecond ||
			   enc->ability == AbilityPermission::OnlyHidden);
		assert(enc->gender >= 0);
		info.tera_type = (uint8_t)enc->tera_type - 2;
		info.shiny = false;
		memcpy(info.iv, enc->iv, sizeof(info.iv));
		info.ability = (uint16_t)get_ability(gen, enc->ability, *enc->personal_info);
		info.gender = enc->gender;
		info.nature = enc->nature;
		info.height = gen.next_byte();
		info.weight = gen.next_byte();
		switch (enc->scale_type)
		{
		case SizeType::RANDOM:
			info.scale = gen.next_byte();
			break;
		case SizeType::XS:
			info.scale = (uint8_t)gen.next_int(0x10);
			break;
		case SizeType::S:
			info.scale = (uint8_t)(gen.next_int(0x20) + 0x10);
			break;
		case SizeType::M:
			info.scale = (uint8_t)(gen.next_int(0xA0) + 0x30);
			break;
		case SizeType::L:
			info.scale = (uint8_t)(gen.next_int(0x20) + 0xD0);
			break;
		case SizeType::XL:
			info.scale = (uint8_t)(gen.next_int(0x10) + 0xF0);
			break;
		case SizeType::VALUE:
			info.scale = enc->scale;
			break;
		default:
			info.scale = 0;
			break;
		}
	}
	else
	{
		info.tera_type = (uint8_t)get_tera_type(enc, seed);
		info.shiny = (((info.pid >> 16) ^ (info.pid & 0xFFFF)) >> 4) == (((TIDSID >> 16) ^ (TIDSID & 0xFFFF)) >> 4);
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
		info.height = gen.next_byte();
		info.weight = gen.next_byte();
		info.scale = gen.next_byte();
	}
	if (use_item_filters())
		info.drops = event_id < 0 ? get_rewards<EncounterType::Gem>(enc, seed) : get_rewards<EncounterType::Dist>(enc, seed);
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

void SeedFinder::visit_encounters(int32_t event_id, std::function<EncounterVisitor> visitor)
{
	if (event_id < 0)
	{
		for (auto& enc_vector : encounters)
			for (auto& enc : enc_vector)
				visitor(enc);
	}
	else
	{
		for (auto& enc : encounters_dist[event_id])
			visitor(enc);
		for (auto& enc : encounters_might[event_id])
			visitor(enc);
	}
}

int SeedFinder::get_star_count(uint32_t seed, int32_t progress, int32_t event_id, Game game)
{
	if (event_id < 0)
	{
		Xoroshiro128Plus gen(seed);
		return get_star_count(gen, progress);
	}
	for (auto& enc : encounters_dist[event_id])
	{
		Xoroshiro128Plus gen(seed);
		gen.next_int(100);
		auto& rand_rate_data = enc.rand_rate_event[progress][game];
		if (rand_rate_data.total == 0)
			continue;
		uint64_t val = gen.next_int(rand_rate_data.total);
		if ((uint32_t)((int32_t)val - rand_rate_data.min) < enc.rand_rate)
			return enc.stars;
	}
	return 0;
}

SeedFinder::BasicParams SeedFinder::get_basic_params() const
{
	BasicParams params;
	params.game = game;
	params.event_id = event_id;
	params.event_group = event_group;
	params.stars = stars;
	params.stage = stage;
	params.raid_boost = raid_boost;
	return params;
}

void SeedFinder::set_basic_params(const BasicParams& params)
{
	game = params.game;
	event_id = params.event_id;
	stars = params.stars;
	stage = params.stage;
	raid_boost = params.raid_boost;
}

bool SeedFinder::is_mighty_event(int32_t event_id)
{
	if (event_id < 0 || event_id >= _countof(encounters_might))
		return false;
	return !encounters_might[event_id].empty();
}

const SeedFinder::GroupInfo* SeedFinder::get_event_info(int32_t event_id)
{
	if (event_id < 0 || event_id >= _countof(event_groups))
		return nullptr;
	return &event_groups[event_id];
}
