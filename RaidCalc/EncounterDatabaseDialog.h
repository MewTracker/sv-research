#pragma once

#include <QDialog>
#include "ui_EncounterDatabaseDialog.h"
#include "EncounterEntryModel.h"

class EncounterDatabaseDialog : public QDialog
{
	Q_OBJECT

public:
	EncounterDatabaseDialog(QWidget *parent = nullptr);
	~EncounterDatabaseDialog();

public slots:
	void on_comboBoxSpecies_currentIndexChanged(int index);
	void on_tableEncounters_doubleClicked(const QModelIndex &index);

signals:
	void parameterChangeRequested(EncounterEntry entry, int32_t species);

private:
	Ui::EncounterDatabaseDialog ui;
	EncounterEntryModel encounter_model;
};
