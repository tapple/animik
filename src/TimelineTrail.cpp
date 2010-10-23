#include <QAction>
#include <QCursor>
#include <QContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QPalette>
#include <QSize>

#include <QMessageBox>      //DEBUG purposes

#include "TimelineTrail.h"
#include "TrailItem.cpp"

#define TRACK_HEIGHT     72
#define MIN_FRAME_WIDTH  8    //minimum frame space width that can't be crossed when zooming
#define MAX_FRAME_WIDTH  20   //TODO: maximum when zooming
#define ZOOM_STEP        4    //TODO: the zooming
#define FRAME_WIDTH      8    //TODO: no place for such constant
#define MAX_TRAIL_FRAMES 4000



TimelineTrail::TimelineTrail(QWidget* parent, Qt::WindowFlags) : QFrame(parent)
{
  offscreen=0;
  currentFrame=0;
  _firstItem = 0;
  _lastItem = 0;
  draggingItem = 0;
  draggingOverFrame = -1;
  leftMouseDown = rightMouseDown = false;
  //we have to receive mouseMoveEvent even when no button down
  setMouseTracking(true);

  //Actions that may appear in context menu
  deleteItemAction = new QAction(tr("Delete animation"), this);
  connect(deleteItemAction, SIGNAL(triggered()), this, SLOT(deleteCurrentItem()));
  moveItemAction = new QAction(tr("Move on timeline"), this);
  connect(moveItemAction, SIGNAL(triggered()), this, SLOT(moveCurrentItem()));
  limbWeightsAction = new QAction(tr("Set limbs' weights"), this);
  connect(limbWeightsAction, SIGNAL(triggered()), this, SLOT(showLimbsWeight()));
}


bool TimelineTrail::AddAnimation(Animation* anim, QString title)
{
  TrailItem* toBePredecessor;
  try {
    toBePredecessor = FindFreeSpace(anim->getNumberOfFrames());
  }
  catch(QString* message)
  {/* Not enough space on this trail. Probably we can't do more here. */
    if(*message == "Not enough space")
      return false;
  }

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
  else if(toBePredecessor == _lastItem)     //will become last item
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

  //try to extend this trail to ensure place at the end
  int remaining = _lastItem ? framesCount-_lastItem->endIndex()-1
                            : framesCount;
  if(coerceExtension(frames-remaining))
    return _lastItem;

  throw new QString("Not enough space");
}


bool TimelineTrail::coerceExtension(int size)
{
  if (framesCount+size <= MAX_TRAIL_FRAMES)
  {
    setFrameCount(framesCount+size);
    emit framesCountChanged(framesCount);
    return true;
  }
  else
    return false;
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

void TimelineTrail::setFrameCount(int frames)
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

void TimelineTrail::drawMovedItemShadow()
{
//done elsewhere  if(draggingItem && draggingOverFrame)
  QPainter* p = new QPainter(offscreen);
  p->fillRect(draggingOverFrame*FRAME_WIDTH, 3,
              draggingItem->frames()*FRAME_WIDTH, TRACK_HEIGHT-6, QColor("#d7df01"));
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
  if(currentFrame>=begin && currentFrame<=item->endIndex())
    p.fillRect(currentFrame*FRAME_WIDTH, 3, FRAME_WIDTH, TRACK_HEIGHT-6, selFrameColor);
}


TrailItem* TimelineTrail::findItemOnFrame(int frameIndex)
{
  TrailItem* currentItem = _firstItem;

  while(currentItem != 0)
  {
    int begin = currentItem->beginIndex();
    if(frameIndex>=begin && frameIndex<=currentItem->endIndex())
      break;
    currentItem = currentItem->nextItem();
  }
  return currentItem;
}

void TimelineTrail::paintEvent(QPaintEvent*)
{
  QSize newSize = QSize(framesCount*FRAME_WIDTH/*TODO: actual*/, TRACK_HEIGHT);

  if(newSize!=size())
  {
    resize(newSize);
    emit resized(newSize);
  }

  drawBackground();
  if(draggingOverFrame >= 0)
    drawMovedItemShadow();

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
  if(rightMouseDown && selectedItem)
  {
    QMenu menu(this);
    menu.addAction(deleteItemAction);
    menu.addAction(moveItemAction);
    menu.addSeparator();
    menu.addAction(limbWeightsAction);
    menu.exec(event->globalPos());
  }
}


void TimelineTrail::mouseMoveEvent(QMouseEvent* me)
{
  if(draggingItem)
  {
    float hitFrame = ((float)me->pos().x()) / FRAME_WIDTH;
    int frame = (int)hitFrame;
    if(frame != draggingOverFrame)
    {
      draggingOverFrame = frame;
      repaint();
    }
  }
}

void TimelineTrail::leaveEvent(QEvent *)
{
  //user dragging an TrailItem left this area
  draggingOverFrame = -1;
  repaint();
}

void TimelineTrail::mousePressEvent(QMouseEvent* e)
{
  leftMouseDown = rightMouseDown = false;
  int clickedFrame = e->x()/FRAME_WIDTH;

  if(draggingItem)
  {
    //TODO: place it here, make it selected (if from different trail)

    cleanupAfterMove();
    emit droppedItem();
    return;
  }
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

  if(e->button()==Qt::LeftButton)
    leftMouseDown = true;
  else if(e->button()==Qt::RightButton)
    rightMouseDown = true;

  repaint();
}


int TimelineTrail::frameWidth()
{
  return FRAME_WIDTH/*TODO: actual width*/;
}

// ---------------------- SLOTS ---------------------- //
void TimelineTrail::deleteCurrentItem()
{
  TrailItem* current = selectedItem;

  if(current==_firstItem && current==_lastItem)     //was only one on this trail
  {
    _firstItem = _lastItem = 0;
  }
  else if(current==_firstItem)                      //was first on trail
  {
    _firstItem = current->nextItem();
    current->nextItem()->setPreviousItem(0);
  }
  else if(current==_lastItem)                        //last on trail
  {
    _lastItem = current->previousItem();
    current->previousItem()->setNextItem(0);
  }
  else    //somewhere in middle (current->previousItem() && current->nextItem())
  {
    current->previousItem()->setNextItem(current->nextItem());
    current->nextItem()->setPreviousItem(current->previousItem());
  }

  delete current;
  selectedItem = 0;
  repaint();
}

void TimelineTrail::moveCurrentItem()
{
  //All in here is DEBUG
//  QMessageBox::warning(this, "Size of this timeline is:", QString("%1 x %2").arg(size().height()).arg(size().width()));
  QPoint corner(selectedItem->beginIndex()*FRAME_WIDTH, 3);
  QCursor::setPos(mapToGlobal(corner));
  emit movingItem(selectedItem);
}

void TimelineTrail::showLimbsWeight()
{
  //reemit to containing widget to handle it own way
  emit adjustLimbsWeight(/*TODO: frameData*/);
}

void TimelineTrail::onMovingItem(TrailItem* draggedItem)
{
  QCursor movCur(Qt::SizeAllCursor);
  setCursor(movCur);
  draggingItem=draggedItem;
}

void TimelineTrail::onDroppedItem()
{
  //TODO: Delete it if comming from here.

  cleanupAfterMove();
  repaint();
}

void TimelineTrail::cleanupAfterMove()
{
  draggingItem = 0;
  draggingOverFrame = -1;
  QCursor defCur;
  setCursor(defCur);
}
