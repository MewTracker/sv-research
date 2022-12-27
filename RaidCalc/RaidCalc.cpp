#include <QMessageBox>
#include <QStandardItemModel>
#include <set>
#include <vector>
#include <algorithm>
#include "RaidCalc.h"
#include "PokemonNames.h"
#include "Benchmarks.h"

RaidCalc::RaidCalc(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    if (!SeedFinder::initialize())
    {
        QMessageBox::critical(this, "Error", "Failed to initialize seed finder.");
        QTimer::singleShot(0, qApp, &QCoreApplication::quit);
    }
    std::set<uint32_t> encounterable_species;
    finder.visit_encounters([&](const EncounterTera9& enc) { encounterable_species.insert(enc.species); });
    std::vector<std::pair<std::string, uint32_t>> species_data;
    for (auto& species : encounterable_species)
        species_data.push_back({ pokemon_names[species], species });
    add_sorted_options(ui.comboBoxSpecies, species_data);
    add_sorted_options(ui.comboBoxTeraType, type_names, _countof(type_names));
    add_sorted_options(ui.comboBoxAbility, ability_names + 1, _countof(ability_names) - 1);
    add_sorted_options(ui.comboBoxNature, nature_names, _countof(nature_names));
    ui.tableSeeds->setModel(&seedModel);
    itemFilters = new ItemFilterDialog(this);
    seedViewer = new SeedViewerDialog(this);
    settings = new SettingsDialog(this, ui.tableSeeds);
    about = new AboutDialog(this);
    finder_timer = new QTimer(this);
    connect(finder_timer, &QTimer::timeout, this, &RaidCalc::on_finder_timer_timeout);
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    for (uint32_t i = 0; i < sys_info.dwNumberOfProcessors - 1; ++i)
        ui.comboBoxThreads->addItem(QString::number(i + 1));
    ui.comboBoxThreads->setCurrentIndex(ui.comboBoxThreads->count() - 1);

    min_iv_widgets[0] = ui.spinBoxMinHP;
    min_iv_widgets[1] = ui.spinBoxMinAtk;
    min_iv_widgets[2] = ui.spinBoxMinDef;
    min_iv_widgets[3] = ui.spinBoxMinSpA;
    min_iv_widgets[4] = ui.spinBoxMinSpD;
    min_iv_widgets[5] = ui.spinBoxMinSpe;
    max_iv_widgets[0] = ui.spinBoxMaxHP;
    max_iv_widgets[1] = ui.spinBoxMaxAtk;
    max_iv_widgets[2] = ui.spinBoxMaxDef;
    max_iv_widgets[3] = ui.spinBoxMaxSpA;
    max_iv_widgets[4] = ui.spinBoxMaxSpD;
    max_iv_widgets[5] = ui.spinBoxMaxSpe;

    do_benchmarks(finder);
}

RaidCalc::~RaidCalc()
{

}

void RaidCalc::add_sorted_options(QComboBox* combo, const char** names, uint32_t name_count, uint32_t offset)
{
    std::vector<std::pair<std::string, uint32_t>> options;
    for (uint32_t i = 0; i < name_count; ++i)
        options.push_back({ names[i], i + offset });
    add_sorted_options(combo, options);
}

void RaidCalc::add_sorted_options(QComboBox* combo, std::vector<std::pair<std::string, uint32_t>>& options)
{
    std::sort(options.begin(), options.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    combo->addItem("Any", 0U);
    for (auto& pair : options)
        combo->addItem(pair.first.c_str(), pair.second);
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
    ui.menuBar->setEnabled(enabled);
    if (!enabled)
    {
        itemFilters->hide();
        seedViewer->hide();
        seedModel.clear();
    }
}

void RaidCalc::on_buttonFindSeeds_clicked()
{
    finder.game = (Game)ui.comboBoxGame->currentIndex();
    finder.stars = ui.comboBoxStars->currentIndex() + 1;
    finder.story_progress = ui.comboBoxStory->currentIndex();
    finder.raid_boost = ui.spinBoxRaidBoost->value();
    finder.thread_count = ui.comboBoxThreads->currentIndex() + 1;
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
    finder.species = ui.comboBoxSpecies->currentData().toUInt();
    finder.shiny = ui.comboBoxShiny->currentIndex();
    finder.tera_type = ui.comboBoxTeraType->currentData().toUInt();
    finder.ability = ui.comboBoxAbility->currentData().toUInt();
    finder.nature = ui.comboBoxNature->currentData().toUInt();
    finder.gender = ui.comboBoxGender->currentIndex();
    for (size_t i = 0; i < _countof(min_iv_widgets); ++i)
        finder.min_iv[i] = min_iv_widgets[i]->value();
    for (size_t i = 0; i < _countof(max_iv_widgets); ++i)
        finder.max_iv[i] = max_iv_widgets[i]->value();
    finder.item_filters_active = ui.checkboxItemFilters->isChecked();
    finder.drop_threshold = ui.spinBoxMinItemsSum->value();
    itemFilters->update_seed_finder(finder);
    if (!finder.use_filters())
    {
        uint64_t seed_count = finder.max_seed - finder.min_seed + 1ULL;
        if (seed_count > MaxSeeds)
        {
            QMessageBox::critical(this, "Error", QString("No filters set. Search aborted because it would exceed maximum allowed number of results (%1).").arg(MaxSeeds));
            return;
        }
        if (seed_count > SeedCountWarningThreshold)
        {
            auto result = QMessageBox::question(this, "Warning", "No filters set, this may result in a huge result set. Do you want to continue anyway?");
            if (result != QMessageBox::Yes)
                return;
        }
    }
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

void RaidCalc::on_buttonResetPokemonFilters_clicked()
{
    ui.comboBoxSpecies->setCurrentIndex(0);
    ui.comboBoxShiny->setCurrentIndex(0);
    ui.comboBoxTeraType->setCurrentIndex(0);
    ui.comboBoxAbility->setCurrentIndex(0);
    ui.comboBoxNature->setCurrentIndex(0);
    ui.comboBoxGender->setCurrentIndex(0);
    for (auto& widget : min_iv_widgets)
        widget->setValue(0);
    for (auto& widget : max_iv_widgets)
        widget->setValue(31);
}

void RaidCalc::on_buttonMaxIV_clicked()
{
    for (auto& widget : min_iv_widgets)
        widget->setValue(31);
    for (auto& widget : max_iv_widgets)
        widget->setValue(31);
}

void RaidCalc::on_finder_timer_timeout()
{
    if (!finder.is_search_done())
        return;
    finder_timer->stop();
    QString msg = QString("Found %1 seeds in %2ms.").arg(QString::number(finder.seeds.size()), QString::number(finder.time_taken.milliseconds()));
    QMessageBox::information(this, "Success", msg);
    seedModel.populateModel(finder);
    resultParams = finder.get_basic_params();
    toggle_ui(true);
}

void RaidCalc::on_actionSeedViewer_triggered(bool checked)
{
    seedViewer->show();
}

void RaidCalc::on_actionSettings_triggered(bool checked)
{
    settings->open();
}

void RaidCalc::on_actionAbout_triggered(bool checked)
{
    about->open();
}

void RaidCalc::on_tableSeeds_doubleClicked(const QModelIndex& index)
{
    seedViewer->display_seed(resultParams, seedModel.get_seed(index.row()));
    seedViewer->show();
}

const RaidCalc::StarsRange& RaidCalc::get_allowed_stars(int progress)
{
    static const StarsRange allowed_stars[5] =
    {
        { 1, 2 },
        { 1, 3 },
        { 1, 4 },
        { 3, 5 },
        { 3, 6 },
    };
    return allowed_stars[progress];
}

void RaidCalc::on_comboBoxStory_currentIndexChanged(int index)
{
    auto& range = get_allowed_stars(index);
    int stars = ui.comboBoxStars->currentIndex() + 1;
    if (stars < range.min_stars)
    {
        ui.comboBoxStars->setCurrentIndex(range.min_stars - 1);
        QApplication::beep();
    }
    if (stars > range.max_stars)
    {
        ui.comboBoxStars->setCurrentIndex(range.max_stars - 1);
        QApplication::beep();
    }
}

void RaidCalc::on_comboBoxStars_currentIndexChanged(int index)
{
    int stars = index + 1;
    int progress = ui.comboBoxStory->currentIndex();
    auto& range = get_allowed_stars(progress);
    if (stars >= range.min_stars && stars <= range.max_stars)
        return;
    for (int i = 0; i < 5; ++i)
    {
        auto& candidate = get_allowed_stars(i);
        if (stars >= candidate.min_stars && stars <= candidate.max_stars)
        {
            ui.comboBoxStory->setCurrentIndex(i);
            break;
        }
    }
    QApplication::beep();
}
