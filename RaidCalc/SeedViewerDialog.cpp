#include "SeedViewerDialog.h"
#include "PokemonNames.hpp"
#include "Utils.h"

SeedViewerDialog::SeedViewerDialog(QWidget* parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	is_seed_valid = false;
	refresh_supressed = false;
	current_seed = 0;
}

SeedViewerDialog::~SeedViewerDialog()
{

}

void SeedViewerDialog::on_comboBoxGame_currentIndexChanged(int index)
{
	refresh_ui();
}

void SeedViewerDialog::on_comboBoxRaidType_currentIndexChanged(int index)
{
	refresh_ui();
}

void SeedViewerDialog::on_editSeed_textChanged(const QString& text)
{
	is_seed_valid = hex_to_uint32(text, current_seed);
	refresh_ui();
}

void SeedViewerDialog::display_seed(Game game, int stars, uint32_t seed)
{
	refresh_supressed = true;
	ui.comboBoxGame->setCurrentIndex((int)game);
	ui.comboBoxRaidType->setCurrentIndex(stars == 6 ? 1 : 0);
	ui.editSeed->setText(format_uint32(seed));
	refresh_supressed = false;
	refresh_ui();
}

void SeedViewerDialog::refresh_ui()
{
	if (refresh_supressed)
		return;
	if (!is_seed_valid)
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
		};
		for (auto label : labels)
			label->setText("Invalid");
		ui.infoIV->setText("0/0/0/0/0/0");
		ui.listRewards->clear();
		return;
	}
	finder.game = (Game)ui.comboBoxGame->currentIndex();
	finder.stars = ui.comboBoxRaidType->currentIndex() == 1 ? 6 : SeedFinder::get_star_count(current_seed);
	SeedFinder::SeedInfo info = finder.get_seed_info(current_seed);
	ui.infoSpecies->setText(pokemon_names[info.species]);
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
	ui.listRewards->clear();
	auto rewards = finder.get_all_rewards(current_seed, SeedFinder::StoryProgress, SeedFinder::RaidBoost);
	for (auto &reward : rewards)
	{
		QString item_name("Invalid item");
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
