#pragma once

#include <vector>
#include "PersonalInfo9SV.h"
#include "Utils.h"

class PersonalTable9SV
{
public:
	PersonalTable9SV(const PersonalTable9SV&) = delete;
	PersonalTable9SV& operator=(const PersonalTable9SV&) = delete;

	static PersonalTable9SV& instance()
	{
		static PersonalTable9SV inst;
		return inst;
	}

	int get_form_index(uint16_t species, uint8_t form)
	{
		return records[species].form_index(species, form);
	}

	PersonalInfo9SV& get_form_entry(uint16_t species, uint8_t form)
	{
		return records[get_form_index(species, form)];
	}

	PersonalInfo9SV& operator[](size_t index)
	{
		return records[index];
	}

private:
	PersonalTable9SV()
	{
		auto raw_data = get_resource_file(":/resources/personal_sv");
		for (size_t i = 0; i < raw_data.size() / PersonalInfo9SV::InfoSize; ++i)
			records.push_back(PersonalInfo9SV(raw_data.data() + i * PersonalInfo9SV::InfoSize));
	}

	std::vector<PersonalInfo9SV> records;
};
