#ifndef BLENDERTIMELINE_H
#define BLENDERTIMELINE_H

#include <QFrame>
#include <QList>

#include "WeightedAnimation.h"

class QKeyEvent;
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
    bool AddAnimation(WeightedAnimation* anim, QString title);
    /** Clean and rebuild time-line content from given list of linked TrailItems */
    void ConstructTimeLine(QList<TrailItem*>* trails);
    QList<TimelineTrail*> Trails();
    int TrailCount();

  signals:
    /** Warn about Animation and its first frame position */
    void resultingAnimationChanged(WeightedAnimation*);


  protected slots:
    void setCurrentFrame(int frameIndex);
    /** Find trail with selected TrailItem and make it unselect the item */
    void unselectOldItem();
    void showLimbsWeightForm(/*TODO: frameData*/);
    void setFramesCount(int newCount);
    void startItemReposition(TrailItem* draggingItem);
    void endItemReposition();


    void onTrailAnimationChanged(WeightedAnimation* anim, int beginFrame);
    void onTrailContentChanged(TrailItem* firstItem);


    void onPlayFrameChanged(int playFrame);

  protected:
    QScrollArea* scrollArea;
    QWidget* stackWidget;
    QList<TimelineTrail*> trails;
    LimbsWeightForm* limbsForm;
    void scrollTo(int x);
    virtual void paintEvent(QPaintEvent*);
    virtual void keyPressEvent(QKeyEvent *);

  private:
    int animationBeginPosition;        //the offset denoting first frame of the overall animation
    int trailFramesCount;
    int needsReshape;
    WeightedAnimation* resultAnimation;

    void fitStackWidgetToContent();
};


#endif // BLENDERTIMELINE_H
