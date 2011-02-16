/*
  The timeline widget inside BlenderTab. Includes 3 tracks.
*/
#define BLENDING_TRACKS      3
#define MIN_TRAIL_FRAMES     150  //edu: trail is 150 frames long

#include <QHBoxLayout>
#include <QScrollArea>
#include "BlenderTimeline.h"
#include "TimelineTrail.h"
#include "TrailItem.cpp"
#include "LimbsWeightForm.h"


#include <QSizePolicy>       //for DEBUG purposes



BlenderTimeline::BlenderTimeline(QWidget* parent, Qt::WindowFlags) : QFrame(parent)
{
  trailFramesCount = MIN_TRAIL_FRAMES;
  resultAnimation = 0;

  scrollArea = new QScrollArea(0);
  scrollArea->setBackgroundRole(QPalette::Dark);
  scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);        //TODO: AsNeeded?
  QVBoxLayout* scrollLayout = new QVBoxLayout(/*scrollArea*/0);
  scrollLayout->setMargin(1);
  scrollLayout->setSpacing(2);


  stackWidget = new QWidget(this);


  for(int i=0; i<BLENDING_TRACKS; i++)
  {
    TimelineTrail* tt = new TimelineTrail(/*scrollArea*/stackWidget);
    tt->setPositionCount(trailFramesCount);
    //if current frame of a trail has changed, sync all others
    connect(tt, SIGNAL(currentPositionChanged(int)), this, SLOT(setCurrentFrame(int)));
    connect(tt, SIGNAL(selectedItemChanged()), this, SLOT(unselectOldItem()));
    connect(tt, SIGNAL(backgroundClicked()), this, SLOT(unselectOldItem()));
    connect(tt, SIGNAL(positionCenter(int)), this, SLOT(scrollTo(int)));
    connect(tt, SIGNAL(movingItem(TrailItem*)), this, SLOT(startItemReposition(TrailItem*)));
    connect(tt, SIGNAL(droppedItem()), this, SLOT(endItemReposition()));
    connect(tt, SIGNAL(adjustLimbsWeight(/*TODO: frameData*/)), this, SLOT(showLimbsWeightForm(/*TODO: frameData*/)));
    connect(tt, SIGNAL(positionsCountChanged(int)), this, SLOT(setFramesCount(int)));
    connect(tt, SIGNAL(trailAnimationChanged(WeightedAnimation*, int)),
            this, SLOT(onTrailAnimationChanged(WeightedAnimation*, int)));

    trails.append(tt);
    scrollLayout->addWidget(tt);
  }

  stackWidget->setFocusPolicy(Qt::WheelFocus);
  stackWidget->setLayout(scrollLayout);
  scrollArea->setWidget(stackWidget);
  needsReshape = true;

  limbsForm = new LimbsWeightForm(this);

  QHBoxLayout* layout=new QHBoxLayout(this);
  layout->setMargin(0);
  layout->addWidget(scrollArea);
  layout->addWidget(limbsForm);
  limbsForm->hide();
}

BlenderTimeline::~BlenderTimeline()
{
  while (!trails.isEmpty())           //delete trails one by one
    delete trails.takeFirst();
}

bool BlenderTimeline::AddAnimation(WeightedAnimation* anim, QString title)
{
  foreach(TimelineTrail* trail, trails)
  {
    if(trail->AddAnimation(anim, title))
    {
//      fitStackWidgetToContent();
      repaint();
      return true;
    }
  }
  return false;
}


void BlenderTimeline::ConstructTimeLine(QList<TrailItem*>* trails)
{
  for(int i=0; i<this->trails.size() && i<trails->size(); i++)
    this->trails.at(i)->ResetContent(trails->at(i));
}


QList<TimelineTrail*> BlenderTimeline::Trails()
{
  return trails;
}

int BlenderTimeline::TrailCount()
{
  return trails.size();
}


void BlenderTimeline::fitStackWidgetToContent()
{
  TimelineTrail* t = trails.at(0);
  int wdth = t->positionWidth()*MIN_TRAIL_FRAMES;
  if(t->positionCount())
    wdth = t->positionWidth()*t->positionCount();
  stackWidget->resize(wdth, size().height()-20);    //20 is cca scrollbar height
}


void BlenderTimeline::paintEvent(QPaintEvent*)
{
  if(needsReshape)
  {
    fitStackWidgetToContent();
    needsReshape=false;
  }
}

// -------------------------- SLOTS -------------------------- //
void BlenderTimeline::setCurrentFrame(int frameIndex)
{
  if(resultAnimation !=0 && frameIndex >= animationBeginPosition &&       //inside of overall animation
     frameIndex < animationBeginPosition + resultAnimation->getNumberOfFrames())
  {
    resultAnimation->setFrame(frameIndex - animationBeginPosition);        //this causes AnimationView and Player to be updated
  }

  foreach(TimelineTrail* trail, trails)
    trail->setCurrentPosition(frameIndex);
}

void BlenderTimeline::unselectOldItem()
{
  foreach(TimelineTrail* trail, trails)
    if(trail != sender())
      trail->cancelTrailSelection();
}

void BlenderTimeline::scrollTo(int x)
{
  QSize size = scrollArea->size();
  scrollArea->ensureVisible(x, 0, size.width()/4, size.height());
}

void BlenderTimeline::startItemReposition(TrailItem* draggingItem)
{
  foreach(TimelineTrail* trail, trails)
    trail->onMovingItem(draggingItem);
}

void BlenderTimeline::endItemReposition()
{
  foreach(TimelineTrail* trail, trails)
    if(trail != sender())
      trail->onDroppedItem();
}

//A TimelineTrail asked us to show and fill the limbs' weights form
void BlenderTimeline::showLimbsWeightForm(/*TODO: frameData*/)
{
  limbsForm->show();
}

void BlenderTimeline::setFramesCount(int newCount)
{
  foreach(TimelineTrail* trail, trails)
//    if(trail != sender())
    trail->setPositionCount(newCount);

  needsReshape = true;
  repaint();
}

void BlenderTimeline::onTrailAnimationChanged(WeightedAnimation* anim, int beginFrame)
{
  //DEBUG so far, TODO: recalculate overall animation
  //TODO: find a mechanism to set the real offset
  if(resultAnimation)
    disconnect(resultAnimation, SIGNAL(currentFrame(int)), 0, 0);     //edu: is this needed?
  resultAnimation = anim;
  if(resultAnimation)
    connect(resultAnimation, SIGNAL(currentFrame(int)), this, SLOT(onPlayFrameChanged(int)));
  animationBeginPosition = beginFrame;
  emit resultingAnimationChanged(anim);
}

void BlenderTimeline::onPlayFrameChanged(int playFrame)
{
  foreach(TimelineTrail* trail, trails)
    trail->setCurrentPosition(animationBeginPosition + playFrame);
}
