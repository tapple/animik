#include "LimbsWeightDialog.h"
#include "ui_LimbsWeightDialog.h"
#include "bvhnode.h"



LimbsWeightDialog::LimbsWeightDialog(QList<BVHNode*>* limbs, int weightHint, QWidget *parent)
  : QDialog(parent), ui(new Ui::LimbsWeightDialog)
{
  if(weightHint >= 0 && weightHint <= 100)
    _weight = weightHint;
  else _weight = 50;

  ui->setupUi(this);
  foreach(BVHNode* limb, *limbs)
    ui->limbNamesListWidget->addItem(limb->name());

  ui->weightSlider->setValue(_weight);
}

LimbsWeightDialog::~LimbsWeightDialog()
{
  delete ui;
}


void LimbsWeightDialog::on_weightSlider_valueChanged(int value)
{
  _weight = value;
  ui->weightLabel->setText(QString::number(_weight));
}
