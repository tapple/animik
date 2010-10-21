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
    int animationCount() { return _animationCount; }
    TrailItem* firstItem() { return _firstItem; }
    TrailItem* lastItem() { return _lastItem; }

  signals:
    void resized(const QSize& newSize);
    void positionCenter(int pos);
    /** Let other trails know current frame changed so they synchronize */
    void currentFrameChanged(int newFrameIndex);
    /** Let other trails know current TrailItem changed so they unselect theirs */
    void selectedItemChanged();

  public slots:
    void setCurrentFrame(int frame);
    /** Unselect currently selected TrailItem */
    void cancelTrailSelection() { selectedItem=0; }
    void setNumberOfFrames(int frames);
  protected slots:
    void showLimbsWeight();

  protected:
    int framesCount;
    /** current frame of playback */
    int currentFrame;
    /** currently highlighted TrailItem */
    TrailItem* selectedItem;

    QPixmap* offscreen;
    QAction* deleteAnimationAction;
    QAction* moveAnimationAction;
    QAction* limbWeightsAction;

    virtual void paintEvent(QPaintEvent* event);
    virtual void contextMenuEvent(QContextMenuEvent *event);
    virtual void mousePressEvent(QMouseEvent* e);
    /** Finds first avaliable space for animation with @param frames frames.
        Returns foregoing TimelineItem* on success or throws QString if there is not enough space.
        Returning 0 indicates the space is available on beginning of a track. */
    TrailItem* FindFreeSpace(int frames);
    /** Find TrailItem that covers given frame */
    TrailItem* findItemOnFrame(int frameIndex);

    void drawBackground();
    void drawTrailItem(TrailItem* item);

  signals:
    void adjustLimbsWeight(/*TODO: frameData*/);

  protected slots:
    void deleteCurrentAnimation();
    void moveCurrentAnimation();

  private:
    bool leftMouseDown;
    bool rightMouseDown;
    int _animationCount;
    TrailItem* _firstItem;
    TrailItem* _lastItem;

    /** Passes thorugh TimelineItems and updates their begin/end positions.
        Called usually after addition of a new item. */
    void updateTimelineItemsIndices();
};

#endif // TIMELINETRAIL_H
