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
  currentFrame=14;   //realy?
  _firstItem = 0;
  _lastItem = 0;
  leftMouseDown = rightMouseDown = false;

  //Actions that may appear in context menu
  deleteAnimationAction = new QAction(tr("Delete animation"), this);
  connect(deleteAnimationAction, SIGNAL(triggered()), this, SLOT(deleteCurrentAnimation()));
  moveAnimationAction = new QAction(tr("Move on timeline"), this);
  connect(moveAnimationAction, SIGNAL(triggered()), this, SLOT(moveCurrentAnimation()));
  limbWeightsAction = new QAction(tr("Set limbs' weights"), this);
  connect(limbWeightsAction, SIGNAL(triggered()), this, SLOT(showLimbsWeight()));
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
  if(frame != currentFrame)
  {
    currentFrame=frame;
    emit positionCenter(currentFrame*FRAME_WIDTH/*TODO: actual*/);
    repaint();
  }
}


void TimelineTrail::drawBackground()
{
  //TODO
  if(!offscreen) return;

  QPainter* p = new QPainter(/*DEBUG this*/ offscreen);

  p->fillRect(0, 0, framesCount*FRAME_WIDTH/*TODO: actuall width*/, TRACK_HEIGHT, QColor("#009900"));
  // draw current frame marker
  p->fillRect(currentFrame*FRAME_WIDTH, 0, FRAME_WIDTH, TRACK_HEIGHT, QColor("#df7401"));

  for(int i=0; i<framesCount; i++)
  {
    QPoint upPoint(i*FRAME_WIDTH, 0);
    QPoint downPoint(i*FRAME_WIDTH, TRACK_HEIGHT);
    if(i==currentFrame)
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

  QColor boxColor;
  QColor selFrameColor;
  if(item==selectedItem)
  {
    boxColor = QColor("#ff0044");
    selFrameColor = QColor("#cb045f");
  }
  else
  {
    boxColor = QColor("#ff0077");
    selFrameColor = QColor("#b4045f");
  }
  p.fillRect(item->beginIndex()*FRAME_WIDTH, 3, item->frames()*FRAME_WIDTH, TRACK_HEIGHT-6, boxColor);
  int begin = item->beginIndex();
  //if selected frame falls inside this animation, highlight respective frame
  if(currentFrame>=begin && currentFrame<=(begin+item->frames()))
    p.fillRect(currentFrame*FRAME_WIDTH, 3, FRAME_WIDTH, TRACK_HEIGHT-6, selFrameColor);
}


TrailItem* TimelineTrail::findItemOnFrame(int frameIndex)
{
  TrailItem* currentItem = _firstItem;

  while(currentItem != 0)
  {
    int begin = currentItem->beginIndex();
    if(frameIndex>=begin && frameIndex<begin+currentItem->frames())
      break;
    currentItem = currentItem->nextItem();
  }
  return currentItem;
}

void TimelineTrail::paintEvent(QPaintEvent* event)
{
  QSize newSize = QSize(framesCount*FRAME_WIDTH/*TODO: actual*/, TRACK_HEIGHT);

  if(newSize!=size())
  {
    resize(newSize);
    emit resized(newSize);
  }

  drawBackground();

  TrailItem* currentItem = _firstItem;
  while(currentItem != 0)
  {
    drawTrailItem(currentItem);
    currentItem = currentItem->nextItem();
  }

  QPainter p(this);
  p.drawPixmap(0, 0, *offscreen);
}


void TimelineTrail::contextMenuEvent(QContextMenuEvent *event)
{
  if(leftMouseDown && selectedItem)
  {
    QMenu menu(this);
    menu.addAction(limbWeightsAction);
    menu.exec(event->globalPos());
  }
}

void TimelineTrail::mousePressEvent(QMouseEvent* e)
{
  int clickedFrame = e->x()/FRAME_WIDTH;
  if(clickedFrame != currentFrame)
  {
    currentFrame = clickedFrame;
    emit currentFrameChanged(currentFrame);
  }
  TrailItem* clickedItem = findItemOnFrame(currentFrame);
  if(clickedItem != selectedItem)
  {
    selectedItem = clickedItem;
    emit selectedItemChanged();
  }

//TODO: apply to resulting animation  animation->setFrame(currentFrame);

  leftMouseDown = rightMouseDown = false;
  if(e->button()==Qt::LeftButton)
    leftMouseDown = true;
  else if(e->button()==Qt::RightButton)
    rightMouseDown = true;

  repaint();
}


// ---------------------- SLOTS ---------------------- //
void TimelineTrail::deleteCurrentAnimation()
{

}

void TimelineTrail::moveCurrentAnimation()
{

}

void TimelineTrail::showLimbsWeight()
{
  //reemit to containing widget to handle it own way
  emit adjustLimbsWeight(/*TODO: frameData*/);
}
