#pragma once

#include <cstdlib>
#include <cstdint>

enum GenderRatio : uint8_t
{
    GenderRatioGenderless = 255,
    GenderRatioFemale = 254,
    GenderRatioMale = 0,
};

class PersonalInfo9SV
{
public:
	static const uint32_t InfoSize = 0x44;
	static const uint32_t CountTM = 172;

	PersonalInfo9SV(uint8_t* data)
	{
        hp = data[0x00];
        atk = data[0x01];
        def = data[0x02];
        spe = data[0x03];
        spa = data[0x04];
        spd = data[0x05];
        type1 = data[0x06];
        type2 = data[0x07];
        catch_rate = data[0x08];
        evo_stage = data[0x09];
        ev_yield = *(uint16_t*)&data[0x0A];
        ev_hp = (ev_yield >> 0) & 0x3;
        ev_atk = (ev_yield >> 2) & 0x3;
        ev_def = (ev_yield >> 4) & 0x3;
        ev_spe = (ev_yield >> 6) & 0x3;
        ev_spa = (ev_yield >> 8) & 0x3;
        ev_spd = (ev_yield >> 10) & 0x3;
        gender = data[0x0C];
        hatch_cycles = data[0x0D];
        base_friendship = data[0x0E];
        exp_growth = data[0x0F];
        egg_group1 = data[0x10];
        egg_group2 = data[0x11];
        ability[0] = *(uint16_t*)&data[0x12];
        ability[1] = *(uint16_t*)&data[0x14];
        ability[2] = *(uint16_t*)&data[0x16];
        form_stats_index = *(uint16_t*)&data[0x18];
        form_count = data[0x1A];
        color = data[0x1B];
        is_present_in_game = data[0x1C] != 0;
        dex_group = data[0x1D];
        dex_index = *(uint16_t*)&data[0x1E];
        height = *(uint16_t*)&data[0x20];
        weight = *(uint16_t*)&data[0x22];
        hatch_species = *(uint16_t*)&data[0x24];
        local_form_index = *(uint16_t*)&data[0x26];
        regional_flags = *(uint16_t*)&data[0x28];
        is_regional_form = (regional_flags & 1) == 1;
        regional_form_index = *(uint16_t*)&data[0x2A];
        for (size_t i = 0; i < _countof(tmhm); ++i)
            tmhm[i] = (data[0x2C + (i >> 3)] >> (i & 7)) & 1;
	}

    int form_index(uint16_t species, uint8_t form)
    {
        if (!has_form(form))
            return species;
        return form_stats_index + form - 1;
    }

    bool has_form(uint8_t form)
    {
        if (form == 0)
            return false;
        if (form_stats_index <= 0)
            return false;
        if (form >= form_count)
            return false;
        return true;
    }

    int32_t hp;
    int32_t atk;
    int32_t def;
    int32_t spe;
    int32_t spa;
    int32_t spd;
    uint8_t type1;
    uint8_t type2;
    int32_t catch_rate;
    int32_t evo_stage;
    int32_t ev_yield;
    int32_t ev_hp;
    int32_t ev_atk;
    int32_t ev_def;
    int32_t ev_spe;
    int32_t ev_spa;
    int32_t ev_spd;
    int32_t gender;
    int32_t hatch_cycles;
    int32_t base_friendship;
    uint8_t exp_growth;
    int32_t egg_group1;
    int32_t egg_group2;
    uint32_t ability[3];
    int32_t form_stats_index;
    uint8_t form_count;
    int32_t color;
    bool is_present_in_game;
    uint8_t dex_group;
    uint16_t dex_index;
    int32_t height;
    int32_t weight;
    uint16_t hatch_species;
    uint16_t local_form_index;
    uint16_t regional_flags;
    bool is_regional_form;
    uint16_t regional_form_index;
    bool tmhm[CountTM];
};
