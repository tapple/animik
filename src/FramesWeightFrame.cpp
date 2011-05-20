#include "FramesWeightFrame.h"
#include "ui_FramesWeightFrame.h"

#include <QPainter>
#include <QPixmap>
#include "TrailItem.cpp"



FramesWeightFrame::FramesWeightFrame(TrailItem* animItem, QWidget *parent) : QFrame(parent), ui(new Ui::FramesWeightFrame)
{
  ui->setupUi(this);

  dirty = false;
  _anim = animItem->getAnimation();
  frames_count = _anim->getNumberOfFrames();
  frameWeights = new int[frames_count];
  for(int i=0; i<frames_count; i++)
    frameWeights[i] = _anim->getFrameWeight(i);

  offscreen = new QPixmap(frames_count*FrameBarWidth/*ui->leftFrame->width()*/, TrackHeight);
}

FramesWeightFrame::~FramesWeightFrame()
{
  delete ui;
}


int FramesWeightFrame::frameCount() const
{
  return _anim->getNumberOfFrames();
}

void FramesWeightFrame::saveWeightsToAnimation()
{
  for(int i=0; i<frameCount(); i++)
    _anim->setFrameWeight(i, frameWeights[i]);
}

const int* FramesWeightFrame::getFrameWeights() const
{
  return frameWeights;
}

void FramesWeightFrame::setCommonWeight(int value)
{
  for(int i=0; i<frames_count; i++)
    frameWeights[i] = value;
  dirty = true;
  repaint();
}


void FramesWeightFrame::paintEvent(QPaintEvent *)
{
  QSize newSize(frames_count*FrameBarWidth, TrackHeight);
  if(newSize != size())
    resize(newSize);

  int dw = width();
  int dh = height();
  int dvain = dw*dh;


  if(dvain) drawWeights();

  QPainter p(this);
  p.drawPixmap(0, 0, *offscreen);
}


void FramesWeightFrame::resizeEvent(QResizeEvent *)           //TODO: this should be obsolete
{
  QPixmap* newOffscreen = new QPixmap(frames_count*FrameBarWidth, TrackHeight);

  int dw = width();
  int dh = height();
  int dvain = dw*dh;

  // copy old offscreen pixmap to new pixmap, if there is one
  if(offscreen && dvain!=0)
  {
    QPainter p(newOffscreen);
    p.drawPixmap(0,0,*offscreen);
    delete offscreen;
  }

  offscreen=newOffscreen;
}

void FramesWeightFrame::drawWeights()
{
  if(!offscreen) return;

  QPainter p(offscreen);
  QColor boxColor = QColor("#5c4179");

  p.fillRect(0, 0, frames_count*FrameBarWidth, TrackHeight, boxColor);

  //emphasize the first frame if it's T-pose
  if(_anim->isFirstFrameTPose())
    p.fillRect(0, 0, FrameBarWidth, TrackHeight, QColor("#84365D"));

  double hFactor = ((double)TrackHeight) / 100.0;
  QColor barColor("#999999");

  for(int i=0; i<frames_count; i++)
  {
    int barHeight = (int)(frameWeights[i] * hFactor);
/*TODO    if(i==currentPosition)
    {
      p.fillRect(i*_positionWidth+2, TOP_MARGIN+BORDER_WIDTH+(FRAME_HEIGHT-BORDER_WIDTH)-barHeight,
                 _positionWidth-2, barHeight, QColor("#0080ff"));
    }
    else    */
    {
      p.fillRect(i*FrameBarWidth +1, TrackHeight-barHeight, FrameBarWidth-2, barHeight, barColor);
    }
  }
}
