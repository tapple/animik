#include "playeroptionsdialog.h"
#include "ui_playeroptionsdialog.h"

PlayerOptionsDialog::PlayerOptionsDialog(int fps, bool loop, QWidget *parent)
  : QDialog(parent), ui(new Ui::PlayerOptionsDialog)
{
  ui->setupUi(this);

  _fps = fps;
  _loop = loop;

  ui->fpsSpinBox->setValue(_fps);
  ui->loopCheckBox->setChecked(_loop);
}

PlayerOptionsDialog::~PlayerOptionsDialog()
{
  delete ui;
}


void PlayerOptionsDialog::on_fpsSpinBox_valueChanged(int value)
{
  _fps = value;
}

void PlayerOptionsDialog::on_loopCheckBox_toggled(bool checked)
{
  _loop = checked;
}
