#ifndef TRAILITEM_C
#define TRAILITEM_C

#include "WeightedAnimation.h"


/** Wrapper for an WeightedAnimation item on BlenderTimeline. TrailItems are supposed
    to build a linked list order by first frame number. */
class TrailItem
{
  public:
    TrailItem(WeightedAnimation* animation, QString name, int begin, bool isShadow)
    {
      this->name = name;
      this->animation = animation;
      this->begin = begin;
      _selectedFrame = -1;
      previous = 0;
      next = 0;
      this->shadow = isShadow;
    }

    int beginIndex() { return begin; }
    void setBeginIndex(int index) { begin=index; }
    //convenience method
    void shiftBeginIndex(int beginOffset) { begin += beginOffset; }
    int endIndex() { return begin + frames() -1; }
    WeightedAnimation* getAnimation() { return animation; }
    /** Get user-defined overall weight of a frame on given position.
        For blending purposes. */
    int getWeight(int frameIndex) {

//      try{      //DEBUG
        return animation->getFrameWeight(frameIndex);
/*      }
      catch(QString* ex){
        return -1;
      }   */

    }
    /** Number of frames of containing animation */
    int frames() { return animation->getNumberOfFrames(); }
    /** Highlight @param frameIndex because it's became selected frame */
    void selectFrame(int frameIndex)
    {
      if(frameIndex<0 || frameIndex>=animation->getNumberOfFrames())
        throw new QString("Argument exception: frame index out of range.");
      _selectedFrame = frameIndex;
    }
    /** Currently highlighted frame of animation. The value has meaning
        only if this item is also highlighted on a timeline. */
    int selectedFrame() { return _selectedFrame; }
    TrailItem* nextItem() { return next; }
    void setNextItem(TrailItem* nextItem) { this->next=nextItem; }
    TrailItem* previousItem() { return previous; }
    void setPreviousItem(TrailItem* previousItem) { this->previous=previousItem; }
    /** TRUE means this is helper animation made to fill gaps on a trail.
        Such item is not shown to the user in any way. */
    bool isShadow() { return shadow; }

  protected:
    QString name;
    int begin;
    int _selectedFrame;
    WeightedAnimation* animation;
    TrailItem* next;
    TrailItem* previous;
    bool shadow;
};

#endif // TRAILITEM_C
