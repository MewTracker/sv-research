#include <QtGlobal>
#include "Benchmarks.h"
#include "Stopwatch.h"

#if defined(ENABLE_BENCHMARKS)

void do_benchmarks(SeedFinder& finder)
{
    Benchmarks::run(finder);
}

#endif

__declspec(noinline) void Benchmarks::do_pokemon_bench(SeedFinder& finder)
{
    Stopwatch stopwatch;
    uint64_t avg_ms = 0;
    SeedFinder::ThreadData data;
    data.finder = &finder;
    data.range_min = 0;
    data.range_max = 10000000;
    data.results.reserve(data.range_max);
    for (int i = 0; i < 10; ++i)
    {
        stopwatch.start();
        finder.pokemon_thread(data);
        stopwatch.stop();
        data.results.clear();
        if (i > 3)
            avg_ms += stopwatch.milliseconds();
        qInfo("%d) Pokemon bench took %llums", i + 1, stopwatch.milliseconds());
    }
    avg_ms /= 6;
    qInfo("Average time is %llums", avg_ms);
}

__declspec(noinline) void Benchmarks::do_rewards_bench(SeedFinder& finder)
{
    Stopwatch stopwatch;
    uint64_t avg_ms = 0;
    SeedFinder::ThreadData data;
    data.finder = &finder;
    data.range_min = 0;
    data.range_max = 10000000;
    data.results.reserve(data.range_max);
    for (int i = 0; i < 10; ++i)
    {
        stopwatch.start();
        finder.rewards_thread(data);
        stopwatch.stop();
        data.results.clear();
        if (i > 3)
            avg_ms += stopwatch.milliseconds();
        qInfo("%d) Rewards bench took %llums", i + 1, stopwatch.milliseconds());
    }
    avg_ms /= 6;
    qInfo("Average time is %llums", avg_ms);
}

void Benchmarks::run(SeedFinder& finder)
{
    finder.game = GameScarlet;
    finder.stars = 6;
    finder.thread_count = 1;
    finder.min_seed = 0;
    finder.max_seed = 0;
    finder.species = 0;
    finder.shiny = 0;
    finder.tera_type = 0;
    finder.ability = 0;
    finder.nature = 0;
    finder.gender = 0;
    for (size_t i = 0; i < _countof(finder.min_iv); ++i)
        finder.min_iv[i] = 0;
    for (size_t i = 0; i < _countof(finder.max_iv); ++i)
        finder.max_iv[i] = 0;
    finder.item_filters_active = false;
    finder.drop_threshold = 0;
    do_pokemon_bench(finder);

    finder.stars = 5;
    do_pokemon_bench(finder);
    finder.stars = 6;

    finder.item_filters_active = true;
    finder.drop_threshold = 8;
    for (int i = 1904; i <= 1908; ++i)
        finder.set_drop_filter(1904, true);
    do_rewards_bench(finder);
}
