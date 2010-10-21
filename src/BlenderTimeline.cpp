/*
  The timeline widget inside BlenderTab. Includes 3 tracks.
*/
#define BLENDING_TRACKS      3
#define MIN_TRAIL_FRAMES     150  //edu: trail is 150 frames long

#include <QHBoxLayout>
#include <QScrollArea>
#include "BlenderTimeline.h"
#include "TimelineTrail.h"
#include "LimbsWeightForm.h"



BlenderTimeline::BlenderTimeline(QWidget* parent, Qt::WindowFlags) : QFrame(parent)
{
  trailFramesCount = MIN_TRAIL_FRAMES;

  scrollArea = new QScrollArea(0);
  scrollArea->setBackgroundRole(QPalette::Dark);
  scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);        //TODO: AsNeeded?
  QVBoxLayout* scrollLayout = new QVBoxLayout(scrollArea);
  scrollLayout->setMargin(1);
  scrollLayout->setSpacing(2);

  for(int i=0; i<BLENDING_TRACKS; i++)
  {
    TimelineTrail* tt = new TimelineTrail(scrollArea);
    tt->setNumberOfFrames(trailFramesCount);
    //if current frame of a trail has changed, sync all others
    connect(tt, SIGNAL(currentFrameChanged(int)), this, SLOT(setCurrentFrame(int)));
    connect(tt, SIGNAL(selectedItemChanged()), this, SLOT(unselectOldItem()));
    connect(tt, SIGNAL(positionCenter(int)), this, SLOT(scrollTo(int)));
    connect(tt, SIGNAL(adjustLimbsWeight(/*TODO: frameData*/)), this, SLOT(showLimbsWeightForm(/*TODO: frameData*/)));
    trails.append(tt);
    scrollLayout->addWidget(tt);
  }

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

bool BlenderTimeline::AddAnimation(Animation* anim, QString title)
{
  foreach(TimelineTrail* trail, trails)
  {
    if(trail->AddAnimation(anim, title))
      return true;
  }
  return false;
}

void BlenderTimeline::setCurrentFrame(int frameIndex)
{
  foreach(TimelineTrail* trail, trails)
    trail->setCurrentFrame(frameIndex);
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

//SLOT. A TimelineTrail asked us to show and fill the limbs' weights form
void BlenderTimeline::showLimbsWeightForm(/*TODO: frameData*/)
{
  limbsForm->show();
}
