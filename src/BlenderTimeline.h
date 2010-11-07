#ifndef BLENDERTIMELINE_H
#define BLENDERTIMELINE_H

#include <QFrame>
#include <QList>

#include "animation.h"

class QScrollArea;
class QSize;
class TimelineTrail;
class TrailItem;
class LimbsWeightForm;


class BlenderTimeline : public QFrame
{
  Q_OBJECT

  public:
    BlenderTimeline(QWidget* parent=0, Qt::WindowFlags f=0);
    ~BlenderTimeline();

    /** Add new animation to a track where fits first. Returns TRUE on success */
    bool AddAnimation(Animation* anim, QString title);

  signals:
    void resultingAnimationChanged(Animation*);


  protected slots:
    void setCurrentFrame(int frameIndex);
    /** Find trail with selected TrailItem and make it unselect it */
    void unselectOldItem();
    void showLimbsWeightForm(/*TODO: frameData*/);
    void setFramesCount(int newCount);
    void startItemReposition(TrailItem* draggingItem);
    void endItemReposition();
    void onTrailAnimationChanged(Animation* anim);

  protected:
    QScrollArea* scrollArea;
    QWidget* stackWidget;
    QList<TimelineTrail*> trails;
    LimbsWeightForm* limbsForm;
    void scrollTo(int x);
    virtual void paintEvent(QPaintEvent*);

  private:
    int trailFramesCount;
    int needsReshape;

    void fitStackWidgetToContent();
};


#endif // BLENDERTIMELINE_H
