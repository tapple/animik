#include "framesweightdialog.h"
#include "ui_framesweightdialog.h"
#include <QPainter>
#include <QPixmap>
#include "TrailItem.cpp"

#define TRACK_HEIGHT        100
#define FRAME_BAR_WIDTH     10


FramesWeightDialog::FramesWeightDialog(TrailItem* animItem, QWidget *parent) :
  QDialog(parent), ui(new Ui::FramesWeightDialog)
{
  ui->setupUi(this);
  offscreen = NULL;

  setWindowTitle(animItem->name() + " - frame weights");
  _anim = animItem->getAnimation();
  frames_count = _anim->getNumberOfFrames();
  frameWeights = new int[frames_count];
  for(int i=0; i<frames_count; i++)
    frameWeights[i] = _anim->getFrameWeight(i);

  offscreen = new QPixmap(frames_count*FRAME_BAR_WIDTH, TRACK_HEIGHT);
}

FramesWeightDialog::~FramesWeightDialog()
{
  delete ui;
}


void FramesWeightDialog::paintEvent(QPaintEvent *)
{
  drawWeights();

  QPainter p(this);
  p.drawPixmap(0, 0, *offscreen);
}

void FramesWeightDialog::drawWeights()
{
  if(!offscreen) return;

  QPainter p(offscreen);
  QColor boxColor = QColor("#5c4179");

  p.fillRect(0, 0, frames_count*FRAME_BAR_WIDTH, TRACK_HEIGHT, boxColor);

  //emphasize the first frame if it's T-pose
  if(_anim->isFirstFrameTPose())
    p.fillRect(0, 0, FRAME_BAR_WIDTH, TRACK_HEIGHT, QColor("#84365D"));

  double hFactor = ((double)TRACK_HEIGHT) / 100.0;
  QColor barColor("#999999");

  for(int i=0; i<frames_count; i++)
  {
    int barHeight = (int)(_anim->getFrameWeight(i)*hFactor);
/*TODO    if(i==currentPosition)
    {
      p.fillRect(i*_positionWidth+2, TOP_MARGIN+BORDER_WIDTH+(FRAME_HEIGHT-BORDER_WIDTH)-barHeight,
                 _positionWidth-2, barHeight, QColor("#0080ff"));
    }
    else    */
    {
      p.fillRect(i*FRAME_BAR_WIDTH +1, TRACK_HEIGHT-barHeight, FRAME_BAR_WIDTH-2, barHeight, barColor);
    }
  }
}


void FramesWeightDialog::on_setWeightPushButton_clicked()
{
  int value = ui->commonWeightSpinBox->value();
  for(int i=0; i<frames_count; i++)
    frameWeights[i] = value;

  repaint();
}
