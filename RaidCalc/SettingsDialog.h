#pragma once

#include <QDialog>
#include <QTableView>
#include <QSettings>
#include "ui_SettingsDialog.h"

class SettingsDialog : public QDialog
{
	Q_OBJECT

public:
	SettingsDialog(QWidget *parent, QTableView* result_table);
	~SettingsDialog();

public slots:
	void on_buttonOk_clicked();
	void on_buttonCancel_clicked();
	void on_buttonShowColumn_clicked();
	void on_buttonHideColumn_clicked();

private:
	void load_settings();
	int get_item_by_column_index(QListWidget* list, int column_index);
	void move_item(QListWidget* from, QListWidget* to, QListWidgetItem* item);
	void move_item(QListWidget* from, QListWidget* to, int row);

	Ui::SettingsDialog ui;
	QTableView* result_table;
	QSettings* settings;
};
