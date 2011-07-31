#include "optionalmessagebox.h"
#include "ui_optionalmessagebox.h"

OptionalMessageBox::OptionalMessageBox(QString title, QString text, QWidget* parent)
  : QDialog(parent), ui(new Ui::OptionalMessageBox)
{
    ui->setupUi(this);
    this->setWindowTitle(title);
    ui->textLabel->setText(text);
    _showAgain = true;
}

OptionalMessageBox::~OptionalMessageBox()
{
    delete ui;
}


void OptionalMessageBox::on_showAgainCheckBox_toggled(bool checked)
{
  _showAgain = checked;
}
