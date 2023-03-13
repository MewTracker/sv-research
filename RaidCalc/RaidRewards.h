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
