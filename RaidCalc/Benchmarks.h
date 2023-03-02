#pragma once

#include "SeedFinder.h"

//#define ENABLE_BENCHMARKS

#if defined(ENABLE_BENCHMARKS)
void do_benchmarks(SeedFinder& finder);
#else
#define do_benchmarks(...)
#endif

class Benchmarks
{
public:
	static void run(SeedFinder& finder);

private:
	static __declspec(noinline) void do_pokemon_bench(SeedFinder& finder);
	static __declspec(noinline) void do_rewards_bench(SeedFinder& finder);
	static __declspec(noinline) void do_tests(SeedFinder& finder);
	static __declspec(noinline) void generate_test_data(SeedFinder& finder, int progress, int raid_boost, bool is6 = false);
	static __declspec(noinline) void dump_personal_table();
	static __declspec(noinline) void dump_personal_info(std::string name, PersonalInfo9SV &info);
};
