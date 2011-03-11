#include <QHashIterator>
#include "LimbsWeightDialog.h"
#include "ui_LimbsWeightDialog.h"
#include "bvhnode.h"



LimbsWeightDialog::LimbsWeightDialog(QString animationName, QList<BVHNode*>* limbs, int frame, int totalFrames, QWidget *parent)
  : QDialog(parent), ui(new Ui::LimbsWeightDialog)
{
  currentFrame = frame;
  this->totalFrames = totalFrames;
  this->limbs = limbs;

  ui->setupUi(this);
  foreach(BVHNode* limb, *limbs)
    ui->limbNamesListWidget->addItem(limb->name());

  initComponents();
  ui->animNameLabel->setText(animationName);
}

LimbsWeightDialog::~LimbsWeightDialog()
{
  delete ui;
}


void LimbsWeightDialog::initComponents()
{
  int commonWeight;
  if(_weights.contains(currentFrame))       //user has already set weight for this frame
  {
    commonWeight = _weights[currentFrame];
    ui->setByUserLabel->setVisible(true);
  }
  else
  {
    //just in case all limbs have the same weight, set commonWeight to that value. Otherwise defaults to 50.
    commonWeight = limbs->at(0)->frameData(currentFrame).weight();
    for(int i=1; i<limbs->count(); i++)
    {
      if(limbs->at(i)->frameData(currentFrame).weight() != commonWeight)
      {
        commonWeight = 50;
        break;
      }
    }

    ui->setByUserLabel->setVisible(false);
//    _weights->insert(frame, commonWeight);        //edu: not done here. The value is saved only when user changes it.
  }

  ui->weightSlider->blockSignals(true);
  ui->weightSlider->setValue(commonWeight);
  ui->weightSlider->blockSignals(false);

  ui->weightLabel->setText(QString::number(commonWeight));
  ui->frameNumberLabel->setText(QString::number(currentFrame+1) + "/" + QString::number(totalFrames));
  ui->prevFrameButton->setEnabled(currentFrame != 0);
  ui->nextFrameButton->setEnabled(currentFrame < totalFrames-1);
}


void LimbsWeightDialog::on_weightSlider_valueChanged(int value)
{
  _weights[currentFrame] = value;           //new key/value pair is quietly inserted if not there yet
  ui->weightLabel->setText(QString::number(value));
  ui->setByUserLabel->setVisible(true);
}

void LimbsWeightDialog::accept()
{
  QDialog::accept();

  //apply saved values to all frames that have adjustments
  QHashIterator<int, int> iter(_weights);
  while (iter.hasNext())
  {
    iter.next();
    foreach(BVHNode* limb, *limbs)
      limb->setKeyframeWeight(iter.key(), iter.value());
  }
}

void LimbsWeightDialog::on_nextFrameButton_clicked()
{
  currentFrame++;
  initComponents();
}

void LimbsWeightDialog::on_prevFrameButton_clicked()
{
  currentFrame--;
  initComponents();
}
