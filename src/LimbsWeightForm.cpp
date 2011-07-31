#include "bvh.h"
#include "bvhnode.h"
#include "weightedanimation.h"
#include "limbsweightform.h"
#include "ui_limbsweightform.h"

LimbsWeightForm::LimbsWeightForm(QWidget *parent) : QWidget(parent), ui(new Ui::LimbsWeightForm)
{
  ui->setupUi(this);

  ui->limbComboBox->blockSignals(true);
  ui->limbComboBox->addItem("position");
  ui->limbComboBox->addItems(BVH::getValidNodeNames());
  ui->limbComboBox->removeItem(ui->limbComboBox->findText("Site"));    //I don't want "Site" to be there
  ui->limbComboBox->blockSignals(false);
  labels.insert("position", ui->positionWLabel);                       //TODO: I don't know, I just don't like this hard code.
  labels.insert("hip", ui->hipWLabel);
  labels.insert("abdomen", ui->abdomenWLabel);
  labels.insert("chest", ui->chestWLabel);
  labels.insert("neck", ui->neckWLabel);
  labels.insert("head", ui->headWLabel);
  labels.insert("lCollar", ui->lCollarWLabel);
  labels.insert("lShldr", ui->lShldrWLabel);
  labels.insert("lForeArm", ui->lForeArmWLabel);
  labels.insert("lHand", ui->lHandWLabel);
  labels.insert("rCollar", ui->rCollarWLabel);
  labels.insert("rShldr", ui->rShldrWLabel);
  labels.insert("rForeArm", ui->rForeArmWLabel);
  labels.insert("rHand", ui->rHandWLabel);
  labels.insert("lThigh", ui->lThighWLabel);
  labels.insert("lShin", ui->lShinWLabel);
  labels.insert("lFoot", ui->lFootWLabel);
  labels.insert("rThigh", ui->rThighWLabel);
  labels.insert("rShin", ui->rShinWLabel);
  labels.insert("rFoot", ui->rFootWLabel);
}

LimbsWeightForm::~LimbsWeightForm()
{
  delete ui;
}


void LimbsWeightForm::UpdateContent(WeightedAnimation* animation, int frame)
{
  if(animation == NULL)
  {
    anim = animation;
    ui->contentWidget->setEnabled(false);
  }
  else
  {
    ui->contentWidget->setEnabled(true);
    anim = animation;
    selectedLimb = anim->getNodeByName(ui->limbComboBox->currentText());
    this->frame = frame;
    updateLabelsHelper(anim->getNode(0));
    updateLabelsHelper(anim->getMotion());
    ui->weightSlider->setValue(selectedLimb->frameData(frame).weight());
  }
}


void LimbsWeightForm::updateLabelsHelper(BVHNode* limb)
{
  labels.value(limb->name())->setText(QString::number(limb->frameData(frame).weight()));
  for(int i=0; i<limb->numChildren(); i++)
  {
    if(limb->child(i)->name() != "Site")
      updateLabelsHelper(limb->child(i));
  }
}

void LimbsWeightForm::on_hideButton_clicked()
{
  hide();
}

void LimbsWeightForm::on_limbComboBox_currentIndexChanged(QString itemText)
{
  selectedLimb = anim->getNodeByName(itemText);
  ui->weightSlider->setValue(selectedLimb->frameData(frame).weight());
}

void LimbsWeightForm::on_weightSlider_valueChanged(int value)
{
  selectedLimb->setKeyframeWeight(frame, value);
  labels.value(selectedLimb->name())->setText(QString::number(selectedLimb->frameData(frame).weight()));
  emit valueChanged();
}
