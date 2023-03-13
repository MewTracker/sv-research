#include "EncounterDatabaseDialog.h"
#include "SeedFinder.h"
#include <set>

EncounterDatabaseDialog::EncounterDatabaseDialog(QWidget *parent)
	: QDialog(parent)
{
    ui.setupUi(this);
    std::set<uint32_t> encounterables;
    std::vector<std::pair<std::string, uint32_t>> filters;
    auto visitor = [&](const EncounterTera9 &enc) { encounterables.insert(enc.species); };
    SeedFinder::visit_encounters(-1, visitor);
    for (int32_t i = 0; i < _countof(event_names); ++i)
        SeedFinder::visit_encounters(i, visitor);
    for (auto species : encounterables)
        filters.push_back({ pokemon_names[species], species });
    std::sort(filters.begin(), filters.end(), [](const auto &a, const auto &b) { return a.first < b.first; });
    for (auto &pair : filters)
        ui.comboBoxSpecies->addItem(pair.first.c_str(), pair.second);
    ui.tableEncounters->setModel(&encounter_model);
}

EncounterDatabaseDialog::~EncounterDatabaseDialog()
{

}

void EncounterDatabaseDialog::on_comboBoxSpecies_currentIndexChanged(int index)
{
    int32_t species = ui.comboBoxSpecies->currentData().toInt();
    int32_t event_id = -1;
    auto visitor = [&](const EncounterTera9 &enc)
    {
        if (enc.species != species)
            return;
        for (int32_t ver = GameScarlet; ver <= GameViolet; ++ver)
        {
            switch (enc.type)
            {
            case EncounterType::Gem:
                if (enc.rand_rate_min[ver] <= 0)
                    continue;
                break;
            case EncounterType::Dist:
            {
                bool is_available = false;
                for (int32_t stage = 0; stage < 4; ++stage)
                {
                    auto &rand_rate_data = enc.rand_rate_event[stage][ver];
                    if (rand_rate_data.total)
                    {
                        is_available = true;
                        break;
                    }
                }
                if (!is_available)
                    continue;
                break;
            }
            }
            EncounterEntry entry;
            entry.game = (Game)ver;
            entry.event_id = event_id;
            entry.event_group = event_id < 0 ? -1 : enc.group;
            entry.stars = enc.stars;
            entry.tera_type = enc.tera_type;
            encounter_model.add_entry(entry);
        }
    };
    encounter_model.clear();
    SeedFinder::visit_encounters(-1, visitor);
    for (event_id = 0; event_id < _countof(event_names); ++event_id)
        SeedFinder::visit_encounters(event_id, visitor);
}

void EncounterDatabaseDialog::on_tableEncounters_doubleClicked(const QModelIndex &index)
{
    emit parameterChangeRequested(encounter_model.get_encounter(index.row()), ui.comboBoxSpecies->currentData().toInt());
}
