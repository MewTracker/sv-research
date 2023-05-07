#pragma once

#include <QDialog>
#include <QSettings>
#include "ui_ExportDialog.h"
#include "SeedTableModel.h"

class ExportDialog : public QDialog
{
	Q_OBJECT

public:
	ExportDialog(QWidget *parent, SeedTableModel *seedModel);
	~ExportDialog();

	void set_params(const SeedFinder::BasicParams &params);

public slots:
	void on_buttonCancel_clicked();
	void on_buttonExport_clicked();

private:
	static const size_t MaxBufferSize = 1000000;

	void exportCsv(bool includeRewards);
	void exportJson(bool includeRewards);

	Ui::ExportDialog ui;
	SeedFinder finder;
	SeedTableModel *seedModel;
	QSettings *settings;
};
