#include "MixZonesDialog.h"
#include "ui_MixZonesDialog.h"



MixZonesDialog::MixZonesDialog(int frames, int mixIn, int mixOut, QWidget *parent)
  : QDialog(parent), ui(new Ui::MixZonesDialog)
{
  ui->setupUi(this);

  framesCount = frames;
  _mixIn = mixIn;
  _mixOut = mixOut;

  ui->mixInSlider->setRange(0, framesCount);
  ui->mixInSpinBox->setRange(0, framesCount);
  ui->mixOutSlider->setRange(0, framesCount);
  ui->mixOutSpinBox->setRange(0, framesCount);
  ui->mixInSlider->setValue(_mixIn);
  ui->mixInSpinBox->setValue(_mixIn);
  ui->mixOutSlider->setValue(_mixOut);
  ui->mixOutSpinBox->setValue(_mixOut);
}

MixZonesDialog::~MixZonesDialog() {
  delete ui;
}


void MixZonesDialog::on_mixInSlider_valueChanged(int value)
{
  _mixIn = value;
  if(_mixIn + _mixOut > framesCount)
    ui->mixOutSlider->setValue(framesCount - _mixIn);    //will do the job for Spinbox and mixOut as well

  ui->mixInSpinBox->setValue(_mixIn);
}

void MixZonesDialog::on_mixOutSlider_valueChanged(int value)
{
  _mixOut = value;
  if(_mixIn + _mixOut > framesCount)
    ui->mixInSlider->setValue(framesCount - _mixOut);

  ui->mixOutSpinBox->setValue(_mixOut);
}

void MixZonesDialog::on_mixInSpinBox_valueChanged(int value)
{
  ui->mixInSlider->setValue(value);   //it's dirty, I know
}


void MixZonesDialog::on_mixOutSpinBox_valueChanged(int value)
{
  ui->mixOutSlider->setValue(value);
}
