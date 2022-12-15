# SV Research
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

This repository contains notes and tools developed during my research of Pokémon Scarlet/Violet games for Nintendo Switch.

## 1. Raids and Herba Mystica farming

Herba Mystica are considered one of most useful items for shiny hunting as they can be used to make Sparkling Power Level 3 sandwitches. They can be obtained as random drops from 5 and 6 star raids. Raids in SV are generated from 32-bit PRNG seeds (Xoroshiro128+, standard for Pokémon games). With only 2^32 possible values it's feasible to check them all for optimal drops. Those seeds can later be injected via [PKHeX](https://github.com/kwsch/PKHeX) or cheat codes. CFW is required for those operations but raids can be hosted, so other people without hacked consoles can benefit from them as well.

RaidCalc is a simple tool to find such optimal seeds. It's built on top of logic obtained from [RaidCrawler](https://github.com/LegoFigure11/RaidCrawler) and [PKHeX](https://github.com/kwsch/PKHeX). Original code was written in C#, wasn't optimized and had no multithreading. RaidCalc is basically a C++ port with extra optimizations and multithreading. Single-threaded performance has been improved by ~92%. Total improvement (with threads) brought execution time down from 2.5 hours to just over 3 minutes on my test machine.

All seeds are for 6 star raids (maximum story progression required):
- Herba Mystica (8, 9 and 10 drops): [Scarlet Seeds](RaidCalc/herba_seeds_scarlet.txt)/[Violet Seeds](RaidCalc/herba_seeds_violet.txt)
- Ability Patches (5 drops): [Scarlet Seeds](RaidCalc/ability_patch_seeds_scarlet.txt)/[Violet Seeds](RaidCalc/ability_patch_seeds_violet.txt)

### TODO:
- searching for other interesting drops
- searching for shiny raids

