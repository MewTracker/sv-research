#pragma once

#include <QtWidgets/QMainWindow>
#include <QTimer>
#include <set>
#include "ui_RaidCalc.h"
#include "ItemFilterDialog.h"
#include "SeedViewerDialog.h"
#include "SeedFinder.h"
#include "SeedTableModel.h"
#include "SettingsDialog.h"
#include "AboutDialog.h"
#include "EncounterDatabaseDialog.h"

class RaidCalc : public QMainWindow
{
    Q_OBJECT

public:
    RaidCalc(QWidget *parent = nullptr);
    ~RaidCalc();

public slots:
    void on_buttonFindSeeds_clicked();
    void on_buttonEditFilters_clicked();
    void on_buttonResetPokemonFilters_clicked();
    void on_buttonMaxIV_clicked();
    void on_finder_timer_timeout();
    void on_actionExportSeeds_triggered(bool checked = false);
    void on_actionSeedViewer_triggered(bool checked = false);
    void on_actionSettings_triggered(bool checked = false);
    void on_actionAbout_triggered(bool checked = false);
    void on_actionEncounterDatabase_triggered(bool checked = false);
    void on_tableSeeds_doubleClicked(const QModelIndex& index);
    void on_comboBoxEvent_currentIndexChanged(int index);
    void on_comboBoxStage_currentIndexChanged(int index);
    void on_comboBoxStars_currentIndexChanged(int index);
    void on_parameterChangeRequested(EncounterEntry entry, int32_t species);

private:
    static const uint64_t MaxSeeds = 10000000ULL;
    static const uint64_t SeedCountWarningThreshold = 100000;
    static const size_t MaxBufferSize = 1000000;

    struct StarsRange
    {
        uint8_t min_stars;
        uint8_t max_stars;
    };

    union SpeciesFilter
    {
        struct
        {
            uint32_t species : 16;
            uint32_t form : 8;
            uint32_t any_form : 1;
            uint32_t rare_form : 1;
            uint32_t common_form : 1;
        };
        uint32_t value;
    };

    void toggle_ui(bool enabled);
    void create_species_filters(std::set<uint32_t>& encounterables, std::vector<std::pair<std::string, uint32_t>>& filters);
    void add_sorted_options(QComboBox* combo, const char** names, uint32_t name_count, uint32_t offset = 1);
    void add_sorted_options(QComboBox* combo, std::vector<std::pair<std::string, uint32_t>>& options);
    void add_options(QComboBox* combo, std::vector<std::pair<std::string, uint32_t>>& options);
    const StarsRange& get_allowed_stars(int progress);
    void fix_progress(int stars);
    void set_event_group_visible(bool visibility);

    Ui::RaidCalcClass ui;
    QSpinBox* min_iv_widgets[6];
    QSpinBox* max_iv_widgets[6];
    ItemFilterDialog* itemFilters;
    SeedViewerDialog* seedViewer;
    SettingsDialog* settings;
    AboutDialog* about;
    EncounterDatabaseDialog* encounterDb;
    QTimer* finder_timer;
    SeedTableModel seedModel;
    SeedFinder finder;
    SeedFinder::BasicParams resultParams;
    std::vector<std::pair<std::string, uint32_t>> species_filters[_countof(event_names) + 1];
};
