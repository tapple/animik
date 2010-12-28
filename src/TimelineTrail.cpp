#include <QAction>
#include <QCursor>
#include <QContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QPalette>
#include <QSize>

#include "TimelineTrail.h"
#include "TrailItem.cpp"
#include "bvh.h"

#define TRACK_HEIGHT     72
#define FRAME_HEIGHT     TRACK_HEIGHT-6
#define TOP_MARGIN       2
#define BORDER_WIDTH     2
#define TEXT_HEIGHT      12
#define MIN_FRAME_WIDTH  8     //minimum frame space width that can't be crossed when zooming
#define MAX_FRAME_WIDTH  20    //TODO: maximum when zooming
#define ZOOM_STEP        4     //TODO: the zooming
#define MAX_TRAIL_FRAMES 400//DEBUG 4000
#define MAX_WEIGHT       100.0 //max frame weight



TimelineTrail::TimelineTrail(QWidget* parent, Qt::WindowFlags) : QFrame(parent)
{
  offscreen=0;
  currentFrame=0;
  _frameWidth = MIN_FRAME_WIDTH;
  _firstItem = 0;
  _lastItem = 0;
  draggingItem = 0;
  draggingOverFrame = -1;
  leftMouseDown = rightMouseDown = false;
  //we need to receive mouseMoveEvent even when no button down
  setMouseTracking(true);

  //Actions that may appear in context menu
  deleteItemAction = new QAction(tr("Delete animation"), this);
  connect(deleteItemAction, SIGNAL(triggered()), this, SLOT(deleteCurrentItem()));
  moveItemAction = new QAction(tr("Move on timeline"), this);
  connect(moveItemAction, SIGNAL(triggered()), this, SLOT(moveCurrentItem()));
  limbWeightsAction = new QAction(tr("Set limbs' weights"), this);
  connect(limbWeightsAction, SIGNAL(triggered()), this, SLOT(showLimbsWeight()));
}


bool TimelineTrail::AddAnimation(WeightedAnimation* anim, QString title)
{
  TrailItem* toBePredecessor;
  try {
    toBePredecessor = findFreeSpace(anim->getNumberOfFrames());
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
  trailContentChange();
  repaint();
  return true;
}



TrailItem* TimelineTrail::findFreeSpace(int frames)
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

  if(frames==framesCount)
    return;   //nothing to do

  framesCount=frames;
  QPixmap* newOffscreen = new QPixmap(framesCount*_frameWidth, TRACK_HEIGHT);

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
    emit positionCenter(currentFrame*_frameWidth);
    repaint();
  }
}


void TimelineTrail::drawBackground()
{
  if(!offscreen) return;

  QPainter* p = new QPainter(offscreen);

  p->fillRect(0, 0, framesCount*_frameWidth, TRACK_HEIGHT, QColor("#009900"));
  // draw current frame marker
  p->fillRect(currentFrame*_frameWidth, 0, _frameWidth, TRACK_HEIGHT, QColor("#df7401"));

  for(int i=0; i<framesCount; i++)
  {
    QPoint upPoint(i*_frameWidth, 0);
    QPoint downPoint(i*_frameWidth, TRACK_HEIGHT);
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
//if(draggingItem && draggingOverFrame){    no need to, done elsewhere
  QPainter* p = new QPainter(offscreen);
  p->fillRect(draggingOverFrame*_frameWidth, 0,
              draggingItem->frames()*_frameWidth, TRACK_HEIGHT-3, QColor("#d7df01"));
  delete p;
}

void TimelineTrail::drawTrailItem(TrailItem* item)
{
  if(!offscreen) return;
  if(item==draggingItem) return;

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
  p.fillRect(item->beginIndex()*_frameWidth+2, TOP_MARGIN+BORDER_WIDTH, item->frames()*_frameWidth-2, FRAME_HEIGHT, boxColor);
  QRectF border(item->beginIndex()*_frameWidth+1, TOP_MARGIN+1, item->frames()*_frameWidth, FRAME_HEIGHT);

  int begin = item->beginIndex();
  //if selected frame falls inside this animation, highlight the frame
  if(currentFrame>=begin && currentFrame<=item->endIndex())
  {
    p.fillRect(currentFrame*_frameWidth+1, TOP_MARGIN+BORDER_WIDTH, _frameWidth, FRAME_HEIGHT-BORDER_WIDTH, selFrameColor);
  }

  //border must go almost last for correct overlapping
  QPen borderPen(QColor("#000000"));
  borderPen.setWidth(BORDER_WIDTH);
  p.setPen(borderPen);
  p.drawRoundedRect(border, 4, 4);

  //show frame weights, as last
  if(item==selectedItem)
  {
    int end = item->endIndex();
    double hFactor = ((double)FRAME_HEIGHT-BORDER_WIDTH) / MAX_WEIGHT;
    QColor barColor("#999999");

    for(int i=begin; i<=end; i++)
    {
      int barHeight = (int)(item->getWeight(i-begin)*hFactor);
      if(i==currentFrame)
      {
        p.fillRect(i*_frameWidth+2, TOP_MARGIN+BORDER_WIDTH+(FRAME_HEIGHT-BORDER_WIDTH)-barHeight,
                   _frameWidth-2, barHeight, QColor("#0080ff"));
      }
      else
      {
        p.fillRect(i*_frameWidth+2, TOP_MARGIN+BORDER_WIDTH+(FRAME_HEIGHT-BORDER_WIDTH)-barHeight,
                   _frameWidth-2, barHeight, barColor);
      }
    }

    QPen textPen(QColor("#ffffff"));
    textPen.setWidth(2);
    p.setPen(textPen);
    p.drawText(currentFrame*_frameWidth-(TEXT_HEIGHT/4), TRACK_HEIGHT-TOP_MARGIN-BORDER_WIDTH-TEXT_HEIGHT,
               TEXT_HEIGHT*2, TEXT_HEIGHT, 0, QString::number(item->getWeight(currentFrame-begin)));
  }
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


bool TimelineTrail::isSuitableSpace(int beginFrame, int framesCount)
{
  int endFrame = beginFrame+framesCount-1;
  if(_firstItem == 0 /*&& _lastItem == 0*/ && endFrame <= (MAX_TRAIL_FRAMES-1))
    return true;

  TrailItem* currentItem = _firstItem;
  while(currentItem != 0)
  {
    if(currentItem->beginIndex()>endFrame)    //all items from here are too far, so safe
      return true;
    if((currentItem->beginIndex()>=beginFrame && currentItem->beginIndex()<=endFrame) ||
       (currentItem->endIndex()>=beginFrame && currentItem->endIndex()<=endFrame) ||
       (beginFrame>=currentItem->beginIndex() && beginFrame<=currentItem->endIndex()))
      return false;                           //overlaps with currentItem

    currentItem = currentItem->nextItem();
  }

  //it wants to be on the end, test (possible) space available
  return endFrame <= (MAX_TRAIL_FRAMES-1);
}

void TimelineTrail::cleanupAfterMove()
{
  draggingItem = 0;
  draggingOverFrame = -1;
  QCursor defCur;
  setCursor(defCur);
}


TrailItem* TimelineTrail::findPreviousItem(int beforeFrame)
{
  if(_firstItem==0/*==_lastItem*/)
    return 0;
  if(_firstItem->beginIndex() > beforeFrame)
    return 0;         //belongs to beginning
  if(_firstItem==_lastItem)
  {
    if(_firstItem->endIndex() < beforeFrame)
      return _firstItem;
    else
      return 0;
  }

  TrailItem* currentItem = _firstItem;
  while(currentItem != 0)
  {
    if(currentItem->nextItem()==0)
      return currentItem;
    if(currentItem->nextItem()->beginIndex() > beforeFrame)
      return currentItem;
    currentItem = currentItem->nextItem();
  }

  throw new QString("findNextItem: unable to return valid TrailItem object");
}


TrailItem* TimelineTrail::findNextItem(int afterFrame)
{
  if(_lastItem==0/*==_firstItem*/)
    return 0;
  if(_lastItem->endIndex() < afterFrame)
    return 0;           //belongs to very end
  if(_firstItem==_lastItem)
  {
    if(_lastItem->beginIndex() > afterFrame)
     return _lastItem;
    else
      return 0;
  }

  TrailItem* currentItem = _lastItem;
  while(currentItem != 0)
  {
    if(currentItem->previousItem()==0)
      return currentItem;
    if(currentItem->previousItem()->endIndex() < afterFrame)
      return currentItem;
    currentItem = currentItem->previousItem();
  }

  throw new QString("findNextItem: unable to return valid TrailItem object");
}



WeightedAnimation* TimelineTrail::getSummaryAnimation()
{
  if(_firstItem==0)           //no animations left at this trail
    return 0;
  if(_firstItem==_lastItem)   //single animation, no need for join
    return _firstItem->getAnimation();

  //TODO
  //DEBUG: so far, just concatenate them ignoring gaps or any other attributes
  int totalFrames = 0;
  TrailItem* currentItem = _firstItem;
  while(currentItem!=0)
  {
    totalFrames += currentItem->frames();
    currentItem = currentItem->nextItem();
  }

  WeightedAnimation* result = new WeightedAnimation(new BVH(), ""); //_firstItem->getAnimation();
  result->setNumberOfFrames(totalFrames);
  //TODO: copy frame weights here

  //copy rotations to the result
  BVHNode* root = _firstItem->getAnimation()->getMotion();
  enhanceResultAnimation(result, root);

  return result;
}


void TimelineTrail::enhanceResultAnimation(WeightedAnimation* destAnim, BVHNode* node)
{
  int limbIndex = destAnim/*_firstItem->getAnimation()*/->getPartIndex(node);
  appendNodeKeyFrames(destAnim, limbIndex);
  for(int i=0; i<node->numChildren(); i++)
    enhanceResultAnimation(destAnim, node->child(i));
}


void TimelineTrail::appendNodeKeyFrames(WeightedAnimation* destAnim, int nodeIndex)
{
  TrailItem* currentItem = _firstItem;//->nextItem();
  int frameOffset = 0;//_firstItem->frames();

  while(currentItem!=0)
  {
    BVHNode* limb = currentItem->getAnimation()->getNode(nodeIndex);
    QList<int> indices = limb->keyframeList();
    int count = indices.count();
    for(int i=0; i<count; i++)
    {
      FrameData oldData = limb->frameData(indices[i]);
      destAnim->getNode(nodeIndex)->addKeyframe(indices[i]+frameOffset, oldData.position(), oldData.rotation());
    }
    frameOffset += currentItem->frames();
    currentItem = currentItem->nextItem();
  }
}


void TimelineTrail::trailContentChange()
{
  WeightedAnimation* result = getSummaryAnimation();
  emit trailAnimationChanged(result, _firstItem ? _firstItem->beginIndex() : -1);
}


void TimelineTrail::paintEvent(QPaintEvent*)
{
  QSize newSize = QSize(framesCount*_frameWidth, TRACK_HEIGHT);

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
    float hitFrame = ((float)me->pos().x()) / _frameWidth;
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
  int clickedFrame = e->x()/_frameWidth;

  //user wishes to place dragged item
  if(draggingItem)
  {
    if(isSuitableSpace(clickedFrame, draggingItem->frames()))
    {
      int endFrame = clickedFrame + draggingItem->frames() -1;
      if(endFrame > framesCount-1)      //we need to extend trail
      {
        int inside = framesCount-clickedFrame;
        if(!coerceExtension(draggingItem->frames() - inside))
          return;       //this should never happen if 'isSuitableSpace' returned true
      }

      TrailItem* newPrevious = findPreviousItem(clickedFrame);
      TrailItem* newNext = findNextItem(endFrame);

      //set it to new location on this trail
      if(newPrevious!=0)
      {
        newPrevious->setNextItem(draggingItem);
        draggingItem->setPreviousItem(newPrevious);
      }
      else      //will become new _first here
      {
        draggingItem->setPreviousItem(0);
        _firstItem = draggingItem;
      }
      if(newNext!=0)
      {
        newNext->setPreviousItem(draggingItem);
        draggingItem->setNextItem(newNext);
      }
      else      //will become new _last
      {
        draggingItem->setNextItem(0);
        _lastItem = draggingItem;
      }

      draggingItem->setBeginIndex(clickedFrame);
      selectedItem = draggingItem;
    }
    else
      return;     //failed to drop an item on timeline

    cleanupAfterMove();
    repaint();
    emit droppedItem();
    trailContentChange();
    return;
  }
  if(clickedFrame != currentFrame)
  {
    currentFrame = clickedFrame;
    emit currentFrameChanged(currentFrame);
  }
  TrailItem* clickedItem = findItemOnFrame(currentFrame);
  if(clickedItem==0)
    emit backgroundClicked();
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


TrailItem* TimelineTrail::cutCurrentItem()
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
  else if(current==_lastItem)                       //last on trail
  {
    _lastItem = current->previousItem();
    current->previousItem()->setNextItem(0);
  }
  else    //somewhere in middle (current->previousItem() && current->nextItem())
  {
    current->previousItem()->setNextItem(current->nextItem());
    current->nextItem()->setPreviousItem(current->previousItem());
  }

  //This is probably not necessary
  current->setPreviousItem(0);
  current->setNextItem(0);

  selectedItem = 0;
  trailContentChange();
  return current;
}


// ---------------------- SLOTS ---------------------- //
/////////////////////////////////////////////////////////
void TimelineTrail::deleteCurrentItem()
{
  TrailItem* current = cutCurrentItem();
  delete current;
  repaint();
}

void TimelineTrail::moveCurrentItem()
{
  draggingItem = cutCurrentItem();
  emit movingItem(draggingItem);
}

void TimelineTrail::showLimbsWeight()
{
  //reemit to containing widget to handle it its own way
  emit adjustLimbsWeight(/*TODO: frameData*/);
}

void TimelineTrail::onMovingItem(TrailItem* draggedItem)
{
  QCursor movCur(Qt::SizeAllCursor);
  setCursor(movCur);
  draggingItem=draggedItem;     //in case of sender, this is not needed
}

void TimelineTrail::onDroppedItem()
{
  cleanupAfterMove();
  repaint();
}
