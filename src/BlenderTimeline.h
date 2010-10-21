#ifndef BLENDERTIMELINE_H
#define BLENDERTIMELINE_H

#include <QFrame>
#include <QList>

#include "animation.h"

class QScrollArea;
class QSize;
class TimelineTrail;
class LimbsWeightForm;


class BlenderTimeline : public QFrame
{
  Q_OBJECT

  public:
    BlenderTimeline(QWidget* parent=0, Qt::WindowFlags f=0);
    ~BlenderTimeline();

    /** Add new animation to a track where fits first. Returns TRUE on success */
    bool AddAnimation(Animation* anim, QString title);

  protected slots:
    void setCurrentFrame(int frameIndex);
    /** Find trail with selected TrailItem and make it unselect it */
    void unselectOldItem();
    void showLimbsWeightForm(/*TODO: frameData*/);

  protected:
    QScrollArea* scrollArea;
    QList<TimelineTrail*> trails;
    LimbsWeightForm* limbsForm;
    void scrollTo(int x);

  private:
    int trailFramesCount;
};


#endif // BLENDERTIMELINE_H
