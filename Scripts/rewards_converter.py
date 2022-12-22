#!/usr/bin/env python3
import os
import json

max_fixed_count = 0
max_lottery_count = 0


def get_fixed_reward_items(record):
    items = []
    for i in range(max_fixed_count):
        item = record['RewardItem%02d' % i]
        item_id_final = item['Category'] * 10000 if item['ItemID'] == 0 else item['ItemID']
        items.append('{ %d, %d, %d }' % (item['Num'], item_id_final, item['SubjectType']))
    return items


def get_lottery_reward_items(record):
    items = []
    for i in range(max_lottery_count):
        item = record['RewardItem%02d' % i]
        flag = 'true' if item['RareItemFlag'] else 'false'
        if item['Category'] == 0:
            item_id_final = item['ItemID']
        elif item['Category'] == 1:
            item_id_final = 10000 if item['ItemID'] == 0 else item['ItemID']
        else:
            item_id_final = 20000 if item['ItemID'] == 0 else item['ItemID']
        items.append('{ %d, %d, %d, %s }' % (item['Num'], item_id_final, item['Rate'], flag))
    return items


def get_record(record, rate_total, item_fetcher):
    return '    { 0x%016X,%s { %s } },\n' % (record['TableName'], rate_total, ', '.join(item_fetcher(record)))


if __name__ == "__main__":
    base_dir = os.path.dirname(os.path.realpath(__file__))
    with open(os.path.join(base_dir, 'raid_fixed_reward_item_array.json'), 'r') as f:
        fixed = json.load(f)
    fixed_cpp = 'static const RaidFixedRewards fixed_rewards[] =\n{\n'
    for i, record in enumerate(fixed):
        for j in range(15):
            item = record['RewardItem%02d' % j]
            if (item['Category'] != 0 or item['ItemID'] != 0) and j + 1 > max_fixed_count:
                max_fixed_count = j + 1
    for record in fixed:
        fixed_cpp += get_record(record, '', get_fixed_reward_items)
    fixed_cpp += '};\n'
    with open(os.path.join(base_dir, '..', 'RaidCalc', 'RaidFixedRewards.inc.h'), 'w') as f:
        f.write(fixed_cpp)

    with open(os.path.join(base_dir, 'raid_lottery_reward_item_array.json'), 'r') as f:
        lottery = json.load(f)
    lottery_cpp = 'static const RaidLotteryRewards lottery_rewards[] =\n{\n'
    for i, record in enumerate(lottery):
        for j in range(30):
            item = record['RewardItem%02d' % j]
            if (item['Num'] != 0 or item['Category'] != 0 or item['ItemID'] != 0 or item['Rate'] != 0) and j + 1 > max_lottery_count:
                max_lottery_count = j + 1
    for record in lottery:
        rate_total = 0
        for i in range(30):
            rate_total += record['RewardItem%02d' % i]['Rate']
        lottery_cpp += get_record(record, ' %d,' % rate_total, get_lottery_reward_items)
    lottery_cpp += '};\n'
    with open(os.path.join(base_dir, '..', 'RaidCalc', 'RaidLotteryRewards.inc.h'), 'w') as f:
        f.write(lottery_cpp)

    print('Max fixed items columns: %d' % max_fixed_count)
    print('Max fixed items columns: %d' % max_lottery_count)
