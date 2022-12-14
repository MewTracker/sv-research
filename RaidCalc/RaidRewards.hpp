#pragma once

#include <cstdint>

struct RaidFixedRewardItemInfo
{
    int32_t category;
    int32_t subject_type;
    int32_t item_id;
    int8_t num;

    int32_t item_id_final;
};

struct RaidFixedRewards
{
    uint64_t table_name;
    RaidFixedRewardItemInfo items[15];
};

struct RaidLotteryRewardItemInfo
{
    int32_t category;
    int32_t item_id;
    int8_t num;
    int32_t rate;
    bool rare_item_flag;

    int32_t item_id_final;
};

struct RaidLotteryRewards
{
    uint64_t table_name;
    int32_t rate_total;
    RaidLotteryRewardItemInfo items[30];
};

#include "RaidFixedRewards.inc"
#include "RaidLotteryRewards.inc"
