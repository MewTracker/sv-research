#pragma once

#include <QAbstractTableModel>
#include "EncounterTera9.h"
#include "PokemonNames.h"

struct EncounterEntry
{
	Game game;
	int32_t event_id;
	int32_t event_group;
	int32_t stars;
	GemType tera_type;
};

class EncounterEntryModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	void clear()
	{
		if (encounters.empty())
			return;
		beginRemoveRows(QModelIndex(), 0, (int)encounters.size() - 1);
		encounters.clear();
		endRemoveRows();
	}

	void add_entry(const EncounterEntry &entry)
	{
		beginInsertRows(QModelIndex(), (int)encounters.size(), (int)encounters.size());
		encounters.push_back(entry);
		endInsertRows();
	}

	EncounterEntry get_encounter(size_t index) const
	{
		return encounters[index];
	}

	int rowCount(const QModelIndex &parent = QModelIndex()) const override
	{
		return (int)encounters.size();
	}

	int columnCount(const QModelIndex &parent = QModelIndex()) const override
	{
		return 5;
	}

	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override
	{
		if (role != Qt::DisplayRole)
			return QVariant();
		if (orientation == Qt::Vertical)
			return section + 1;
		if (section >= 5)
			return QVariant();
		static const char *headers[] =
		{
			"Game",
			"Event",
			"Event group",
			"Stars",
			"Tera Type",
		};
		return headers[section];
	}

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
	{
		if (role != Qt::DisplayRole || !index.isValid())
			return QVariant();
		auto &entry = encounters[index.row()];
		switch (index.column())
		{
		case 0:
			return entry.game == GameScarlet ? "Scarlet" : "Violet";
		case 1:
			return entry.event_id < 0 ? "N/A" : event_names[entry.event_id];
		case 2:
			return entry.event_id < 0 ? "N/A" : QString::number(entry.event_group);
		case 3:
			return entry.stars;
		case 4:
			return entry.tera_type != GemType::Random ? type_names[static_cast<int8_t>(entry.tera_type) - 2] : "Random";
		default:
			return QVariant();
		}
	}

protected:
	std::vector<EncounterEntry> encounters;
};
