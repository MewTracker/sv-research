#include "SeedViewerDialog.h"
#include "PokemonNames.h"
#include "FormUtils.h"
#include "Utils.h"

SeedViewerDialog::SeedViewerDialog(QWidget* parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	is_seed_valid = false;
	refresh_supressed = false;
	current_seed = 0;
	ui.comboBoxEvent->addItem("None");
	for (auto event_name : event_names)
		ui.comboBoxEvent->addItem(event_name);
	set_event_group_visible(false);
}

SeedViewerDialog::~SeedViewerDialog()
{

}

void SeedViewerDialog::on_comboBoxGame_currentIndexChanged(int index)
{
	refresh_ui();
}

void SeedViewerDialog::on_comboBoxEvent_currentIndexChanged(int index)
{
	if (!refresh_supressed)
	{
		ui.comboBoxStage->blockSignals(true);
		ui.comboBoxStage->clear();
		ui.comboBoxEventGroup->blockSignals(true);
		ui.comboBoxEventGroup->clear();
		ui.comboBoxEventGroup->blockSignals(false);
		if (index == 0)
		{
			ui.labelStage->setText("Story progress:");
			for (auto& stage_name : stage_names_story)
				ui.comboBoxStage->addItem(stage_name);
			set_event_group_visible(false);
		}
		else
		{
			ui.labelStage->setText("Event progress:");
			for (auto& stage_name : stage_names_event)
				ui.comboBoxStage->addItem(stage_name);
			auto group = SeedFinder::get_event_info(index - 1);
			ui.comboBoxEventGroup->blockSignals(true);
			for (auto id : group->dist)
				ui.comboBoxEventGroup->addItem(QString::number(id), id);
			ui.comboBoxEventGroup->blockSignals(false);
			set_event_group_visible(ui.comboBoxEventGroup->count() > 1);
		}
		ui.comboBoxStage->setCurrentIndex(ui.comboBoxStage->count() - 1);
		ui.comboBoxStage->blockSignals(false);
		ui.comboBoxRaidType->blockSignals(true);
		ui.comboBoxRaidType->setCurrentIndex(1);
		ui.comboBoxRaidType->blockSignals(false);
	}
	refresh_ui();
}

void SeedViewerDialog::on_comboBoxEventGroup_currentIndexChanged(int index)
{
	refresh_ui();
}

void SeedViewerDialog::on_comboBoxRaidType_currentIndexChanged(int index)
{
	if (!refresh_supressed)
	{
		if (index == 1 && ui.comboBoxStage->currentIndex() != ui.comboBoxStage->count() - 1)
		{
			ui.comboBoxStage->setCurrentIndex(ui.comboBoxStage->count() - 1);
			QApplication::beep();
		}
		set_event_group_visible(index != 1 && ui.comboBoxEventGroup->count() > 1);
	}
	refresh_ui();
}

void SeedViewerDialog::on_comboBoxStage_currentIndexChanged(int index)
{
	if (!refresh_supressed)
	{
		if (index != ui.comboBoxStage->count() - 1 && ui.comboBoxRaidType->currentIndex() == 1)
		{
			ui.comboBoxRaidType->setCurrentIndex(0);
			QApplication::beep();
		}
	}
	refresh_ui();
}

void SeedViewerDialog::on_spinBoxRaidBoost_valueChanged(int value)
{
	refresh_ui();
}

void SeedViewerDialog::on_editSeed_textChanged(const QString& text)
{
	is_seed_valid = hex_to_uint32(text, current_seed);
	refresh_ui();
}

void SeedViewerDialog::display_seed(SeedFinder::BasicParams params, uint32_t seed)
{
	ui.comboBoxEvent->setCurrentIndex(params.event_id + 1);
	refresh_supressed = true;
	select_option(ui.comboBoxEventGroup, params.event_group);
	ui.comboBoxGame->setCurrentIndex((int)params.game);
	ui.comboBoxRaidType->setCurrentIndex(params.stars >= 6 ? 1 : 0);
	ui.comboBoxStage->setCurrentIndex(params.stage);
	ui.spinBoxRaidBoost->setValue(params.raid_boost);
	ui.editSeed->setText(format_uint32(seed));
	refresh_supressed = false;
	refresh_ui();
}

void SeedViewerDialog::set_invalid_state()
{
	QLabel* labels[] = {
		ui.infoSpecies,
		ui.infoDifficulty,
		ui.infoEC,
		ui.infoPID,
		ui.infoShiny,
		ui.infoTeraType,
		ui.infoGender,
		ui.infoNature,
		ui.infoAbility,
		ui.infoMove1,
		ui.infoMove2,
		ui.infoMove3,
		ui.infoMove4,
		ui.infoHeight,
		ui.infoWeight,
		ui.infoScale,
	};
	for (auto label : labels)
		label->setText("Invalid");
	ui.infoIV->setText("0/0/0/0/0/0");
	ui.listRewards->clear();
}

void SeedViewerDialog::refresh_ui()
{
	if (refresh_supressed)
		return;
	if (!is_seed_valid)
	{
		set_invalid_state();
		return;
	}
	finder.game = (Game)ui.comboBoxGame->currentIndex();
	finder.event_id = ui.comboBoxEvent->currentIndex() - 1;
	finder.event_group = ui.comboBoxEventGroup->currentData().toUInt();
	finder.stage = ui.comboBoxStage->currentIndex();
	finder.raid_boost = ui.spinBoxRaidBoost->value();
	finder.stars = ui.comboBoxRaidType->currentIndex() == 1 ? (finder.event_id < 0 ? 6 : 7) : SeedFinder::get_star_count(current_seed, finder.stage, finder.event_id, finder.game);
	SeedFinder::SeedInfo info = finder.get_seed_info(current_seed);
	if (info.species == 0)
	{
		set_invalid_state();
		return;
	}
	ui.infoSpecies->setText(FormUtils::get_pokemon_name(info.species, info.form, info.ec).c_str());
	ui.infoDifficulty->setText(QString("%1 star%2").arg(QString::number(info.stars), info.stars > 1 ? "s" : ""));
	ui.infoEC->setText(format_uint32(info.ec));
	ui.infoPID->setText(format_uint32(info.pid));
	ui.infoShiny->setText(info.shiny ? "Yes" : "No");
	ui.infoTeraType->setText(type_names[info.tera_type]);
	ui.infoGender->setText(gender_names[info.gender]);
	ui.infoNature->setText(nature_names[info.nature]);
	ui.infoAbility->setText(ability_names[info.ability]);
	ui.infoIV->setText(QString("%1/%2/%3/%4/%5/%6").arg(
		QString::number(info.iv[0]),
		QString::number(info.iv[1]),
		QString::number(info.iv[2]),
		QString::number(info.iv[3]),
		QString::number(info.iv[4]),
		QString::number(info.iv[5])));
	QLabel* moves[] = { ui.infoMove1, ui.infoMove2, ui.infoMove3, ui.infoMove4 };
	for (size_t i = 0; i < _countof(moves); ++i)
		moves[i]->setText(move_names[info.moves[i]]);
	ui.infoHeight->setText(QString::number(info.height));
	ui.infoWeight->setText(QString::number(info.weight));
	ui.infoScale->setText(QString::number(info.scale));
	ui.listRewards->clear();
	auto rewards = finder.get_all_rewards(current_seed);
	for (auto &reward : rewards)
	{
		QString item_name = QString("Invalid item (%1)").arg(reward.item_id);
		for (auto& info : reward_info)
		{
			if (info.item_id == reward.item_id)
			{
				item_name = info.name;
				break;
			}
		}
		if (reward.count > 1)
			item_name += QString(" (x%1)").arg(reward.count);
		ui.listRewards->addItem(item_name);
	}
}

void SeedViewerDialog::set_event_group_visible(bool visibility)
{
	ui.labelEventGroup->setVisible(visibility);
	ui.comboBoxEventGroup->setVisible(visibility);
}
