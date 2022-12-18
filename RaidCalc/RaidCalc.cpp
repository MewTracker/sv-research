#include <QMessageBox>
#include <QStandardItemModel>
#include <vector>
#include <algorithm>
#include "RaidCalc.h"
#include "PokemonNames.hpp"

RaidCalc::RaidCalc(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    if (!finder.initialize())
    {
        QMessageBox::critical(this, "Error", "Failed to initialize seed finder.");
        QTimer::singleShot(0, qApp, &QCoreApplication::quit);
    }
    std::vector<std::pair<std::string, int>> species_data;
    for (int i = 1; i < _countof(pokemon_names); ++i)
        species_data.push_back({ pokemon_names[i], i });
    std::sort(species_data.begin(), species_data.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    species_lookup.push_back(0);
    ui.comboBoxSpecies->addItem("Any");
    for (auto& pair : species_data)
    {
        species_lookup.push_back(pair.second);
        ui.comboBoxSpecies->addItem(pair.first.c_str());
    }
    itemFilters = new ItemFilterDialog(this);
    seedModel = new QStandardItemModel(this);
    ui.tableSeeds->setModel(seedModel);
    finder_timer = new QTimer(this);
    connect(finder_timer, &QTimer::timeout, this, &RaidCalc::on_finder_timer_timeout);

    // TODO: Implement filters
    ui.labelTeraType->setVisible(false);
    ui.labelAbility->setVisible(false);
    ui.labelNature->setVisible(false);
    ui.labelGender->setVisible(false);
    ui.comboBoxTeraType->setVisible(false);
    ui.comboBoxAbility->setVisible(false);
    ui.comboBoxNature->setVisible(false);
    ui.comboBoxGender->setVisible(false);
}

RaidCalc::~RaidCalc()
{

}

bool RaidCalc::hex_to_uint32(const QString& hex_string, uint32_t& result)
{
    return hex_to_uint32(hex_string.toStdString().c_str(), result);
}

bool RaidCalc::hex_to_uint32(const char* hex_string, uint32_t& result)
{
    char* end_ptr = nullptr;
    errno = 0;
    result = strtoul(hex_string, &end_ptr, 16);
    if ((size_t)(end_ptr - hex_string) != strlen(hex_string) || errno == ERANGE)
        return false;
    return true;
}

QString RaidCalc::format_uint32(uint32_t value)
{
    return QString("%1").arg(value, 8, 16, QLatin1Char('0'));
}

QStandardItem* RaidCalc::readonly_item(QString text)
{
    QStandardItem* item = new QStandardItem(text);
    item->setEditable(false);
    return item;
}

void RaidCalc::toggle_ui(bool enabled)
{
    for (auto widget : ui.basicGroup->findChildren<QWidget*>())
        widget->setEnabled(enabled);
    for (auto widget : ui.pokemonGroup->findChildren<QWidget*>())
        widget->setEnabled(enabled);
    for (auto widget : ui.itemGroup->findChildren<QWidget*>())
        widget->setEnabled(enabled);
    ui.tableSeeds->setEnabled(enabled);
    if (!enabled)
    {
        itemFilters->hide();
        seedModel->clear();
    }
}

void RaidCalc::on_buttonFindSeeds_clicked()
{
    finder.game = (Game)ui.comboBoxGame->currentIndex();
    if (!hex_to_uint32(ui.editMinSeed->text(), finder.min_seed))
    {
        QMessageBox::critical(this, "Error", "Bad min seed value.");
        return;
    }
    if (!hex_to_uint32(ui.editMaxSeed->text(), finder.max_seed))
    {
        QMessageBox::critical(this, "Error", "Bad max seed value.");
        return;
    }
    if (finder.max_seed < finder.min_seed)
    {
        QMessageBox::critical(this, "Error", "Max seed cannot be smaller than min seed.");
        return;
    }
    finder.stars = ui.comboBoxStars->currentIndex() + 1;
    finder.species = species_lookup[ui.comboBoxSpecies->currentIndex()];
    finder.shiny = ui.comboBoxShiny->currentIndex();
    finder.tera_type = ui.comboBoxTeraType->currentIndex();
    finder.ability = ui.comboBoxAbility->currentIndex();
    finder.nature = ui.comboBoxNature->currentIndex();
    finder.gender = ui.comboBoxGender->currentIndex();
    QSpinBox* min_widgets[] = {
        ui.spinBoxMinHP,
        ui.spinBoxMinAtk,
        ui.spinBoxMinDef,
        ui.spinBoxMinSpA,
        ui.spinBoxMinSpD,
        ui.spinBoxMinSpe,
    };
    for (size_t i = 0; i < _countof(min_widgets); ++i)
        finder.min_iv[i] = min_widgets[i]->value();
    QSpinBox* max_widgets[] = {
        ui.spinBoxMaxHP,
        ui.spinBoxMaxAtk,
        ui.spinBoxMaxDef,
        ui.spinBoxMaxSpA,
        ui.spinBoxMaxSpD,
        ui.spinBoxMaxSpe,
    };
    for (size_t i = 0; i < _countof(max_widgets); ++i)
        finder.max_iv[i] = max_widgets[i]->value();
    finder.use_item_filters = ui.checkboxItemFilters->isChecked();
    finder.drop_threshold = ui.spinBoxMinItemsSum->value();
    itemFilters->update_seed_finder(finder);
    toggle_ui(false);
    if (!finder.find_seeds())
    {
        QMessageBox::critical(this, "Error", "Failed to start seed finder.");
        toggle_ui(true);
        return;
    }
    finder_timer->start(250);
}

void RaidCalc::on_buttonEditFilters_clicked()
{
    itemFilters->show();
}

void RaidCalc::on_finder_timer_timeout()
{
    if (!finder.is_search_done())
        return;
    finder_timer->stop();
    QString msg = QString("Found %1 seeds in %2ms.").arg(QString::number(finder.seeds.size()), QString::number(finder.time_taken.milliseconds()));
    QMessageBox::information(this, "Success", msg);
    QStringList headers;
    headers << "Seed" << "Species" << "Shiny" << "EC" << "PID" << "HP" << "Atk" << "Def" << "SpA" << "SpD" << "Spe" << "Drops";
    seedModel->insertRows(0, finder.seeds.size());
    seedModel->setHorizontalHeaderLabels(headers);
    for (size_t i = 0; i < finder.seeds.size(); ++i)
    {
        SeedFinder::SeedInfo info = finder.get_seed_info(finder.seeds[i]);
        seedModel->setItem(i, 0, readonly_item(format_uint32(info.seed)));
        seedModel->setItem(i, 1, readonly_item(pokemon_names[info.species]));
        seedModel->setItem(i, 2, readonly_item(info.shiny ? "Yes" : "No"));
        seedModel->setItem(i, 3, readonly_item(format_uint32(info.ec)));
        seedModel->setItem(i, 4, readonly_item(format_uint32(info.pid)));
        for (int index = 0; index < _countof(info.iv); ++index)
            seedModel->setItem(i, 5 + index, readonly_item(QString::number(info.iv[index])));
        seedModel->setItem(i, 11, readonly_item(QString::number(info.drops)));
    }
    toggle_ui(true);
}
