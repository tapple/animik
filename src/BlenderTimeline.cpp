/*
  The timeline widget inside BlenderTab. Includes 3 tracks.
*/
#define BLENDING_TRACKS      3
#define MIN_TRAIL_FRAMES     150  //edu: trail is 150 frames long

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QScrollArea>
#include "Blender.h"
#include "BlenderTimeline.h"
#include "NoArrowsScrollArea.h"
#include "TimelineTrail.h"
#include "TrailItem.cpp"
#include "LimbsWeightForm.h"


#include <QSizePolicy>       //for DEBUG purposes



BlenderTimeline::BlenderTimeline(QWidget* parent, Qt::WindowFlags) : QFrame(parent)
{
  trailFramesCount = MIN_TRAIL_FRAMES;
  resultAnimation = 0;

  scrollArea = new NoArrowsScrollArea(this);
  scrollArea->setBackgroundRole(QPalette::Dark);
  scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);        //TODO: AsNeeded?
  QVBoxLayout* scrollLayout = new QVBoxLayout(/*scrollArea*/0);
  scrollLayout->setMargin(1);
  scrollLayout->setSpacing(2);

  stackWidget = new QWidget(/*this*/scrollArea);

  for(int i=0; i<BLENDING_TRACKS; i++)
  {
    TimelineTrail* tt = new TimelineTrail(stackWidget, 0, QString::number(i+1));
    tt->setPositionCount(trailFramesCount);
    //if current frame of a trail has changed, sync all others
    connect(tt, SIGNAL(currentPositionChanged(int)), this, SLOT(setCurrentFrame(int)));
    connect(tt, SIGNAL(selectedItemChanged()), this, SLOT(unselectOldItem()));
    connect(tt, SIGNAL(backgroundClicked()), this, SLOT(unselectOldItem()));
    connect(tt, SIGNAL(movingItem(TrailItem*)), this, SLOT(startItemReposition(TrailItem*)));
    connect(tt, SIGNAL(droppedItem()), this, SLOT(endItemReposition()));
    connect(tt, SIGNAL(adjustLimbsWeight(/*TODO: frameData*/)), this, SLOT(showLimbsWeightForm(/*TODO: frameData*/)));
    connect(tt, SIGNAL(positionsCountChanged(int)), this, SLOT(setFramesCount(int)));


    connect(tt, SIGNAL(trailAnimationChanged(WeightedAnimation*, int)),             //TODO: the signal is soon to be obsolete. Don't forget to delete this then
            this, SLOT(onTrailAnimationChanged(WeightedAnimation*, int)));

    connect(tt, SIGNAL(trailContentChanged(TrailItem*)), this, SLOT(onTrailContentChanged()));

    trails.append(tt);
    scrollLayout->addWidget(tt);
  }

  setFocusPolicy(Qt::WheelFocus);         //I need this if I want to handle key press event
  stackWidget->setFocusPolicy(Qt::WheelFocus);
  scrollArea->setFocusPolicy(Qt::WheelFocus);
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

TrailItem* BlenderTimeline::GetSelectedItem() const
{
  for(int i=0; i<trails.size(); i++)
  {
    if(trails.at(i)->getSelectedItem() != NULL)
      return trails.at(i)->getSelectedItem();
  }

  return NULL;
}


void BlenderTimeline::RebuildResultingAnimation()
{
  int count = trails.size();
  int minBeginIndex = 999999999;
  TrailItem** rails = new TrailItem*[count];

  //construct array form needed by Blender and find first frame position of result
  for(int i=0; i<count; i++)
  {
    rails[i] = trails.at(i)->firstItem();
    if(trails.at(i)->firstItem() != NULL && trails.at(i)->firstItem()->beginIndex() < minBeginIndex)
      minBeginIndex = trails.at(i)->firstItem()->beginIndex();
  }
  animationBeginPosition = minBeginIndex;

  if(resultAnimation)
    disconnect(resultAnimation, SIGNAL(currentFrame(int)), 0, 0);     //edu: is this needed?

  Blender* blender = new Blender();
  resultAnimation = blender->BlendTrails(rails, count);

  if(resultAnimation)
    connect(resultAnimation, SIGNAL(currentFrame(int)), this, SLOT(onPlayFrameChanged(int)));
  emit resultingAnimationChanged(resultAnimation);
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

void BlenderTimeline::keyPressEvent(QKeyEvent* e)
{
  e->ignore();
}

void BlenderTimeline::ensurePlayFrameVisibility(int position)
{
  scrollArea->ensureVisible(TimelineTrail::positionWidth()*position, 0, TimelineTrail::positionWidth()*5, 0);
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

  ensurePlayFrameVisibility(frameIndex);
}

void BlenderTimeline::unselectOldItem()
{
  foreach(TimelineTrail* trail, trails)
    if(trail != sender())
      trail->cancelTrailSelection();
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




void BlenderTimeline::onTrailAnimationChanged(WeightedAnimation* anim, int beginFrame)          //This is never called. TODO: cleanup
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

void BlenderTimeline::onTrailContentChanged()
{
  RebuildResultingAnimation();
}





void BlenderTimeline::onPlayFrameChanged(int playFrame)
{
  foreach(TimelineTrail* trail, trails)
    trail->setCurrentPosition(animationBeginPosition + playFrame);
}
