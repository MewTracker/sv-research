#pragma once

#include <QDialog>
#include "ui_AboutDialog.h"

class AboutDialog : public QDialog
{
	Q_OBJECT

public:
	AboutDialog(QWidget *parent = nullptr);
	~AboutDialog();

public slots:
	void on_buttonOk_clicked();

private:
	Ui::AboutDialog ui;
};
