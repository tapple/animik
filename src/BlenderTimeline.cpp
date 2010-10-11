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
  _firstItem = 0;
  _lastItem = 0;

  framesCount = MIN_TRACK_FRAMES;
  setNumberOfFrames(framesCount);
}

bool BlenderTimeline::AddAnimation(Animation* anim, QString title)
{
  //TODO

  TimelineItem* toBePredecessor;
  try {
    toBePredecessor = FindFreeSpace(0, anim->getNumberOfFrames()/*, toBePredecessor*/);
  }
  catch(QString) { return false; }
  
  TimelineItem* newItem;
  if(toBePredecessor == 0)                  //will become new first item
  {
    newItem = new TimelineItem(anim, title, 0);
    newItem->setNextItem(_firstItem);
    if(_firstItem)
      _firstItem->setPreviousItem(newItem);
    else
      _lastItem = newItem;
    newItem->setPreviousItem(0);
    _firstItem = newItem;
  }
  else if(toBePredecessor == _lastItem)     //wil become last item
  {
    newItem = new TimelineItem(anim, title, toBePredecessor->endIndex()+1);
    newItem->setNextItem(0);
    newItem->setPreviousItem(toBePredecessor);
    toBePredecessor->setNextItem(newItem);
    _lastItem = newItem;
  }
  else
  {
    newItem = new TimelineItem(anim, title, toBePredecessor->endIndex()+1);
    newItem->setNextItem(toBePredecessor->nextItem());
    newItem->setPreviousItem(toBePredecessor);
    toBePredecessor->setNextItem(newItem);
    newItem->nextItem()->setPreviousItem(newItem);
  }

  updateTimelineItemsIndices();
  return true;
}


/** Passes thorugh TimelineItems and updates their begin/end positions.
    Called usually after addition of new item */
void BlenderTimeline::updateTimelineItemsIndices()      //edu: actually only begin position needs to be adjusted
{
  if(!_firstItem)
    return;

  TimelineItem* currentItem = _firstItem;
  while(currentItem->nextItem() && currentItem->endIndex()>currentItem->nextItem()->beginIndex())
  {
    int diff = currentItem->endIndex() - currentItem->nextItem()->beginIndex() + 1;
    currentItem->nextItem()->shiftBeginIndex(diff);
    currentItem = currentItem->nextItem();
  }
}


TimelineItem* BlenderTimeline::FindFreeSpace(int track, int frames)
{
  //TODO: must be track dependent of course
  if(_firstItem == 0 && frames <= framesCount)
    return 0;
  if(_firstItem->beginIndex() >= frames)      //is there a space before first?
    return 0;

  TimelineItem* currentItem = _firstItem;

  while(currentItem->nextItem() != 0)
  {
    if((currentItem->nextItem()->beginIndex() - currentItem->endIndex() - 1) >= frames)
      return currentItem;
    currentItem = currentItem->nextItem();
  }

  //check space after last item
  if((framesCount - _lastItem->endIndex()) >= frames)
    return _lastItem;

  throw "Not enough space";
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
