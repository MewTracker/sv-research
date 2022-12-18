#pragma once

#include <QDialog>
#include <QStandardItemModel>
#include "ui_ItemFilterDialog.h"
#include "SeedFinder.hpp"

class ItemFilterDialog : public QDialog
{
	Q_OBJECT

public:
	ItemFilterDialog(QWidget *parent);
	~ItemFilterDialog();

	void update_seed_finder(SeedFinder& finder);

public slots:
	void on_buttonClearAll_clicked();

private:
	Ui::ItemFilterDialog ui;
	QStandardItemModel* tableModel;
};
