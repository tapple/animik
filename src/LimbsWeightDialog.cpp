#include "LimbsWeightDialog.h"
#include "ui_LimbsWeightDialog.h"
#include "bvhnode.h"



LimbsWeightDialog::LimbsWeightDialog(QString animationName, QList<BVHNode*>* limbs, int frame,
                                     QWidget *parent)
  : QDialog(parent), ui(new Ui::LimbsWeightDialog)
{
  this->frame = frame;
  this->limbs = limbs;

  //just in case all limbs have the same weight, set _weight to that value. Otherwise defaults to 50.
  int commonWeight = limbs->at(0)->frameData(frame).weight();
  for(int i=1; i<limbs->count(); i++)
  {
    if(limbs->at(i)->frameData(frame).weight() != commonWeight)
    {
      commonWeight = 50;
      break;
    }
  }
  _weight = commonWeight;

  ui->setupUi(this);
  foreach(BVHNode* limb, *limbs)
    ui->limbNamesListWidget->addItem(limb->name());

  ui->weightSlider->setValue(_weight);
  ui->animNameLabel->setText(animationName);
  ui->weightLabel->setText(QString::number(_weight));
  ui->frameNumberLabel->setText(QString::number(frame+1));
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

void LimbsWeightDialog::accept()
{
  QDialog::accept();
  foreach(BVHNode* limb, *limbs)
    limb->setKeyframeWeight(frame, _weight);
}
