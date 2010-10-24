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
    int frameCount() { return framesCount; }

    /** Frame width in pixels. Some (parent) widget need to know before paint. */
    int frameWidth() { return _frameWidth; }

  signals:
    void resized(const QSize& newSize);
    void positionCenter(int pos);
    /** Let other trails know current frame changed so they synchronize */
    void currentFrameChanged(int newFrameIndex);
    /** Let other trails know current TrailItem changed so they unselect theirs */
    void selectedItemChanged();
    void backgroundClicked();
    /** Let other trails know this one changed it's size in frames so they do the same */
    void framesCountChanged(int newCount);
    void adjustLimbsWeight(/*TODO: frameData*/);
    void movingItem(TrailItem* draggedItem);
    /** Announce done position change for an item so it's original holder
        can release/reposition it or new one can adopt it. */
    void droppedItem();

  public slots:
    void setCurrentFrame(int frame);
    /** Unselect currently selected TrailItem */
    void cancelTrailSelection() { selectedItem=0; repaint(); }
    void setFrameCount(int frames);
    void onMovingItem(TrailItem* draggedItem);
    void onDroppedItem();
  protected slots:
    void deleteCurrentItem();
    void moveCurrentItem();
    void showLimbsWeight();

  protected:
    int framesCount;
    /** current frame of playback */
    int currentFrame;
    /** width of one frame in pixels */
    int _frameWidth;
    /** currently highlighted TrailItem */
    TrailItem* selectedItem;
    /** Not-null indicates that user is dragging an animation above timeline
        ready to drop it on new location, which as well may be this trail */
    TrailItem* draggingItem;
    /** An item is being repositioned and mouse is over this frame. Else -1 */
    int draggingOverFrame;

    QPixmap* offscreen;
    QAction* deleteItemAction;
    QAction* moveItemAction;
    QAction* limbWeightsAction;

    virtual void paintEvent(QPaintEvent*);
    virtual void contextMenuEvent(QContextMenuEvent *event);
    virtual void mousePressEvent(QMouseEvent* e);
    /** Reimplemented for moving an item. Potential new location is "shadowed" */
    virtual void mouseMoveEvent(QMouseEvent* me);
    virtual void leaveEvent(QEvent*);
    /** Finds first avaliable space for animation with @param frames frames.
        Returns foregoing TimelineItem* on success or throws QString if there is not enough space.
        Returning 0 indicates the space is available on beginning of a track. */
    TrailItem* FindFreeSpace(int frames);
    bool isSuitableSpace(int beginFrame, int framesCount);
    /** Find an item laying before given frame */
    TrailItem* findPreviousItem(int beforeFrame);
    /** Find an item laying after given frame */
    TrailItem* findNextItem(int afterFrame);
    /** Find TrailItem that covers given frame */
    TrailItem* findItemOnFrame(int frameIndex);
    /** Try to extend this trail with empty frame space. Returns TRUE on success. FALSE means
        that extension would exceed maximum length allowed. */
    bool coerceExtension(int size);
    TrailItem* cutCurrentItem();

    void drawBackground();
    void drawMovedItemShadow();
    void drawTrailItem(TrailItem* item);

  private:
    bool leftMouseDown;
    bool rightMouseDown;
    int _animationCount;
    TrailItem* _firstItem;
    TrailItem* _lastItem;

    /** Passes thorugh TimelineItems and updates their begin/end positions.
        Called usually after addition of a new item. */
    void updateTimelineItemsIndices();
    void cleanupAfterMove();
};

#endif // TIMELINETRAIL_H
