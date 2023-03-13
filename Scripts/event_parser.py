#!/usr/bin/env python3
import os
import json
from struct import *
from pathlib import Path

STAGE_STARS = (
    (1, 2),
    (1, 2, 3),
    (1, 2, 3, 4),
    (3, 4, 5, 6, 7),
)


def parse_event(path):
    dist_count = 0
    might_count = 0
    event_dist = bytes()
    event_might = bytes()
    with open(path / 'raid_enemy_array.json', 'r') as f:
        enemies = [x['RaidEnemyInfo'] for x in json.load(f)['Table']]

    weight_total_s = [[0 for _ in range(4)] for _ in range(11)]
    weight_total_v = [[0 for _ in range(4)] for _ in range(11)]
    for enemy in enemies:
        if enemy['Rate'] == 0:
            continue
        id = enemy['DeliveryGroupID']
        difficulty = enemy['Difficulty']
        for stage in range(4):
            if difficulty not in STAGE_STARS[stage]:
                continue
            if enemy['RomVer'] != 2:
                weight_total_s[id][stage] += enemy['Rate']
            if enemy['RomVer'] != 1:
                weight_total_v[id][stage] += enemy['Rate']

    weight_min_s = [[0 for _ in range(4)] for _ in range(11)]
    weight_min_v = [[0 for _ in range(4)] for _ in range(11)]
    for enemy in enemies:
        if enemy['Rate'] == 0:
            continue
        boss = enemy['BossPokePara']
        packed = pack('<HBBBBBBHHHHBBBB',
             boss['DevId'],
             boss['FormId'],
             boss['Sex'],
             boss['Tokusei'],
             boss['TalentVnum'],
             boss['RareType'],
             enemy['CaptureLv'],
             boss['Waza1']['WazaId'],
             boss['Waza2']['WazaId'],
             boss['Waza3']['WazaId'],
             boss['Waza4']['WazaId'],
             1 if boss['GemType'] == 0 else boss['GemType'],
             enemy['DeliveryGroupID'],
             enemy['Difficulty'],
             enemy['Rate'])
        id = enemy['DeliveryGroupID']
        difficulty = enemy['Difficulty']
        for stage in range(4):
            if difficulty not in STAGE_STARS[stage]:
                packed += pack('<HHHH', 0, 0, 0, 0)
                continue
            packed += pack('<HHHH',
                weight_min_s[id][stage] if enemy['RomVer'] != 2 else 0,
                weight_min_v[id][stage] if enemy['RomVer'] != 1 else 0,
                weight_total_s[id][stage] if enemy['RomVer'] != 2 else 0,
                weight_total_v[id][stage] if enemy['RomVer'] != 1 else 0)
            if enemy['RomVer'] != 2:
                weight_min_s[id][stage] += enemy['Rate']
            if enemy['RomVer'] != 1:
                weight_min_v[id][stage] += enemy['Rate']
        packed += pack('<QQ', enemy['DropTableFix'], enemy['DropTableRandom'])
        if difficulty == 7:
            packed += pack('<BBBBBBBBBB',
                25 if boss['Seikaku'] == 0 else boss['Seikaku'] - 1,
                boss['TalentValue']['HP'],
                boss['TalentValue']['ATK'],
                boss['TalentValue']['DEF'],
                boss['TalentValue']['SPA'],
                boss['TalentValue']['SPD'],
                boss['TalentValue']['SPE'],
                0 if boss['TalentType'] == 0 else 1,
                boss['ScaleType'],
                boss['ScaleValue'])
            event_might += packed
            might_count += 1
        else:
            event_dist += packed
            dist_count += 1

    event_id = int(os.path.split(path)[1].split('_')[0]) - 1
    return pack('<LL', event_id, dist_count) + event_dist, pack('<LL', event_id, might_count) + event_might


def generate_event_names(events):
    names = []
    for event in events:
        names.append(' '.join(os.path.split(event)[1].split('_')[1:]))
    header = 'static const char *event_names[] =\n{\n'
    for name in names:
        header += '    "%s",\n' % name
    header += '};\n'
    return header


def parse_events():
    base_dir = Path(os.path.dirname(os.path.realpath(__file__)))
    events_dir = base_dir / 'events'
    events = [Path(f.path) for f in os.scandir(events_dir) if f.is_dir()]
    events.sort()
    with open(base_dir / '..' / 'RaidCalc' / 'EventNames.inc.h', 'w') as f:
        f.write(generate_event_names(events))
    events_dist = bytes()
    events_might = bytes()
    for path in events:
        event_data = parse_event(path)
        events_dist += event_data[0]
        events_might += event_data[1]
    resources_dir = base_dir / '..' / 'RaidCalc' / 'resources'
    with open(resources_dir / 'encounter_dist_paldea.pklex', 'wb') as f:
        f.write(events_dist)
    with open(resources_dir / 'encounter_might_paldea.pklex', 'wb') as f:
        f.write(events_might)
