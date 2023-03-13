#!/usr/bin/env python3
import os
import json
from pathlib import Path
from unidecode import unidecode

all_items = set()


def get_fixed_reward_items(record, max_column_count):
    items = []
    for i in range(max_column_count):
        item = record['RewardItem%02d' % i]
        item_id_final = item['Category'] * 10000 if item['ItemID'] == 0 else item['ItemID']
        all_items.add(item_id_final)
        items.append('{ %d, %d, %d }' % (item['Num'], item_id_final, item['SubjectType']))
    return items


def get_lottery_reward_items(record, max_column_count):
    items = []
    for i in range(max_column_count):
        item = record['RewardItem%02d' % i]
        flag = 'true' if item['RareItemFlag'] else 'false'
        if item['Category'] == 0:
            item_id_final = item['ItemID']
        elif item['Category'] == 1:
            item_id_final = 10000 if item['ItemID'] == 0 else item['ItemID']
        else:
            item_id_final = 20000 if item['ItemID'] == 0 else item['ItemID']
        all_items.add(item_id_final)
        items.append('{ %d, %d, %d, %s }' % (item['Num'], item_id_final, item['Rate'], flag))
    return items


def get_record(record, rate_total, item_fetcher, max_column_count):
    return '    { 0x%016X,%s { %s } },\n' % (record['TableName'], rate_total, ', '.join(item_fetcher(record, max_column_count)))


def get_used_coulmn_count(record, column_count):
    used_coulmn_count = 0
    for j in range(column_count):
        item = record['RewardItem%02d' % j]
        if item['Num'] != 0 or item['Category'] != 0 or item['ItemID'] != 0:
            used_coulmn_count = j + 1
    return used_coulmn_count


def get_max_used_column_count(rows, column_count):
    max_column_count = 0
    for record in rows:
        used_column_count = get_used_coulmn_count(record, column_count)
        if used_column_count > max_column_count:
            max_column_count = used_column_count
    return max_column_count


def write_cpp(type_name, variable_name, is_lottery, rows, path):
    item_fetcher = get_lottery_reward_items if is_lottery else get_fixed_reward_items
    column_count = 30 if is_lottery else 15
    max_column_count = get_max_used_column_count(rows, column_count)
    cpp = 'static const %s %s[] =\n{\n' % (type_name, variable_name)
    for record in rows:
        if get_used_coulmn_count(record, column_count) == 0:
            continue
        rate_total = 0
        if is_lottery:
            for i in range(column_count):
                rate_total += record['RewardItem%02d' % i]['Rate']
        cpp += get_record(record, ' %d,' % rate_total if rate_total > 0 else '', item_fetcher, max_column_count)
    cpp += '};\n'
    with open(path, 'w') as f:
        f.write(cpp)
    return max_column_count


def generate_possible_rewards(base_dir):
    with open(base_dir / 'resources' / 'text_Items_en.txt', encoding='utf-8') as f:
        item_names = f.readlines()
    items = list(all_items)
    items.sort()
    items = items[1:]
    header = 'static const RewardInfo reward_info[] =\n{\n'
    for item_id in items:
        if item_id == 10000:
            item_name = 'Material'
        elif item_id == 20000:
            item_name = 'Tera Shard'
        else:
            item_name = unidecode(item_names[item_id].strip())
        header += '    { %d, "%s" },\n' % (item_id, item_name)
    header += '};\n'
    return header


def parse_rewards():
    base_dir = Path(os.path.dirname(os.path.realpath(__file__)))
    events_dir = base_dir / 'events'
    events = [Path(f.path) for f in os.scandir(events_dir) if f.is_dir()]
    events.sort()
    with open(base_dir / 'resources' / 'fixed_reward_item_array.json', 'r') as f:
        fixed = json.load(f)
    max_fixed_columns_base = get_max_used_column_count(fixed, 15)
    for event_path in events:
        with open(event_path / 'fixed_reward_item_array.json', 'r') as f:
            fixed.extend(json.load(f)['Table'])
    with open(base_dir / 'resources' / 'lottery_reward_item_array.json', 'r') as f:
        lottery = json.load(f)
    max_lottery_columns_base = get_max_used_column_count(lottery, 30)
    for event_path in events:
        with open(event_path / 'lottery_reward_item_array.json', 'r') as f:
            lottery.extend(json.load(f)['Table'])

    raidcalc_dir = base_dir / '..' / 'RaidCalc'
    max_fixed_columns_events = write_cpp('RaidFixedRewards', 'fixed_rewards', False, fixed, raidcalc_dir / 'RaidFixedRewards.inc.h')
    max_lottery_columns_events = write_cpp('RaidLotteryRewards', 'lottery_rewards', True, lottery, raidcalc_dir / 'RaidLotteryRewards.inc.h')
    with open(raidcalc_dir / 'RewardInfo.inc.h', 'w') as f:
        f.write(generate_possible_rewards(base_dir))
    print('Max fixed items columns (base): %d' % max_fixed_columns_base)
    print('Max lottery items columns (base): %d' % max_lottery_columns_base)
    print('Max fixed items columns (events): %d' % max_fixed_columns_events)
    print('Max lottery items columns (events): %d' % max_lottery_columns_events)
