#ifndef BLENDERTIMELINE_H
#define BLENDERTIMELINE_H

#include <QFrame>
#include <QList>
#include "WeightedAnimation.h"

class QKeyEvent;
class QSize;
class LimbsWeightForm;
class NoArrowsScrollArea;
class TimelineTrail;
class TrailItem;


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
    /** External object may use this to explicitely reevaluate the time-line and rebuild
        the overall animation. Supposed to be used for case of change in global application settings. */
    void RebuildResultingAnimation();
    void HideLimsForm();
    /** Return currently selected TrailItem. If none was selected by user, NUUL is returned */
    TrailItem* getSelectedItem() const;
    /** True if doesn't contain any animation */
    bool isClear() const;
    QList<TimelineTrail*> Trails();
    int TrailCount();
    void limitUserActions(bool limit);

  signals:
    /** Warn about Animation and its first frame position */
    void resultingAnimationChanged(WeightedAnimation*);


  protected slots:
    void setCurrentFrame(int frameIndex);
    /** Find trail with selected TrailItem and make it unselect the item */
    void unselectOldItem();
    void showLimbsWeightForm();
    void setFramesCount(int newCount);
    void startItemReposition(TrailItem* draggingItem);
    void endItemReposition();


//TODO: delete    void onTrailAnimationChanged(WeightedAnimation* anim, int beginFrame);
    void onTrailContentChanged();


    void onPlayFrameChanged(int playFrame);

  protected:
    NoArrowsScrollArea* scrollArea;
    QWidget* stackWidget;
    QList<TimelineTrail*> trails;
    LimbsWeightForm* limbsForm;
    virtual void paintEvent(QPaintEvent*);
    virtual void keyPressEvent(QKeyEvent *);

  private:
    int animationBeginPosition;        //the offset denoting first frame of the overall animation
    int trailFramesCount;
    int needsReshape;
    WeightedAnimation* resultAnimation;

    void fitStackWidgetToContent();
    void ensurePlayFrameVisibility(int position);
};


#endif // BLENDERTIMELINE_H
