#pragma once

#include <QDialog>
#include "ui_SeedViewerDialog.h"
#include "SeedFinder.h"

class SeedViewerDialog : public QDialog
{
	Q_OBJECT

public:
	SeedViewerDialog(QWidget* parent);
	~SeedViewerDialog();

	void display_seed(SeedFinder::BasicParams params, uint32_t seed);

public slots:
	void on_comboBoxGame_currentIndexChanged(int index);
	void on_comboBoxEvent_currentIndexChanged(int index);
	void on_comboBoxRaidType_currentIndexChanged(int index);
	void on_comboBoxStage_currentIndexChanged(int index);
	void on_spinBoxRaidBoost_valueChanged(int value);
	void on_editSeed_textChanged(const QString& text);

private:
	void refresh_ui();
	void set_invalid_state();

	Ui::SeedViewerDialog ui;
	uint32_t current_seed;
	bool is_seed_valid;
	bool refresh_supressed;
	SeedFinder finder;
};
