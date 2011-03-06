#include <QAction>
#include <QCursor>
#include <QContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QPalette>
#include <QSize>
#include "bvh.h"
#include "MixZonesDialog.h"
#include "settings.h"
#include "TimelineTrail.h"
#include "TrailItem.cpp"
#include "TrailJoiner.h"


#include <QMessageBox>        //DEBUG purposes


#define TRACK_HEIGHT     72
#define FRAME_HEIGHT     TRACK_HEIGHT-6
#define TOP_MARGIN       2
#define BORDER_WIDTH     2
#define TEXT_SIZE        16
#define MIN_FRAME_WIDTH  8     //minimum frame space width that can't be crossed when zooming
#define MAX_FRAME_WIDTH  20    //TODO: maximum when zooming
#define ZOOM_STEP        4     //TODO: the zooming
#define MAX_TRAIL_FRAMES 400//DEBUG 4000
#define MAX_WEIGHT       100.0 //max frame weight


int TimelineTrail::_positionWidth = MIN_FRAME_WIDTH;


TimelineTrail::TimelineTrail(QWidget* parent, Qt::WindowFlags, QString debugName) : QFrame(parent)
{
  offscreen = NULL;
  currentPosition = 0;
  _firstItem = NULL;
  _lastItem = NULL;
  selectedItem = NULL;
  draggingItem = NULL;
  draggingOverPosition = -1;
  settingWeight = false;
  leftMouseDown = rightMouseDown = false;
  addingNewItem = false;
  //we need to receive mouseMoveEvent even when no button down
  setMouseTracking(true);
  //receive key press events
  setFocusPolicy(Qt::WheelFocus);

  //Actions that may appear in context menu
  deleteItemAction = new QAction(tr("Delete animation"), this);
  connect(deleteItemAction, SIGNAL(triggered()), this, SLOT(deleteCurrentItem()));
  moveItemAction = new QAction(tr("Move on timeline"), this);
  connect(moveItemAction, SIGNAL(triggered()), this, SLOT(moveCurrentItem()));
  mixZonesAction = new QAction(tr("Set mix-in/out"), this);
  connect(mixZonesAction, SIGNAL(triggered()), this, SLOT(setMixZones()));
  limbWeightsAction = new QAction(tr("Set limbs' weights"), this);
  connect(limbWeightsAction, SIGNAL(triggered()), this, SLOT(showLimbsWeight()));


  _debugName = debugName;
}


bool TimelineTrail::AddAnimation(WeightedAnimation* anim, QString title)
{
  addingNewItem = true;
  clearShadowItems();

  TrailItem* toBePredecessor;
  try {
    toBePredecessor = findFreeSpace(anim->getNumberOfFrames());
  }
  catch(QString* message)
  {    //Not enough space on this trail. Probably we can't do more here.
    if(*message == "Not enough space")
      return false;
  }

  TrailItem* newItem;
  if(toBePredecessor == 0)                  //will become new first item
  {
    newItem = new TrailItem(anim, title, 0, false);
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
    newItem = new TrailItem(anim, title, toBePredecessor->endIndex()+1, false);
    newItem->setNextItem(0);
    newItem->setPreviousItem(toBePredecessor);
    toBePredecessor->setNextItem(newItem);
    _lastItem = newItem;
  }
  else
  {
    newItem = new TrailItem(anim, title, toBePredecessor->endIndex()+1, false);
    newItem->setNextItem(toBePredecessor->nextItem());
    newItem->setPreviousItem(toBePredecessor);
    toBePredecessor->setNextItem(newItem);
    newItem->nextItem()->setPreviousItem(newItem);
  }

  addingNewItem = false;
  updateTimelineItemsIndices();
  trailContentChange();
  repaint();
  return true;
}


void TimelineTrail::ResetContent(TrailItem *first)
{
  //no deletion is performed because this should be always used to an empty trail
  _firstItem = first;

  //let's find last item
  TrailItem* currentItem=_firstItem;
  while(currentItem->nextItem() != NULL)
    currentItem = currentItem->nextItem();
  _lastItem = currentItem;
  
  if(_lastItem->endIndex() >= positionCount())
    coerceExtension(_lastItem->endIndex() - positionCount() +1);

  trailContentChange();
}


void TimelineTrail::setPositionCount(int positions)
{
//  qDebug("TimelineTrail::setPositionCount(%d==%d): %0lx", positionsCount, positions, (unsigned long) offscreen);

  if(positions==positionsCount)
    return;   //nothing to do

  positionsCount=positions;
  QPixmap* newOffscreen = new QPixmap(positionsCount*_positionWidth, TRACK_HEIGHT);

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


void TimelineTrail::setCurrentPosition(int position)
{
  if(position != currentPosition)
  {
    currentPosition=position;
    emit currentPositionChanged(position);
    repaint();
  }

  //Find TrailItem underneath if any, and select it's frame
  //Also done for the Trail where the frame was set by click.
  TrailItem* item = findItemOnPosition(currentPosition);
  if(item != 0)
  {
    int toSel = currentPosition - item->beginIndex();
    item->selectFrame(toSel);
  }
}

/*********************************** RELATED TO ADDING AN ITEM ************************************/
TrailItem* TimelineTrail::findFreeSpace(int positions)
{
  if(_firstItem == 0 && positions <= positionsCount)
    return 0;
  if(_firstItem->beginIndex() >= positions)      //is there a space before first?
    return 0;

  TrailItem* currentItem = _firstItem;

  while(currentItem->nextItem() != 0)
  {
    if((currentItem->nextItem()->beginIndex() - currentItem->endIndex() - 1) >= positions)
      return currentItem;
    currentItem = currentItem->nextItem();
  }

  //check space after last item
  if((positionsCount - _lastItem->endIndex()) >= positions)
    return _lastItem;

  //try to extend this trail to ensure place at the end
  int remaining = _lastItem ? positionsCount-_lastItem->endIndex()-1
                            : positionsCount;
  if(coerceExtension(positions-remaining))
    return _lastItem;

  throw new QString("Not enough space");
}


bool TimelineTrail::coerceExtension(int size)
{
  if (positionsCount+size <= MAX_TRAIL_FRAMES)
  {
    setPositionCount(positionsCount+size +2);       //+2 for user's convenience
    emit positionsCountChanged(positionsCount);
    return true;
  }
  else
    return false;
}

void TimelineTrail::updateTimelineItemsIndices()
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

bool TimelineTrail::isSuitableSpace(int beginPosition, int positionsCount)
{
  int endFrame = beginPosition+positionsCount-1;
  if(_firstItem == 0 /*&& _lastItem == 0*/ && endFrame <= (MAX_TRAIL_FRAMES-1))
    return true;

  TrailItem* currentItem = _firstItem;
  while(currentItem != 0)
  {
    if(currentItem->beginIndex()>endFrame)    //all items from here are too far, so safe
      return true;
    if((currentItem->beginIndex()>=beginPosition && currentItem->beginIndex()<=endFrame) ||
       (currentItem->endIndex()>=beginPosition && currentItem->endIndex()<=endFrame) ||
       (beginPosition>=currentItem->beginIndex() && beginPosition<=currentItem->endIndex()))
      return false;                           //overlaps with currentItem

    currentItem = currentItem->nextItem();
  }

  //it wants to be on the end, test (possible) space available
  return endFrame <= (MAX_TRAIL_FRAMES-1);
}
/******************************************************************************************/


/*********************************** DRAW GUI ELEMENTS ************************************/
void TimelineTrail::drawBackground()
{
  if(!offscreen) return;

  QPainter* p = new QPainter(offscreen);

  p->fillRect(0, 0, positionsCount*_positionWidth, TRACK_HEIGHT, QColor("#aeaeae"));
  // draw current frame marker
  p->fillRect(currentPosition*_positionWidth, 0, _positionWidth, TRACK_HEIGHT, QColor("#cd8f1d"));

  for(int i=0; i<positionsCount; i++)
  {
    QPoint upPoint(i*_positionWidth, 0);
    QPoint downPoint(i*_positionWidth, TRACK_HEIGHT);
    if(i==currentPosition)
      p->setPen(QColor("#ffffff"));
    else
      p->setPen(QColor("#808080"));
    p->drawLine(upPoint, downPoint);
  }

  delete p;
}

void TimelineTrail::drawMovedItemShadow()
{
  QPainter* p = new QPainter(offscreen);
  p->fillRect(draggingOverPosition*_positionWidth, 0,
              draggingItem->frames()*_positionWidth, TRACK_HEIGHT-3, QColor("#d7df01"));
  delete p;
}

void TimelineTrail::drawTrailItem(TrailItem* item)
{
  if(!offscreen) return;
  if(item==draggingItem) return;

  QPainter p(offscreen);
  QColor boxColor;
  QColor selFrameColor;
  if(item==selectedItem)
  {
    boxColor = QColor("#5c4179");
    selFrameColor = QColor("#453755");
  }
  else
  {
    boxColor = QColor("#584a68");
    selFrameColor = QColor("#725518");
  }

  if(Settings::Instance()->Debug() && item->isShadow())
    boxColor = QColor("#55bb55");

  if(Settings::Instance()->Debug() || !item->isShadow())
  {
    int dbg = item->isShadow() ? (TOP_MARGIN+BORDER_WIDTH) : 0;

    p.fillRect(item->beginIndex()*_positionWidth+2, TOP_MARGIN+BORDER_WIDTH-dbg,
               item->frames()*_positionWidth-2, FRAME_HEIGHT+dbg+dbg, boxColor);

    //emphasize the first frame if it's T-pose
    if(!item->isShadow() && item->getAnimation()->isFirstFrameTPose())
      p.fillRect(item->beginIndex()*_positionWidth+2, TOP_MARGIN+BORDER_WIDTH, _positionWidth-1,
                 FRAME_HEIGHT, QColor("#84365D"));

    //Write the name of animation onto it's rectangle for all except the selected one.
    if(!item->isShadow() && item!=selectedItem)
    {
        QPen textPen(QColor("#a181c0"));
        QFont f ("Arial", 13, QFont::Bold);
        p.setPen(textPen);
        p.setFont(f);

        p.drawText(item->beginIndex()*_positionWidth+BORDER_WIDTH, TRACK_HEIGHT/3+TOP_MARGIN, item->frames()*_positionWidth,
                   TRACK_HEIGHT/3, Qt::AlignLeft, item->Name);
    }

    QRectF border(item->beginIndex()*_positionWidth+1, TOP_MARGIN+1,
                  item->frames()*_positionWidth, FRAME_HEIGHT);

    int begin = item->beginIndex();
    //if selected frame falls inside this animation, highlight the frame
    if(currentPosition>=begin && currentPosition<=item->endIndex())
    {
      p.fillRect(currentPosition*_positionWidth+1, TOP_MARGIN+BORDER_WIDTH,
                 _positionWidth, FRAME_HEIGHT-BORDER_WIDTH, selFrameColor);
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
        if(i==currentPosition)
        {
          p.fillRect(i*_positionWidth+2, TOP_MARGIN+BORDER_WIDTH+(FRAME_HEIGHT-BORDER_WIDTH)-barHeight,
                     _positionWidth-2, barHeight, QColor("#0080ff"));
        }
        else
        {
          p.fillRect(i*_positionWidth+2, TOP_MARGIN+BORDER_WIDTH+(FRAME_HEIGHT-BORDER_WIDTH)-barHeight,
                     _positionWidth-2, barHeight, barColor);
        }

        //Weight value for selected frame is show as the very last thing
        if(currentPosition>=begin && currentPosition<=end)
        {
          int w = item->getWeight(currentPosition-begin);
          QPen textPen(QColor("#f8f8f8"));
          QFont f ("Arial", 11, QFont::Normal);
          p.setPen(textPen);
          p.setFont(f);

         //for some galactic reason the center alignment doesn't work as expected, so must be done manually
          if(w==100)
            p.drawText(currentPosition*_positionWidth-_positionWidth, TRACK_HEIGHT-TOP_MARGIN-BORDER_WIDTH-TEXT_SIZE,
                       _positionWidth*3, TEXT_SIZE, Qt::AlignJustify, QString::number(w));
          else if(w<10)
            p.drawText(currentPosition*_positionWidth, TRACK_HEIGHT-TOP_MARGIN-BORDER_WIDTH-TEXT_SIZE,
                       _positionWidth, TEXT_SIZE, Qt::AlignJustify, QString::number(w));
          else    //10-99
            p.drawText(currentPosition*_positionWidth-(_positionWidth/2), TRACK_HEIGHT-TOP_MARGIN-BORDER_WIDTH-TEXT_SIZE,
                       _positionWidth*2, TEXT_SIZE, Qt::AlignJustify, QString::number(w));
        }
      }
    }

  }//if(Debug())
}
/***********************************************************************************/


TrailItem* TimelineTrail::findItemOnPosition(int positionIndex)
{
  TrailItem* currentItem = _firstItem;

  while(currentItem != NULL)
  {
    int begin = currentItem->beginIndex();
    if(positionIndex>=begin && positionIndex<=currentItem->endIndex())
      break;
    currentItem = currentItem->nextItem();
  }
  return currentItem;
}


TrailItem* TimelineTrail::findPreviousItem(int beforePosition)
{
  if(_firstItem==NULL/*==_lastItem*/)
    return NULL;
  if(_firstItem->beginIndex() > beforePosition)
    return NULL;         //belongs to beginning
  if(_firstItem==_lastItem)
  {
    if(_firstItem->endIndex() < beforePosition)
      return _firstItem;
    else
      return NULL;
  }

  TrailItem* currentItem = _firstItem;
  while(currentItem != NULL)
  {
    if(currentItem->nextItem() == NULL)
      return currentItem;
    if(currentItem->nextItem()->beginIndex() > beforePosition)
      return currentItem;
    currentItem = currentItem->nextItem();
  }

  throw new QString("findNextItem: unable to return valid TrailItem object");
}


TrailItem* TimelineTrail::findNextItem(int afterPosition)
{
  if(_lastItem == NULL /*==_firstItem*/)
    return NULL;
  if(_lastItem->endIndex() < afterPosition)
    return NULL;           //belongs to very end
  if(_firstItem==_lastItem)
  {
    if(_lastItem->beginIndex() > afterPosition)
     return _lastItem;
    else
      return NULL;
  }

  TrailItem* currentItem = _lastItem;
  while(currentItem != NULL)
  {
    if(currentItem->previousItem() == NULL)
      return currentItem;
    if(currentItem->previousItem()->endIndex() < afterPosition)
      return currentItem;
    currentItem = currentItem->previousItem();
  }

  throw new QString("findNextItem: unable to return valid TrailItem object");
}


void TimelineTrail::cleanupAfterMove()
{
  if(draggingItem != NULL)
    draggingItem = NULL;

  draggingOverPosition = -1;
  QCursor defCur;
  setCursor(defCur);
}


/**************************** BUILD RESULTING ANIMATION ****************************/
WeightedAnimation* TimelineTrail::getSummaryAnimation()
{
  if(_firstItem==NULL || draggingItem!=NULL || addingNewItem)
    return NULL;

  clearShadowItems();
  TrailJoiner joiner(_firstItem);
  return joiner.GetJoinedAnimation();
}


/** After placement change, some auxiliary items placed by TrailJoiner may become invalid.
    It's better just to erase them all. Done by this method. */
void TimelineTrail::clearShadowItems()                //Shouldn't this rather be done in the Blender class? TODO: resolve it.
{
  TrailItem* currentItem = _firstItem;
  while(currentItem != NULL)
  {
    if(currentItem->isShadow())
    {
      TrailItem* next = currentItem->nextItem();

      TrailItem* rubish = cutItem(currentItem);
      delete rubish;

      currentItem = next;
      continue;
    }

    currentItem = currentItem->nextItem();
  }
}
/***********************************************************************************/



TrailItem* TimelineTrail::cutItem(TrailItem* current)
{
  //Cutting shadow (helper) item. That never can be _firstItem of _lastItem, so must treat it bit specially
  if(Settings::Instance()->Debug() && current->isShadow())
  {
    if(current->previousItem() != NULL)
      current->previousItem()->setNextItem(current->nextItem());
    if(current->nextItem() != NULL)
      current->nextItem()->setPreviousItem(current->previousItem());
    return current;
  }


  if(current==_firstItem && current==_lastItem)     //was only one on this trail
  {
    _firstItem = _lastItem = NULL;
  }
  else if(current==_firstItem)                      //was first on trail
  {
    //This is an ugly tax paid for that debug conditional code. I need to find REAL nextItem()
    TrailItem* anItem = current->nextItem();
    while(anItem->isShadow())
      anItem = anItem->nextItem();

    _firstItem = anItem;    //current->nextItem();
    current->nextItem()->setPreviousItem(NULL);
  }
  else if(current==_lastItem)                       //last on trail
  {
    //First find REAL previousItem()
    TrailItem* anItem = current->previousItem();
    while(anItem->isShadow())
      anItem = anItem->previousItem();

    _lastItem = anItem;     //current->previousItem();
    current->previousItem()->setNextItem(NULL);
  }
  else    //somewhere in middle (current->previousItem() && current->nextItem())
  {
    current->previousItem()->setNextItem(current->nextItem());
    current->nextItem()->setPreviousItem(current->previousItem());
  }
  //This is probably not necessary
  current->setPreviousItem(NULL);
  current->setNextItem(NULL);
  selectedItem = NULL;
  return current;
}


void TimelineTrail::adjustFrameWeight(int cursorYPosition)
{
  int offset = TOP_MARGIN+BORDER_WIDTH;
  float real_frame_height = TRACK_HEIGHT - 2*offset;
  int touch = cursorYPosition - offset;
  if(touch>=0 && touch<=real_frame_height)     //not above/under the TrailItem
  {
    float weight = 100.0 * (float)touch / real_frame_height;
    int frameIndex = currentPosition - selectedItem->beginIndex();
    int oldWeight = selectedItem->getAnimation()->getFrameWeight(frameIndex);
    int newWeight = 100 - (int)weight;
    if(oldWeight != newWeight)
    {
      selectedItem->getAnimation()->setFrameWeight(frameIndex, newWeight);
      emit trailContentChanged(_firstItem);
      repaint(currentPosition*_positionWidth-10, 0, _positionWidth+20, TRACK_HEIGHT);
    }
  }
}


void TimelineTrail::paintEvent(QPaintEvent*)
{
  QSize newSize = QSize(positionsCount*_positionWidth, TRACK_HEIGHT);

  if(newSize!=size())
  {
    resize(newSize);
    emit resized(newSize);
  }

  drawBackground();
  if(draggingOverPosition >= 0)
    drawMovedItemShadow();

  TrailItem* currentItem = _firstItem;
  while(currentItem != NULL)
  {
    drawTrailItem(currentItem);
    currentItem = currentItem->nextItem();
  }

  QPainter p(this);
  p.drawPixmap(0, 0, *offscreen);
}

void TimelineTrail::contextMenuEvent(QContextMenuEvent *event)
{
  if(/*rightMouseDown &&*/ selectedItem!=NULL && !selectedItem->isShadow())
  {
    QMenu menu(this);
    menu.addAction(deleteItemAction);
    menu.addAction(moveItemAction);
    menu.addSeparator();
    menu.addAction(mixZonesAction);
    menu.addAction(limbWeightsAction);
    menu.exec(event->globalPos());
  }
}

void TimelineTrail::mouseMoveEvent(QMouseEvent* me)
{
  if(draggingItem)
  {
    float hitFrame = ((float)me->pos().x()) / _positionWidth;
    int frame = (int)hitFrame;
    if(frame != draggingOverPosition)
    {
      draggingOverPosition = frame;
      repaint();
    }
  }

  if(leftMouseDown && settingWeight)
    adjustFrameWeight(me->y());
}

void TimelineTrail::leaveEvent(QEvent *)
{
  //user dragging an TrailItem left this area
  draggingOverPosition = -1;
  repaint();
}

void TimelineTrail::keyPressEvent(QKeyEvent* e)
{
  e->ignore();            //send it to parent
}

void TimelineTrail::mousePressEvent(QMouseEvent* e)
{
  int clickedFrame = e->x()/_positionWidth;

  //user wishes to place dragged item
  if(draggingItem)
  {
    if(isSuitableSpace(clickedFrame, draggingItem->frames()))
    {
      int endFrame = clickedFrame + draggingItem->frames() -1;
      if(endFrame+2 > positionsCount)      //we need to extend trail
      {
        int inside = positionsCount-clickedFrame;
        if(!coerceExtension(draggingItem->frames() - inside))
          throw new QString("coerceExtension unsuccessfull!");       //this should never happen
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
  if(clickedFrame != currentPosition)
  {
    currentPosition = clickedFrame;
    emit currentPositionChanged(currentPosition);
  }
  else if(selectedItem != 0 && !selectedItem->isShadow())     //second click on selected frame, user wishes to set weight
  {
    settingWeight = true;
    adjustFrameWeight(e->y());
  }
  TrailItem* clickedItem = findItemOnPosition(currentPosition);
  if(clickedItem==0)
    emit backgroundClicked();
  if(clickedItem != selectedItem)
  {
    selectedItem = clickedItem;
    emit selectedItemChanged();
  }


  //NOTE: current frame of every TrailItem that might be crossed is set elsewhere, through signal/slot loop.
//TODO: apply to resulting animation  animation->setFrame(currentFrame);
  if(e->button()==Qt::LeftButton)
    leftMouseDown = true;
  else if(e->button()==Qt::RightButton)
    rightMouseDown = true;

  repaint();
}

void TimelineTrail::mouseReleaseEvent(QMouseEvent *)
{
  if(settingWeight)
    repaint();
  leftMouseDown = rightMouseDown = settingWeight = false;
}


void TimelineTrail::trailContentChange()
{
//  WeightedAnimation* result = getSummaryAnimation();
//  emit trailAnimationChanged(result, _firstItem ? _firstItem->beginIndex() : -1);


//  clearShadowItems();
  emit trailContentChanged(_firstItem);
}


// ---------------------- SLOTS ---------------------- //
/////////////////////////////////////////////////////////
void TimelineTrail::deleteCurrentItem()
{
  TrailItem* current = cutItem(selectedItem);
  delete current;
  trailContentChange();
  repaint();
}

void TimelineTrail::moveCurrentItem()
{
  draggingItem = cutItem(selectedItem);
  emit movingItem(draggingItem);
}

void TimelineTrail::showLimbsWeight()
{
  //reemit to containing widget to handle it its own way
  emit adjustLimbsWeight(/*TODO: frameData*/);
}

void TimelineTrail::setMixZones()
{
  MixZonesDialog* mzd = new MixZonesDialog(selectedItem->frames(), selectedItem->mixIn(),
                                           selectedItem->mixOut(), this);
  mzd->exec();
  if(mzd->result() == QDialog::Accepted)
  {
    selectedItem->setMixIn(mzd->mixIn());
    selectedItem->setMixOut(mzd->mixOut());
    emit trailContentChanged(_firstItem);
  }
}

void TimelineTrail::onMovingItem(TrailItem* draggedItem)
{
  QCursor movCur(Qt::SizeAllCursor);
  setCursor(movCur);
  draggingItem=draggedItem;     //in case of sender, this is not needed

  //remove helper items so they're not in way of the one being dragged
  clearShadowItems();
}

void TimelineTrail::onDroppedItem()
{
  cleanupAfterMove();
  trailContentChange();
  repaint();
}
