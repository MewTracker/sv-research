#include "ItemFilterDialog.h"
#include "RaidRewards.h"

ItemFilterDialog::ItemFilterDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
    tableModel = new QStandardItemModel(_countof(reward_info), 1, this);
	for (size_t i = 0; i < _countof(reward_info); ++i)
	{
        QStandardItem* item = new QStandardItem(reward_info[i].name);
        item->setCheckable(true);
        item->setEditable(false);
		tableModel->setItem(i, 0, item);
	}
    ui.itemFilterTable->setModel(tableModel);
}

ItemFilterDialog::~ItemFilterDialog()
{

}

void ItemFilterDialog::on_buttonClearAll_clicked()
{
	for (int i = 0; i < tableModel->rowCount(); ++i)
	{
		QStandardItem* item = tableModel->item(i, 0);
		item->setCheckState(Qt::Unchecked);
	}
}

void ItemFilterDialog::update_seed_finder(SeedFinder& finder)
{
	for (int i = 0; i < tableModel->rowCount(); ++i)
	{
		QStandardItem* item = tableModel->item(i, 0);
		finder.set_drop_filter(reward_info[i].item_id, item->checkState() == Qt::Checked);
	}
}
