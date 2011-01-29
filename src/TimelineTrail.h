#ifndef TIMELINETRAIL_H
#define TIMELINETRAIL_H

#include <QFrame>
#include "WeightedAnimation.h"

class QPixmap;
class QAction;
class TrailItem;


/** One trail of blender timeline. Emits signal with sum of all
    contained animations */
class TimelineTrail : public QFrame
{
  Q_OBJECT

  public:
    TimelineTrail(QWidget* parent=0, Qt::WindowFlags f=0);

    /** Add new animation on first gap big enough. Returns TRUE on success
        or FALSE if there's not enough space on this trail. */
    bool AddAnimation(WeightedAnimation* anim, QString title);
    int animationCount() { return _animationCount; }
    TrailItem* firstItem() { return _firstItem; }
    TrailItem* lastItem() { return _lastItem; }
    int positionCount() { return positionsCount; }

    /** Frame position width in pixels. Some (parent) widget need to know
        before paint. */
    int positionWidth() { return _positionWidth; }

  signals:
    void resized(const QSize& newSize);
    void positionCenter(int pos);
    /** Let other trails know current position changed so they synchronize */
    void currentPositionChanged(int newPositionIndex);
    /** Let other trails know current TrailItem changed so they unselect theirs */
    void selectedItemChanged();
    void backgroundClicked();
    /** Let other trails know this one changed it's size in frame positions
        so they do the same */
    void positionsCountChanged(int newCount);
    void adjustLimbsWeight(/*TODO: frameData*/);
    void movingItem(TrailItem* draggedItem);
    /** Announce done position change for an item so it's original holder
        can release/reposition it or new one can adopt it. */
    void droppedItem();
    /** Summary animation was changed */
    void trailAnimationChanged(WeightedAnimation* animation, int firstFrame);

  public slots:
    void setCurrentPosition(int position);
    /** Unselect currently selected TrailItem */
    void cancelTrailSelection() { selectedItem=0; repaint(); }
    void setPositionCount(int position);
    void onMovingItem(TrailItem* draggedItem);
    void onDroppedItem();
  protected slots:
    void deleteCurrentItem();
    void moveCurrentItem();
    void showLimbsWeight();
    void setMixZones();

  protected:
    int positionsCount;
    /** current position (frame) of playback */
    int currentPosition;
    /** width of one frame position in pixels */
    int _positionWidth;
    /** currently highlighted TrailItem */
    TrailItem* selectedItem;
    /** Not-null indicates that user is dragging an animation above timeline
        ready to drop it on new location, which as well may be this trail */
    TrailItem* draggingItem;
    /** TRUE means that weight for frame under currently selected position
        is being set by user. */
    bool settingWeight;
    /** An item is being repositioned and mouse is over this position. Else -1 */
    int draggingOverPosition;

    QPixmap* offscreen;
    QAction* deleteItemAction;
    QAction* moveItemAction;
    QAction* mixZonesAction;
    QAction* limbWeightsAction;

    /** After any change of position or number of animations at this trail,
        the resulting summary animation is gained with this method.
        Returns 0 if there's no item left. */
    WeightedAnimation* getSummaryAnimation();
    void clearShadowItems();
    void trailContentChange();


    virtual void paintEvent(QPaintEvent*);
    virtual void contextMenuEvent(QContextMenuEvent *event);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent *);
    /** Reimplemented for moving an item. Potential new location is "shadowed" */
    virtual void mouseMoveEvent(QMouseEvent* me);
    virtual void leaveEvent(QEvent*);


  private:
    bool leftMouseDown;
    bool rightMouseDown;
    int _animationCount;
    /** TRUE when new item is being added. Means user send it to the timeline,
        but for some reason wasn't placed yet. */
    bool addingNewItem;
    TrailItem* _firstItem;
    TrailItem* _lastItem;

    /** Finds first avaliable space for animation with @param frames frames.
        Returns foregoing TimelineItem* on success or throws QString if there is not enough space.
        Returning 0 indicates the space is available on beginning of a track. */
    TrailItem* findFreeSpace(int positions);
    bool isSuitableSpace(int beginPosition, int positionsCount);
    /** Find an item laying before given frame position */
    TrailItem* findPreviousItem(int beforePosition);
    /** Find an item laying after given position */
    TrailItem* findNextItem(int afterPosition);
    /** Find TrailItem that covers given position */
    TrailItem* findItemOnPosition(int positionIndex);
    /** Try to extend this trail with empty position space. Returns TRUE on success.
        FALSE means that extension would exceed maximum length allowed. */
    bool coerceExtension(int size);
    /** Delete given TrailItem from this trail and return it (to be copied
        to a 'clipboard') */
    TrailItem* cutItem(TrailItem*);
    /** Passes thorugh TimelineItems and updates their begin/end positions.
        Called usually after addition of a new item. */
    void updateTimelineItemsIndices();
    void cleanupAfterMove();
    void adjustFrameWeight(int cursorYPosition);

    void drawBackground();
    void drawMovedItemShadow();
    void drawTrailItem(TrailItem* item);
};

#endif // TIMELINETRAIL_H
