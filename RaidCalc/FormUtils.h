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
		case Shellos:
		case Gastrodon:
		case Toxtricity:
		case Maushold:
		case Sinistcha:
			return { 0, 1 };
		case Indeedee:
		case Oinkologne:
		case Basculegion:
			if (!include_gender_forms)
				return {};
			return { 0, 1 };
		case Lycanroc:
		case Tatsugiri:
			return { 0, 1, 2 };
		case Basculin:
			return { 2 };
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
		Tauros = 128,
		Dunsparce = 206,
		Shellos = 422,
		Gastrodon = 423,
		Basculin = 550,
		Lycanroc = 745,
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
