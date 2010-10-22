#ifndef TRAILITEM_C
#define TRAILITEM_C

#include "animation.h"


/** Wrapper for an Animation item on BlenderTimeline. TrailItems are supposed
    to build a linked list order by first frame number. */
class TrailItem
{
  public:
    TrailItem(Animation* animation, QString name, int begin)
    {
      this->name = name;
      this->animation = animation;
      this->begin = begin;
      selectedFrame = -1;
      previous = 0;
      next = 0;
    }

    int beginIndex() { return begin; }
    void shiftBeginIndex(int beginOffset) { begin += beginOffset; }
    int endIndex() { return begin + frames() -1; }
    Animation* getAnimation() { return animation; }
    /** Number of frames of containing animation */
    int frames() { return animation->getNumberOfFrames(); }
    /** Highlight @param frameIndex because it's became selected frame */
    void selectFrame(int frameIndex) { selectedFrame = frameIndex; }
    /** Gets weight of a frame to be used in blending */
    float frameWeight(int frameIndex) { return 0.5; }       //TODO: dynamically from frameData
    TrailItem* nextItem() { return next; }
    void setNextItem(TrailItem* nextItem) { this->next=nextItem; }
    TrailItem* previousItem() { return previous; }
    void setPreviousItem(TrailItem* previousItem) { this->previous=previousItem; }

  protected:
    QString name;
    int begin;
    /** Currently highlighted frame of animation. The value has meaning
        only if this item is also highlighted on a timeline. */
    int selectedFrame;
    Animation* animation;
    TrailItem* next;
    TrailItem* previous;
};

#endif // TRAILITEM_C
