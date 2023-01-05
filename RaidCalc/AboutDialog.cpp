#include "AboutDialog.h"

AboutDialog::AboutDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	if (RAIDCALC_VERSION[0] != '\0')
		ui.labelVersion->setText(ui.labelVersion->text() + " " + RAIDCALC_VERSION);
	setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
}

AboutDialog::~AboutDialog()
{

}

void AboutDialog::on_buttonOk_clicked()
{
	close();
}
