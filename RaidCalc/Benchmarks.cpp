#include <QtGlobal>
#include <cstdio>
#include <string>
#include "Benchmarks.h"
#include "Stopwatch.h"
#include "PokemonNames.h"
#include "Utils.h"

#if defined(ENABLE_BENCHMARKS)

void do_benchmarks(SeedFinder& finder)
{
    Benchmarks::run(finder);
}

#endif

__declspec(noinline) void Benchmarks::generate_test_data(SeedFinder& finder, int progress, int raid_boost, bool is6)
{
    finder.game = GameScarlet;
    finder.story_progress = progress;
    finder.raid_boost = raid_boost;
    for (uint32_t seed = 0; seed < 100000; ++seed)
    {
        finder.stars = is6 ? 6 : SeedFinder::get_star_count(seed, progress);
        SeedFinder::SeedInfo info = finder.get_seed_info(seed);
        auto rewards = finder.get_all_rewards(seed);
        std::string pokemon;
        pokemon += format_uint32(seed).toStdString() + "|";
        pokemon += format_uint32(info.ec).toStdString() + "|";
        pokemon += format_uint32(info.pid).toStdString() + "|";
        pokemon += info.shiny ? "Shiny" : "NotShiny";
        pokemon += "|";
        pokemon += type_names[info.tera_type];
        pokemon += "|";
        pokemon += QString::number(info.stars).toStdString();
        pokemon += "|";
        pokemon += pokemon_names[info.species];
        pokemon += "|";
        pokemon += gender_names[info.gender];
        pokemon += "|";
        pokemon += nature_names[info.nature];
        pokemon += "|";
        pokemon += ability_names[info.ability];
        pokemon += "|";
        for (size_t i = 0; i < _countof(info.moves); ++i)
        {
            if (info.moves[i] == 0)
                pokemon += "(None)";
            else
                pokemon += move_names[info.moves[i]];
            pokemon += "|";
        }
        QString iv = QString::asprintf("%02d/%02d/%02d/%02d/%02d/%02d", info.iv[0], info.iv[1], info.iv[2], info.iv[3], info.iv[4], info.iv[5]);
        pokemon += iv.toStdString();
        for (auto reward : rewards)
            pokemon += QString("|(%1,%2)").arg(QString::number(reward.item_id), QString::number(reward.count)).toStdString();
        printf("%s\n", pokemon.c_str());
    }
}

__declspec(noinline) void Benchmarks::do_tests(SeedFinder& finder)
{
    for (int progress = 0; progress <= 4; ++progress)
    {
        for (int raid_boost = 0; raid_boost <= 1; ++raid_boost)
        {
            printf("Progress=%d|RaidBoost=%d\n", progress, raid_boost);
            generate_test_data(finder, progress, raid_boost);
        }
    }
    printf("Progress=4|RaidBoost=0|is6=true\n");
    generate_test_data(finder, 4, 0, true);
}

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
        finder.worker_thread<true, true, true, false, false>(data);
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
        finder.worker_thread<true, false, false, false, true>(data);
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
    // TODO: Better place for this?
    do_tests(finder);
    return;

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
