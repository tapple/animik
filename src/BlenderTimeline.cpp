/*
  The timeline widget inside BlenderTab. Includes 3 tracks.
*/


#define BLENDING_TRACKS      3
#define MIN_TRAIL_FRAMES     150  //edu: trail 150 frames long

#include <QHBoxLayout>
#include <QScrollArea>
#include "BlenderTimeline.h"
#include "TimelineTrail.h"



BlenderTimeline::BlenderTimeline(QWidget* parent, Qt::WindowFlags) : QFrame(parent)
{
  trailFramesCount = MIN_TRAIL_FRAMES;

  for(int i=0; i<BLENDING_TRACKS; i++)
  {
    TimelineTrail* tt = new TimelineTrail(this, 0);         //TODO: full trail initialisation
    tt->setNumberOfFrames(trailFramesCount);
    connect(tt, SIGNAL(positionCenter(int)), this, SLOT(scrollTo(int)));
    trails.append(tt);
  }

  scrollArea = new QScrollArea(0);
  scrollArea->setBackgroundRole(QPalette::Dark);
  scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);        //TODO: AsNeeded?

  QHBoxLayout* layout=new QHBoxLayout(this);
  layout->addWidget(scrollArea);
}

BlenderTimeline::~BlenderTimeline()
{
  while (!trails.isEmpty())           //delet trails one by one
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

void BlenderTimeline::scrollTo(int x)
{
  QSize size = scrollArea->size();
  scrollArea->ensureVisible(x, 0, size.width()/4, size.height());
}
