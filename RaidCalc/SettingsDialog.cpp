#include <QHeaderView>
#include "SettingsDialog.h"

SettingsDialog::SettingsDialog(QWidget *parent, QTableView* result_table)
	: QDialog(parent), result_table(result_table)
{
	ui.setupUi(this);
	settings = new QSettings("RaidCalc.ini", QSettings::IniFormat, this);
	auto model = result_table->model();
	for (int i = 0; i < model->columnCount(); ++i)
	{
		QListWidgetItem* item = new QListWidgetItem(model->headerData(i, Qt::Horizontal).toString());
		item->setData(Qt::UserRole, QVariant(i));
		ui.listActive->addItem(item);
	}
	load_settings();
}

SettingsDialog::~SettingsDialog()
{

}

void SettingsDialog::on_buttonOk_clicked()
{
	QVariantList hidden_columns;
	for (int i = 0; i < ui.listHidden->count(); ++i)
		hidden_columns.append(ui.listHidden->item(i)->data(Qt::UserRole).toInt());
	settings->setValue("hidden_columns", hidden_columns);
	load_settings();
	close();
}

void SettingsDialog::on_buttonCancel_clicked()
{
	load_settings();
	close();
}

void SettingsDialog::on_buttonShowColumn_clicked()
{
	move_item(ui.listHidden, ui.listActive, ui.listHidden->currentItem());
}

void SettingsDialog::on_buttonHideColumn_clicked()
{
	move_item(ui.listActive, ui.listHidden, ui.listActive->currentItem());
}

void SettingsDialog::load_settings()
{
	auto header_view = result_table->horizontalHeader();
	auto model = result_table->model();
	while (ui.listHidden->count())
		move_item(ui.listHidden, ui.listActive, 0);
	for (int i = 0; i < model->columnCount(); ++i)
		header_view->showSection(i);
	for (auto variant : settings->value("hidden_columns").toList())
	{
		int column_index = variant.toInt();
		move_item(ui.listActive, ui.listHidden, get_item_by_column_index(ui.listActive, column_index));
		header_view->hideSection(column_index);
	}
}

int SettingsDialog::get_item_by_column_index(QListWidget* list, int column_index)
{
	for (int i = 0; i < list->count(); ++i)
		if (list->item(i)->data(Qt::UserRole).toInt() == column_index)
			return i;
	return -1;
}

void SettingsDialog::move_item(QListWidget* from, QListWidget* to, QListWidgetItem* item)
{
	if (!item)
		return;
	int column_index = item->data(Qt::UserRole).toInt();
	int row = get_item_by_column_index(from, column_index);
	move_item(from, to, row);
}

void SettingsDialog::move_item(QListWidget* from, QListWidget* to, int row)
{
	if (row < 0)
		return;
	auto item = from->takeItem(row);
	int column_index = item->data(Qt::UserRole).toInt();
	bool inserted = false;
	for (int i = 0; i < to->count(); ++i)
	{
		if (to->item(i)->data(Qt::UserRole).toInt() > column_index)
		{
			to->insertItem(i, item);
			inserted = true;
			break;
		}
	}
	if (!inserted)
		to->addItem(item);
}
