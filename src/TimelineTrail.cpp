#include <QSize>
#include <QPainter>
#include "TimelineTrail.h"
#include "TrailItem.cpp"

#define TRACK_HEIGHT         40   //edu: 40px high
#define MIN_FRAME_WIDTH      5    //each frame must be at least 5px wide



TimelineTrail::TimelineTrail(QWidget* parent, Qt::WindowFlags) : QFrame(parent)
{
  offscreen=0;
  _firstItem = 0;
  _lastItem = 0;
}


bool TimelineTrail::AddAnimation(Animation* anim, QString title)
{
  //TODO

  TrailItem* toBePredecessor;
  try {
    toBePredecessor = FindFreeSpace(anim->getNumberOfFrames()/*, toBePredecessor*/);
  }
  catch(QString) { return false; }

  TrailItem* newItem;
  if(toBePredecessor == 0)                  //will become new first item
  {
    newItem = new TrailItem(anim, title, 0);
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
    newItem = new TrailItem(anim, title, toBePredecessor->endIndex()+1);
    newItem->setNextItem(0);
    newItem->setPreviousItem(toBePredecessor);
    toBePredecessor->setNextItem(newItem);
    _lastItem = newItem;
  }
  else
  {
    newItem = new TrailItem(anim, title, toBePredecessor->endIndex()+1);
    newItem->setNextItem(toBePredecessor->nextItem());
    newItem->setPreviousItem(toBePredecessor);
    toBePredecessor->setNextItem(newItem);
    newItem->nextItem()->setPreviousItem(newItem);
  }

  updateTimelineItemsIndices();
  return true;
}



TrailItem* TimelineTrail::FindFreeSpace(int frames)
{
  if(_firstItem == 0 && frames <= framesCount)
    return 0;
  if(_firstItem->beginIndex() >= frames)      //is there a space before first?
    return 0;

  TrailItem* currentItem = _firstItem;

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


void TimelineTrail::updateTimelineItemsIndices()      //edu: actually only begin position needs to be adjusted
{
  if(!_firstItem)
    return;

  TrailItem* currentItem = _firstItem;
  while(currentItem->nextItem() && currentItem->endIndex()>currentItem->nextItem()->beginIndex())
  {
    int diff = currentItem->endIndex() - currentItem->nextItem()->beginIndex() + 1;
    currentItem->nextItem()->shiftBeginIndex(diff);
    currentItem = currentItem->nextItem();
  }
}

void TimelineTrail::setNumberOfFrames(int frames)
{
  qDebug("TimelineTrail::setNumberOfFrames(%d==%d): %0lx", framesCount, frames, (unsigned long) offscreen);

  framesCount=frames;
  QPixmap* newOffscreen = new QPixmap(framesCount*MIN_FRAME_WIDTH/*TODO: actual*/, TRACK_HEIGHT);

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


void TimelineTrail::setCurrentFrame(int frame)
{
  frameSelected=frame;
  emit positionCenter(frameSelected*MIN_FRAME_WIDTH/*TODO: actual*/);
}


void TimelineTrail::paintEvent(QPaintEvent* event)
{
  QSize newSize = QSize(framesCount*MIN_FRAME_WIDTH/*TODO: actual*/, TRACK_HEIGHT);

  if(newSize!=size())
  {
    resize(newSize);
    emit resized(newSize);
  }

  drawBackground();     //and so on...
}


void TimelineTrail::drawBackground()
{
  //TODO

  QPainter p(this);
  p.drawPixmap(0, 0, *offscreen);

  // draw current frame marker
  p.fillRect(50+MIN_FRAME_WIDTH, 2, 140, TRACK_HEIGHT, QColor("#ff0077"));
}

void TimelineTrail::drawTrailItem(TrailItem* item)
{
  //TODO
}
