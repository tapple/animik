/*
  The timeline widget inside BlenderTab. Includes 3 independent tracks.
*/


#define BLENDING_TRACKS      1    //edu: 1 track so far
#define MIN_TRACK_FRAMES     150  //edu: 150 frames long

#define TRACK_HEIGHT         40   //edu: 40px high
#define MIN_FRAME_WIDTH      5    //each frame must be at least 5px wide

#include <QSize>
#include <QPainter>

#include "BlenderTimeline.h"



BlenderTimeline::BlenderTimeline(QWidget* parent, Qt::WindowFlags) : QFrame(parent)
{
  offscreen=0;

  framesCount = MIN_TRACK_FRAMES;
  setNumberOfFrames(framesCount);
}



bool BlenderTimeline::AddAnimation(Animation* anim)
{
  //TODO

  TimelineItem* toBePredecessor = 0;
  if(!FindFreeSpace(0, anim->getNumberOfFrames(), toBePredecessor))
    return false;

  TimelineItem* newItem = new TimelineItem(anim, toBePredecessor->endIndex());
  TimelineItem* nextItem = toBePredecessor->nextItem();
  toBePredecessor->setNextItem(newItem);
  newItem->setNextItem(nextItem);

  return true;
}


bool BlenderTimeline::FindFreeSpace(int track, int frames, TimelineItem* outPredecessor)
{
  //TODO: must be track dependent of course

  outPredecessor = 0;

  if(_firstItem == 0 && frames <= framesCount)
    return true;

  TimelineItem* currentItem = _firstItem;

  while(currentItem->nextItem() != 0)
  {
    if((currentItem->nextItem()->beginIndex() - currentItem->endIndex() - 1) >= frames)
    {
      outPredecessor = currentItem;
      return true;
    }
    currentItem = currentItem->nextItem();
  }

  //check space after last item
  if((framesCount - _lastItem->endIndex()) >= frames)
  {
    outPredecessor = _lastItem;
    return true;
  }

  return false;
}


void BlenderTimeline::setNumberOfFrames(int frames)
{
  qDebug("BlenderTimeline::setNumberOfFrames(%d==%d): %0lx", framesCount, frames, (unsigned long) offscreen);

//TODO  framesCount=frames;

  QPixmap* newOffscreen = new QPixmap(framesCount*MIN_FRAME_WIDTH/*TODO: actual*/, TRACK_HEIGHT*BLENDING_TRACKS);

  // copy old offscreen pixmap to new pixmap, if there is one
  if(offscreen)
  {
    QPainter p(newOffscreen);
    p.drawPixmap(0,0,*offscreen);
    delete offscreen;
  }

  offscreen=newOffscreen;

//  fullRepaint=true;
  repaint();
}


void BlenderTimeline::drawBackground()
{
  //TODO

  QPainter p(this);
  p.drawPixmap(0,0,*offscreen);

  // draw current frame marker
  p.fillRect(50+MIN_FRAME_WIDTH, 2, 140, TRACK_HEIGHT, QColor("#ff0077"));
}

void BlenderTimeline::drawItem(TimelineItem* item)
{
  //TODO
}

void BlenderTimeline::paintEvent(QPaintEvent* event)
{
  QSize newSize = QSize(framesCount*MIN_FRAME_WIDTH/*TODO: actual*/, TRACK_HEIGHT*BLENDING_TRACKS);

  if(newSize!=size())
  {
    resize(newSize);
    emit resized(newSize);
  }

  drawBackground();     //and so on...
}
