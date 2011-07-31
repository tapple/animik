#include "FramesWeightFrame.h"
#include "ui_FramesWeightFrame.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include "trailitem.cpp"

FramesWeightFrame::FramesWeightFrame(TrailItem* animItem, QWidget *parent) : QFrame(parent), ui(new Ui::FramesWeightFrame)
{
  ui->setupUi(this);

  dirty = false;
  mouseDown = false;
  weightedFrame = -1;
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

void FramesWeightFrame::mouseMoveEvent(QMouseEvent* mouseEvent)
{
  if(mouseDown)
  {
    weightedFrame = mouseEvent->x()/FrameBarWidth;
    if(weightedFrame<0 || weightedFrame>=frameCount())
      return;
    adjustWeight(mouseEvent->y());
    repaint();
  }
}

void FramesWeightFrame::mousePressEvent(QMouseEvent* mouseEvent)
{
  mouseDown = true;
  weightedFrame = mouseEvent->x()/FrameBarWidth;
  adjustWeight(mouseEvent->y());
  repaint();
}

void FramesWeightFrame::mouseReleaseEvent(QMouseEvent *)
{
  mouseDown = false;
  weightedFrame = -1;
  repaint();
}

void FramesWeightFrame::paintEvent(QPaintEvent *)
{
  QSize newSize(frames_count*FrameBarWidth, TrackHeight);
  if(newSize != size())
    resize(newSize);

  drawWeights();

  QPainter p(this);
  p.drawPixmap(0, 0, *offscreen);
}


void FramesWeightFrame::resizeEvent(QResizeEvent *)           //TODO: this should be obsolete
{
  QPixmap* newOffscreen = new QPixmap(frames_count*FrameBarWidth, TrackHeight);

  // copy old offscreen pixmap to new pixmap, if there is one
  if(offscreen)
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
    if(i==weightedFrame)
      p.fillRect(i*FrameBarWidth +1, TrackHeight-barHeight, FrameBarWidth-2, barHeight, QColor("#0080ff"));
    else
      p.fillRect(i*FrameBarWidth +1, TrackHeight-barHeight, FrameBarWidth-2, barHeight, barColor);

    if(weightedFrame!=-1)
    {
      int w = frameWeights[weightedFrame];
      QPen textPen(QColor("#f8f8f8"));
      QFont f ("Arial", 11, QFont::Normal);
      p.setPen(textPen);
      p.setFont(f);

     //for some galactic reason the center alignment doesn't work as expected, so must be done manually
      if(w==100)
        p.drawText(weightedFrame*FrameBarWidth-FrameBarWidth, TrackHeight-TextSize,
                   FrameBarWidth*3, TextSize, Qt::AlignJustify, QString::number(w));
      else if(w<10)
        p.drawText(weightedFrame*FrameBarWidth, TrackHeight-TextSize,
                   FrameBarWidth, TextSize, Qt::AlignJustify, QString::number(w));
      else    //10-99
        p.drawText(weightedFrame*FrameBarWidth-(FrameBarWidth/2), TrackHeight-TextSize,
                   FrameBarWidth*2, TextSize, Qt::AlignJustify, QString::number(w));
    }
  }
}

void FramesWeightFrame::adjustWeight(int y)
{
  if(weightedFrame<0 || weightedFrame>=frameCount())
    return;
  if(y>TrackHeight)           //When under track, mimic zero weight
    y=TrackHeight;
  if(y<0)                     //Above. It's possible when mouse moves across desktop with primary button down
    y=0;

  float weight = 100.0 * (float)y / TrackHeight;
  int oldWeight = _anim->getFrameWeight(weightedFrame);
  int newWeight = 100 - (int)weight;
  if(oldWeight != newWeight)
  {
    dirty=true;
    frameWeights[weightedFrame] = newWeight;
    repaint(weightedFrame*FrameBarWidth-10, 0, FrameBarWidth+20, TrackHeight);
  }
}
