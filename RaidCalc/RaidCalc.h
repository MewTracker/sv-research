#pragma once

#include <QtWidgets/QMainWindow>
#include <QTimer>
#include "ui_RaidCalc.h"
#include "ItemFilterDialog.h"
#include "SeedViewerDialog.h"
#include "SeedFinder.h"
#include "SeedTableModel.h"

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
    void on_actionSeedViewer_triggered(bool checked = false);
    void on_tableSeeds_doubleClicked(const QModelIndex& index);

private:
    static const uint64_t MaxSeeds = 10000000ULL;
    static const uint64_t SeedCountWarningThreshold = 100000;

    void toggle_ui(bool enabled);
    void add_sorted_options(QComboBox* combo, const char** names, uint32_t name_count, uint32_t offset = 1);
    void add_sorted_options(QComboBox* combo, std::vector<std::pair<std::string, uint32_t>>& options);

    Ui::RaidCalcClass ui;
    QSpinBox* min_iv_widgets[6];
    QSpinBox* max_iv_widgets[6];
    ItemFilterDialog* itemFilters;
    SeedViewerDialog* seedViewer;
    QTimer* finder_timer;
    SeedTableModel seedModel;
    SeedFinder finder;
    SeedFinder::BasicParams resultParams;
};
