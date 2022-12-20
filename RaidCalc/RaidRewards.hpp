#pragma once

#include <cstdint>

struct RaidFixedRewardItemInfo
{
    int8_t num;
    int32_t item_id;
    int32_t subject_type;
};

struct RaidFixedRewards
{
    uint64_t table_name;
    RaidFixedRewardItemInfo items[7];
};

struct RaidLotteryRewardItemInfo
{
    int8_t num;
    int32_t item_id;
    int32_t rate;
    bool rare_item_flag;
};

struct RaidLotteryRewards
{
    uint64_t table_name;
    int32_t rate_total;
    RaidLotteryRewardItemInfo items[19];
};

#include "RaidFixedRewards.inc.hpp"
#include "RaidLotteryRewards.inc.hpp"

struct RewardInfo
{
    int32_t item_id;
    const char* name;
};

static const RewardInfo reward_info[] =
{
    { 4, "Poke Ball" },
    { 50, "Rare Candy" },
    { 51, "PP Up" },
    { 86, "Tiny Mushroom" },
    { 87, "Big Mushroom" },
    { 88, "Pearl" },
    { 89, "Big Pearl" },
    { 90, "Stardust" },
    { 91, "Star Piece" },
    { 92, "Nugget" },
    { 149, "Cheri Berry" },
    { 150, "Chesto Berry" },
    { 151, "Pecha Berry" },
    { 152, "Rawst Berry" },
    { 153, "Aspear Berry" },
    { 155, "Oran Berry" },
    { 156, "Persim Berry" },
    { 157, "Lum Berry" },
    { 158, "Sitrus Berry" },
    { 159, "Figy Berry" },
    { 160, "Wiki Berry" },
    { 161, "Mago Berry" },
    { 162, "Aguav Berry" },
    { 163, "Iapapa Berry" },
    { 169, "Pomeg Berry" },
    { 170, "Kelpsy Berry" },
    { 171, "Qualot Berry" },
    { 172, "Hondew Berry" },
    { 173, "Grepa Berry" },
    { 174, "Tamato Berry" },
    { 565, "Health Feather" },
    { 566, "Muscle Feather" },
    { 567, "Resist Feather" },
    { 568, "Genius Feather" },
    { 569, "Clever Feather" },
    { 570, "Swift Feather" },
    { 580, "Balm Mushroom" },
    { 582, "Pearl String" },
    { 583, "Comet Shard" },
    { 645, "Ability Capsule" },
    { 795, "Bottle Cap" },
    { 1124, "Exp. Candy XS" },
    { 1125, "Exp. Candy S" },
    { 1126, "Exp. Candy M" },
    { 1127, "Exp. Candy L" },
    { 1128, "Exp. Candy XL" },
    { 1231, "Lonely Mint" },
    { 1232, "Adamant Mint" },
    { 1233, "Naughty Mint" },
    { 1234, "Brave Mint" },
    { 1235, "Bold Mint" },
    { 1236, "Impish Mint" },
    { 1237, "Lax Mint" },
    { 1238, "Relaxed Mint" },
    { 1239, "Modest Mint" },
    { 1240, "Mild Mint" },
    { 1241, "Rash Mint" },
    { 1242, "Quiet Mint" },
    { 1243, "Calm Mint" },
    { 1244, "Gentle Mint" },
    { 1245, "Careful Mint" },
    { 1246, "Sassy Mint" },
    { 1247, "Timid Mint" },
    { 1248, "Hasty Mint" },
    { 1249, "Jolly Mint" },
    { 1250, "Naive Mint" },
    { 1606, "Ability Patch" },
    { 1904, "Sweet Herba Mystica" },
    { 1905, "Salty Herba Mystica" },
    { 1906, "Sour Herba Mystica" },
    { 1907, "Bitter Herba Mystica" },
    { 1908, "Spicy Herba Mystica" },
    { 10000, "Material" },
    { 20000, "Tera Shard" },
};
