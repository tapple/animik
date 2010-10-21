#ifndef TIMELINETRAIL_H
#define TIMELINETRAIL_H

#include <QFrame>
#include "animation.h"

class QPixmap;
class QAction;
class TrailItem;


class TimelineTrail : public QFrame
{
  Q_OBJECT

  public:
    TimelineTrail(QWidget* parent=0, Qt::WindowFlags f=0);

    /** Add new animation on first gap big enough. Returns TRUE on success
        or FALSE if there's not enough space on this trail. */
    bool AddAnimation(Animation* anim, QString title);
    void setCurrentFrame(int frame);
    int animationCount() { return _animationCount; }
    TrailItem* firstItem() { return _firstItem; }
    TrailItem* lastItem() { return _lastItem; }

  signals:
    void resized(const QSize& newSize);
    void positionCenter(int pos);
    void adjustLimbsWeight(/*TODO: frameData*/);

  public slots:
    void setNumberOfFrames(int frames);
  protected slots:
    void showLimbsWeight();

  protected:
    int framesCount;
    /** current frame of playback */
    int frameSelected;
    QPixmap* offscreen;
    QAction* deleteAnimationAction;
    QAction* moveAnimationAction;
    QAction* limbWeightsAction;

    virtual void paintEvent(QPaintEvent* event);
    void contextMenuEvent(QContextMenuEvent *event);
    /** Finds first avaliable space for animation with @param frames frames.
        Returns foregoing TimelineItem* on success or throws QString if there is not enough space.
        Returning 0 indicates the space is available on beginning of a track. */
    TrailItem* FindFreeSpace(int frames);

    void drawBackground();
    void drawTrailItem(TrailItem* item);

  private:
    int _animationCount;
    TrailItem* _firstItem;
    TrailItem* _lastItem;

    /** Passes thorugh TimelineItems and updates their begin/end positions.
        Called usually after addition of new item */
    void updateTimelineItemsIndices();
};

#endif // TIMELINETRAIL_H

