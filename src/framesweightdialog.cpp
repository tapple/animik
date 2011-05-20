#include "framesweightdialog.h"
#include "ui_framesweightdialog.h"

#include "FramesWeightFrame.h"
#include "NoArrowsScrollArea.h"
#include <QKeyEvent>
#include "TrailItem.cpp"


FramesWeightDialog::FramesWeightDialog(TrailItem* animItem, QWidget *parent) :
  QDialog(parent), ui(new Ui::FramesWeightDialog)
{
  ui->setupUi(this);
  setWindowTitle(animItem->name() + " - frame weights");

  scrollArea = new NoArrowsScrollArea(ui->leftFrame);
  scrollArea->setBackgroundRole(QPalette::Dark);
  scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  stackWidget = new QWidget(scrollArea);

  QVBoxLayout* scrollLayout = new QVBoxLayout(stackWidget);
  scrollLayout->setMargin(0);

  fwf = new FramesWeightFrame(animItem, stackWidget);
  //TODO: connect key press signals
  scrollLayout->addWidget(fwf);

  setFocusPolicy(Qt::WheelFocus);         //I need this if I want to handle key press event
  stackWidget->setFocusPolicy(Qt::WheelFocus);
  scrollArea->setFocusPolicy(Qt::WheelFocus);
  stackWidget->setLayout(scrollLayout);
  scrollArea->setWidget(stackWidget);
  stackWidget->resize(fwf->FrameBarWidth * fwf->frameCount(), fwf->TrackHeight + fwf->ScrollBarOffset);

  needsReshape=true;
}

FramesWeightDialog::~FramesWeightDialog()
{
  delete ui;
}

void FramesWeightDialog::keyPressEvent(QKeyEvent* keyEvent)
{
  if((keyEvent->key()==Qt::Key_Enter || keyEvent->key()==Qt::Key_Return) && ui->commonWeightSpinBox->hasFocus())
    on_setWeightPushButton_clicked();
  else
    keyEvent->ignore();
}

void FramesWeightDialog::paintEvent(QPaintEvent*)
{
  if(needsReshape)
  {
    int wdth = fwf->FrameBarWidth * fwf->frameCount();
    if(wdth > ui->leftFrame->width())
      wdth = ui->leftFrame->width();
    int hght = fwf->TrackHeight + fwf->ScrollBarOffset;
    scrollArea->resize(wdth+4, hght);

    needsReshape=false;
  }
}

void FramesWeightDialog::resizeEvent(QResizeEvent *)
{
  needsReshape=true;
}


void FramesWeightDialog::on_setWeightPushButton_clicked()
{
  int value = ui->commonWeightSpinBox->value();
  fwf->setCommonWeight(value);
}

void FramesWeightDialog::on_buttonBox_accepted()
{
  if(fwf->isDirty())
  {
    setResult(QDialog::Accepted);
    fwf->saveWeightsToAnimation();
  }
  else
    setResult(QDialog::Rejected);
}
