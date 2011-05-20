#ifndef TIMELINETRAIL_H
#define TIMELINETRAIL_H

#include <QFrame>
#include "WeightedAnimation.h"

class QPixmap;
class QAction;
class TrailItem;


/** One trail of blender timeline. Holds animations to be concatenated in result. */
class TimelineTrail : public QFrame
{
  Q_OBJECT

  public:
    TimelineTrail(QWidget* parent=0, Qt::WindowFlags f=0, QString debugName="<0>");
    ~TimelineTrail();

    /** Add new animation on first gap big enough. Returns TRUE on success
        or FALSE if there's not enough space on this trail. */
    bool AddAnimation(WeightedAnimation* anim, QString title);
    /** Clear content of this trail and set new with given first TrailItem */
    void ResetContent(TrailItem* first);
    int animationCount() const          { return _animationCount; }
    TrailItem* firstItem() const        { return _firstItem; }
    TrailItem* lastItem() const         { return _lastItem; }
    TrailItem* getSelectedItem() const  { return selectedItem; }
    int positionCount() const           { return positionsCount; }

    /** Frame position width in pixels. Some (parent) widget need to know
        before paint. */
    static int positionWidth()          { return _positionWidth; }
    void limitUserActions(bool limit);

  signals:
    void resized(const QSize& newSize);       //TODO: is this ever listened?
    /** Let other trails know current position changed so they synchronize */
    void currentPositionChanged(int newPositionIndex);
    /** Let other trails know current TrailItem changed so they unselect theirs */
    void selectedItemChanged();
    void backgroundClicked();
    /** Let other trails know this one changed it's size in frame positions
        so they do the same */
    void positionsCountChanged(int newCount);
    /*! Emited in a very special case that user has unselected the current item by changing
        time-line position with an arrow key. !*/
    void selectedItemLost();
    void adjustLimbsWeight(/*TODO: frameData*/);
    void movingItem(TrailItem* draggedItem);
    /** Announce done position change for an item so it's original holder
        can release/reposition it or new one can adopt it. */
    void droppedItem();


    /** Summary animation was changed */
//    void trailAnimationChanged(WeightedAnimation* animation, int firstFrame);         //TODO: should become obsolete soon

    /** A change on this trail was made requiring overall animation to be recalculated */
    void trailContentChanged(TrailItem* firstItem);



  public slots:
    void setCurrentPosition(int position);
    /** Unselect currently selected TrailItem. Note that this doesn't emit selectedItemChanged. */
    void CancelTrailSelection();
    void setPositionCount(int position);
    void onMovingItem(TrailItem* draggedItem);
    void onDroppedItem();

  protected slots:
    void deleteCurrentItem();
    void moveCurrentItem();
    void onFramesWeight();
    void showLimbsWeight();
    void setMixZones();

  protected:
    int positionsCount;
    /** current position (frame) of playback */
    int currentPosition;
    /** width of one frame position in pixels */
    static int _positionWidth;
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
    QAction* framesWeightAction;


    void clearShadowItems();
    void trailContentChange();


    virtual void paintEvent(QPaintEvent*);
    virtual void contextMenuEvent(QContextMenuEvent *event);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent *);
    /** Reimplemented for moving an item. Potential new location is "shadowed" */
    virtual void mouseMoveEvent(QMouseEvent* me);
    virtual void leaveEvent(QEvent*);
    virtual void keyPressEvent(QKeyEvent *);


  private:
    bool leftMouseDown;
    bool rightMouseDown;
    int _animationCount;
    /** TRUE when new item is being added. Means user send it to the timeline,
        but for some reason wasn't placed yet. */
    bool addingNewItem;
    bool userActionsLimited;
    bool needsRebuild;
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


    QString _debugName;
};

#endif // TIMELINETRAIL_H
