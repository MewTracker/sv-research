#pragma once

#include <cstdint>
#include <map>
#include <string>

struct RaidFixedRewardItemInfo
{
    int8_t num;
    int32_t item_id;
    int32_t subject_type;
};

struct RaidFixedRewards
{
    uint64_t table_name;
    RaidFixedRewardItemInfo items[12];
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
    RaidLotteryRewardItemInfo items[25];
};

struct RewardInfo
{
    int32_t item_id;
    const char* name;
};

#include "RaidFixedRewards.inc.h"
#include "RaidLotteryRewards.inc.h"
#include "RewardInfo.inc.h"

class ItemDatabase
{
public:
    ItemDatabase(const ItemDatabase &) = delete;
    ItemDatabase &operator=(const ItemDatabase &) = delete;

    static ItemDatabase &instance()
    {
        static ItemDatabase db;
        return db;
    }

    std::string get_item_name(int32_t item_id)
    {
        auto it = items.find(item_id);
        if (it == items.end())
            return "Invalid item (" + std::to_string(item_id) + ")";
        return it->second;
    }

private:
    ItemDatabase()
    {
        for (auto item : reward_info)
            items[item.item_id] = item.name;
    }

    std::map<int32_t, std::string> items;
};
