#pragma once

#include <QAbstractTableModel>
#include "SeedFinder.h"
#include "PokemonNames.h"
#include "FormUtils.h"
#include "Utils.h"

class SeedTableModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	void clear()
	{
		if (seed_data.empty())
			return;
		beginRemoveRows(QModelIndex(), 0, (int)seed_data.size() - 1);
		seed_data.clear();
		endRemoveRows();
	}

	void populateModel(const SeedFinder &finder)
	{
		clear();
		if (finder.seeds.empty())
			return;
		beginInsertRows(QModelIndex(), 0, (int)finder.seeds.size() - 1);
		for (uint32_t seed : finder.seeds)
			seed_data.push_back(finder.get_seed_info(seed));
		endInsertRows();
	}

	uint32_t get_seed(int row)
	{
		return seed_data[row].seed;
	}

	int rowCount(const QModelIndex& parent = QModelIndex()) const override
	{
		return (int)seed_data.size();
	}

	int columnCount(const QModelIndex& parent = QModelIndex()) const override
	{
		return 19;
	}

	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override
	{
		if (role != Qt::DisplayRole)
			return QVariant();
		if (orientation == Qt::Vertical)
			return section + 1;
		if (section >= 19)
			return QVariant();
		static const char* headers[] =
		{
			"Seed",
			"Species",
			"Shiny",
			"EC",
			"PID",
			"Tera Type",
			"HP",
			"Atk",
			"Def",
			"SpA",
			"SpD",
			"Spe",
			"Ability",
			"Nature",
			"Gender",
			"Height",
			"Weight",
			"Scale",
			"Drops",
		};
		return headers[section];
	}

	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
	{
		if (role != Qt::DisplayRole || !index.isValid())
			return QVariant();
		auto &info = seed_data[index.row()];
		switch (index.column())
		{
		case 0:
			return format_uint32(info.seed);
		case 1:
			return FormUtils::get_pokemon_name(info.species, info.form, info.ec).c_str();
		case 2:
			return info.shiny ? "Yes" : "No";
		case 3:
			return format_uint32(info.ec);
		case 4:
			return format_uint32(info.pid);
		case 5:
			return type_names[info.tera_type];
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
			return info.iv[index.column() - 6];
		case 12:
			return ability_names[info.ability];
		case 13:
			return nature_names[info.nature];
		case 14:
			return gender_names[info.gender];
		case 15:
			return info.height;
		case 16:
			return info.weight;
		case 17:
			return info.scale;
		case 18:
			return info.drops;
		default:
			return QVariant();
		}
	}

protected:
	std::vector<SeedFinder::SeedInfo> seed_data;
};
