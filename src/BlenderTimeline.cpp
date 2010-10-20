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
    connect(tt, SIGNAL(positionCenter(int)), this, SLOT(scrollTo(int)));
    trails.append(tt);
    scrollLayout->addWidget(tt);
  }

  LimbsWeightForm* lwForm = new LimbsWeightForm(this);

  QHBoxLayout* layout=new QHBoxLayout(this);
  layout->setMargin(0);
  layout->addWidget(scrollArea);
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

void BlenderTimeline::scrollTo(int x)
{
  QSize size = scrollArea->size();
  scrollArea->ensureVisible(x, 0, size.width()/4, size.height());
}
