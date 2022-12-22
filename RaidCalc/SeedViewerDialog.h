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

	void display_seed(Game game, int stars, uint32_t seed);

public slots:
	void on_comboBoxGame_currentIndexChanged(int index);
	void on_comboBoxRaidType_currentIndexChanged(int index);
	void on_editSeed_textChanged(const QString& text);

private:
	void refresh_ui();

	Ui::SeedViewerDialog ui;
	uint32_t current_seed;
	bool is_seed_valid;
	bool refresh_supressed;
	SeedFinder finder;
};
