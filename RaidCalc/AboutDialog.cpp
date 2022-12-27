#include "AboutDialog.h"

AboutDialog::AboutDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
}

AboutDialog::~AboutDialog()
{

}

void AboutDialog::on_buttonOk_clicked()
{
	close();
}
