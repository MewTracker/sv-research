#pragma once

#include "SeedFinder.hpp"

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
};
