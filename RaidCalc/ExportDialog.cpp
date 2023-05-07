#include <QMessageBox>
#include <QFileDialog>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <algorithm>
#include <vector>
#include <string>
#include "ExportDialog.h"

ExportDialog::ExportDialog(QWidget *parent, SeedTableModel *seedModel)
    : QDialog(parent), seedModel(seedModel)
{
    ui.setupUi(this);
    settings = new QSettings("RaidCalc.ini", QSettings::IniFormat, this);
    if (settings->value("export_format_json").toBool())
        ui.buttonJson->setChecked(true);
    else
        ui.buttonCsv->setChecked(true);
    if (settings->value("include_rewards").toBool())
        ui.buttonRewardsYes->setChecked(true);
    else
        ui.buttonRewardsNo->setChecked(true);
}

ExportDialog::~ExportDialog()
{

}

void ExportDialog::set_params(const SeedFinder::BasicParams &params)
{
    finder.set_basic_params(params);
}

void ExportDialog::on_buttonCancel_clicked()
{
    close();
}

void ExportDialog::on_buttonExport_clicked()
{
    close();
    bool isJson = ui.buttonJson->isChecked();
    bool includeRewards = ui.buttonRewardsYes->isChecked();
    settings->setValue("export_format_json", isJson);
    settings->setValue("include_rewards", includeRewards);
    if (isJson)
        exportJson(includeRewards);
    else
        exportCsv(includeRewards);
}

void ExportDialog::exportCsv(bool includeRewards)
{
    QString path = QFileDialog::getSaveFileName(this, QString(), QString(), "Comma separated values (*.csv)");
    if (path.isEmpty())
        return;
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(this, "Error", "Failed to open file.");
        return;
    }
    ItemDatabase &itemDb = ItemDatabase::instance();
    std::string buffer;
    int rows = seedModel->rowCount();
    int columns = seedModel->columnCount();
    for (int i = 0; i < columns; ++i)
        buffer += seedModel->headerData(i, Qt::Horizontal).toString().toStdString() + ",";
    if (includeRewards)
        buffer += "Rewards,";
    buffer[buffer.size() - 1] = '\n';
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < columns; ++j)
            buffer += seedModel->data(seedModel->index(i, j)).toString().toStdString() + ",";
        if (includeRewards)
        {
            auto rewards = finder.get_all_rewards(seedModel->get_seed(i));
            for (auto &reward : rewards)
                buffer += std::to_string(reward.item_id) + "|" + itemDb.get_item_name(reward.item_id) + "|" + std::to_string(reward.count) + "|";
            if (!rewards.empty())
                buffer.pop_back();
            buffer += ",";
        }
        buffer[buffer.size() - 1] = '\n';
        if (buffer.size() > MaxBufferSize)
        {
            file.write(buffer.c_str());
            buffer.clear();
        }
    }
    if (!buffer.empty())
        file.write(buffer.c_str());
    file.close();
}

void ExportDialog::exportJson(bool includeRewards)
{
    QString path = QFileDialog::getSaveFileName(this, QString(), QString(), "JSON (*.json)");
    if (path.isEmpty())
        return;
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(this, "Error", "Failed to open file.");
        return;
    }
    QJsonArray seeds;
    ItemDatabase &itemDb = ItemDatabase::instance();
    std::vector<std::string> headers;
    int rows = seedModel->rowCount();
    int columns = seedModel->columnCount();
    for (int i = 0; i < columns; ++i)
    {
        std::string header = seedModel->headerData(i, Qt::Horizontal).toString().toStdString();
        std::transform(header.begin(), header.end(), header.begin(), [](unsigned char c) { return std::tolower(c); });
        std::replace(header.begin(), header.end(), ' ', '_');
        headers.push_back(header);
    }
    for (int i = 0; i < rows; ++i)
    {
        QVariantMap seedObject;
        for (int j = 0; j < columns; ++j)
            seedObject[headers[j].c_str()] = seedModel->data(seedModel->index(i, j));
        if (includeRewards)
        {
            QJsonArray rewardsArray;
            auto rewards = finder.get_all_rewards(seedModel->get_seed(i));
            for (auto &reward : rewards)
            {
                QVariantMap rewardObject;
                rewardObject["item_id"] = reward.item_id;
                rewardObject["name"] = itemDb.get_item_name(reward.item_id).c_str();
                rewardObject["count"] = reward.count;
                rewardsArray.append(QJsonObject::fromVariantMap(rewardObject));
            }
            seedObject["rewards"] = rewardsArray;
        }
        seeds.append(QJsonObject::fromVariantMap(seedObject));
    }
    QJsonObject root, meta;
    SeedFinder::BasicParams params = finder.get_basic_params();
    meta["game"] = params.game == GameScarlet ? "Scarlet" : "Violet";
    meta["event_id"] = params.event_id;
    meta["event_name"] = params.event_id >= 0 ? event_names[params.event_id] : "";
    meta["stars"] = params.stars;
    meta["stage"] = params.stage;
    meta["raid_boost"] = params.raid_boost;
    root["meta"] = meta;
    root["seeds"] = seeds;
    file.write(QJsonDocument(root).toJson());
    file.close();
}
