#include <QAction>
#include <QContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QPalette>
#include <QSize>

#include "TimelineTrail.h"
#include "TrailItem.cpp"

#define TRACK_HEIGHT     72
#define MIN_FRAME_WIDTH  8    //minimum frame space width that can't be crossed when zooming
#define MAX_FRAME_WIDTH  20   //TODO: maximum when zooming
#define ZOOM_STEP        4    //TODO: the zooming
#define FRAME_WIDTH      8    //TODO: no place for such constant



TimelineTrail::TimelineTrail(QWidget* parent, Qt::WindowFlags) : QFrame(parent)
{
  offscreen=0;
  frameSelected=14;   //realy?
  _firstItem = 0;
  _lastItem = 0;

  //Actions that may appear in context menu
  limbWeightsAction = new QAction(tr("Set limbs' weights"), this);
  connect(limbWeightsAction, SIGNAL(triggered()), this, SLOT(showLimbsWeight()));
  //TODO: and the other actions
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
  repaint();
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
  QPixmap* newOffscreen = new QPixmap(framesCount*FRAME_WIDTH/*TODO: actual*/, TRACK_HEIGHT);

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
  emit positionCenter(frameSelected*FRAME_WIDTH/*TODO: actual*/);
}


void TimelineTrail::paintEvent(QPaintEvent* event)
{
  QSize newSize = QSize(framesCount*FRAME_WIDTH/*TODO: actual*/, TRACK_HEIGHT);

  if(newSize!=size())
  {
    resize(newSize);
    emit resized(newSize);
  }

  drawBackground();     //and so on...

  TrailItem* testItem = new TrailItem(0, "Test animation", 11);
  drawTrailItem(testItem);

  QPainter p(this);
  p.drawPixmap(0, 0, *offscreen);
}


void TimelineTrail::contextMenuEvent(QContextMenuEvent *event)
{
  QMenu menu(this);
  menu.addAction(limbWeightsAction);
  menu.exec(event->globalPos());
}


void TimelineTrail::drawBackground()
{
  //TODO
  if(!offscreen) return;

  QPainter* p = new QPainter(/*DEBUG this*/ offscreen);

  p->fillRect(0, 0, framesCount*FRAME_WIDTH/*TODO: actuall width*/, TRACK_HEIGHT, QColor("#009900"));
  // draw current frame marker
  p->fillRect(frameSelected*FRAME_WIDTH, 0, FRAME_WIDTH, TRACK_HEIGHT, QColor("#df7401"));

  for(int i=0; i<framesCount; i++)
  {
    QPoint upPoint(i*FRAME_WIDTH, 0);
    QPoint downPoint(i*FRAME_WIDTH, TRACK_HEIGHT);
    if(i==frameSelected)
      p->setPen(QColor("#ffffff"));
    else
      p->setPen(QColor("#cccccc"));
    p->drawLine(upPoint, downPoint);
  }

  delete p;
}

void TimelineTrail::drawTrailItem(TrailItem* item)
{
  if(!offscreen) return;

  QPainter p(/*DEBUG this*/ offscreen);

  p.fillRect(item->beginIndex()*FRAME_WIDTH, 3, /*DEBUG item->frames()*/30*FRAME_WIDTH, TRACK_HEIGHT-6, QColor("#ff0077"));
  int begin = item->beginIndex();
  //if selected frame falls inside this animation, highlight respective frame
  if(frameSelected>=begin && frameSelected<=(begin+/*item->frames()*/30))
    p.fillRect(frameSelected*FRAME_WIDTH, 3, FRAME_WIDTH, TRACK_HEIGHT-6, QColor("#b4045f"));
}


void TimelineTrail::showLimbsWeight()
{
  //reemit to containing widget to handle it with custom widget
  emit adjustLimbsWeight(/*TODO: frameData*/);
}
