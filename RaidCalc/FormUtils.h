#pragma once

#include <vector>
#include <string>
#include "PokemonNames.h"

class FormUtils
{
public:

	static std::string get_pokemon_name(uint32_t species, uint8_t form)
	{
		const char* form_name = get_form_name(species, form);
		if (*form_name == '\0')
			return pokemon_names[species];
		std::string name(pokemon_names[species]);
		name += " (";
		name += form_name;
		name += ")";
		return name;
	}

	static std::string get_pokemon_name(uint32_t species, uint8_t form, uint32_t ec)
	{
		std::string name(get_pokemon_name(species, form));
		if (!has_rare_form(species))
			return name;
		return name + ((ec % 100) == 0 ? " (Rare)" : " (Common)");
	}

	static const char* get_form_name(uint32_t species, uint8_t form)
	{
		switch (species)
		{
		case Tauros:
		{
			if (form < 1 || form > 3)
				return "";
			static const char* tauros_forms[] =
			{
				"Combat Breed",
				"Blaze Breed",
				"Aqua Breed",
			};
			return tauros_forms[form - 1];
		}
		case Shellos:
		case Gastrodon:
		{
			if (form > 1)
				return "";
			static const char* shellos_forms[] =
			{
				"West",
				"East",
			};
			return shellos_forms[form];
		}
		case Basculin:
		{
			if (form != 2) // Restrict to obtainable form
				return "";
			static const char* basculin_forms[] =
			{
				"Red",
				"Blue",
				"White",
			};
			return basculin_forms[form];
		}
		case Lycanroc:
		{
			if (form > 2)
				return "";
			static const char* lycanroc_forms[] =
			{
				"Midday",
				"Midnight",
				"Dusk",
			};
			return lycanroc_forms[form];
		}
		case Toxtricity:
		{
			if (form > 1)
				return "";
			static const char* toxtricity_forms[] =
			{
				"Amped",
				"Low Key",
			};
			return toxtricity_forms[form];
		}
		case Maushold:
		{
			if (form > 1)
				return "";
			static const char* maushold_forms[] =
			{
				"Family of Three",
				"Family of Four",
			};
			return maushold_forms[form];
		}
		case Tatsugiri:
		{
			if (form > 2)
				return "";
			static const char* tatsugiri_forms[] =
			{
				"Curly",
				"Droopy",
				"Stretchy",
			};
			return tatsugiri_forms[form];
		}
		case Sinistcha:
		{
			if (form > 1)
				return "";
			static const char* sinistcha_forms[] =
			{
				"Unremarkable Form",
				"Masterpiece Form",
			};
			return sinistcha_forms[form];
		}
		case Minior:
		{
			if (form != 7) // Restrict to obtainable form
				return "";
			static const char *minior_forms[] =
			{
				"Red Meteor",
				"Orange Meteor",
				"Yellow Meteor",
				"Green Meteor",
				"Blue Meteor",
				"Indigo Meteor",
				"Violet Meteor",
				"Red Core",
				"Orange Core",
				"Yellow Core",
				"Green Core",
				"Blue Core",
				"Indigo Core",
				"Violet Core",
			};
			return minior_forms[form];
		}
		case Sandshrew:
		case Sandslash:
		case Vulpix:
		case Ninetales:
		case Diglett:
		case Dugtrio:
		case Geodude:
		case Golem:
		case Grimer:
		case Muk:
		case Exeggutor:
			if (form != 1)
				return "";
			return "Alolan";
		case Slowpoke:
		case Slowking:
			if (form != 1)
				return "";
			return "Galarian";
		case Slowbro:
			if (form != 2)
				return "";
			return "Galarian";
		case Typhlosion:
		case Samurott:
		case Decidueye:
			if (form != 1)
				return "";
			return "Hisuian";
		case Wooper:
			if (form != 1)
				return "";
			return "Paldean";
		default:
			return "";
		}
	}

	static std::vector<uint8_t> get_forms(uint32_t species, bool include_gender_forms = false)
	{
		switch (species)
		{
		case Tauros:
			return { 1, 2, 3 };
		case Sandshrew:
		case Vulpix:
		case Ninetales:
		case Diglett:
		case Dugtrio:
		case Geodude:
		case Golem:
		case Slowpoke:
		case Grimer:
		case Muk:
		case Exeggutor:
		case Typhlosion:
		case Wooper:
		case Slowking:
		case Shellos:
		case Gastrodon:
		case Samurott:
		case Decidueye:
		case Toxtricity:
		case Maushold:
		case Sinistcha:
			return { 0, 1 };
		case Meowstic:
		case Indeedee:
		case Oinkologne:
		case Basculegion:
			if (!include_gender_forms)
				return {};
			return { 0, 1 };
		case Slowbro:
			return { 0, 2 };
		case Lycanroc:
		case Tatsugiri:
			return { 0, 1, 2 };
		case Basculin:
			return { 2 };
		case Sandslash:
			return { 1 };
		case Minior:
			return { 7 };
		default:
			return {};
		}
	}

	static bool has_rare_form(uint32_t species)
	{
		switch (species)
		{
		case Dunsparce:
		case Tandemaus:
			return true;
		default:
			return false;
		}
	}

private:
	enum FormSpecies
	{
		Sandshrew = 27,
		Sandslash = 28,
		Vulpix = 37,
		Ninetales = 38,
		Diglett = 50,
		Dugtrio = 51,
		Geodude = 74,
		Golem = 76,
		Slowpoke = 79,
		Slowbro = 80,
		Grimer = 88,
		Muk = 89,
		Exeggutor = 103,
		Tauros = 128,
		Typhlosion = 157,
		Wooper = 194,
		Slowking = 199,
		Dunsparce = 206,
		Shellos = 422,
		Gastrodon = 423,
		Samurott = 503,
		Basculin = 550,
		Meowstic = 678,
		Decidueye = 724,
		Lycanroc = 745,
		Minior = 774,
		Toxtricity = 849,
		Indeedee = 876,
		Basculegion = 902,
		Oinkologne = 916,
		Tandemaus = 945,
		Maushold = 946,
		Tatsugiri = 952,
		Sinistcha = 1025,
	};
};
