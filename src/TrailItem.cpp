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
      this->animation = animation;
      if(isShadow)          //if is helper animation, remove it's mix-in/-out
      {
        this->animation->setMixIn(0);
        this->animation->setMixOut(0);
      }
      this->Name = name;
      this->begin = begin;
      _selectedFrame = -1;
      previous = 0;
      next = 0;
      this->shadow = isShadow;
    }

    QString Name;       //mainly for debug purposes so far

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
    /** Size of mix-in zone */
    int mixIn() { return animation->mixIn(); }
    void setMixIn(int mixIn) { animation->setMixIn(mixIn); }
    /** Size of mix-out zone */
    int mixOut() { return animation->mixOut(); }
    void setMixOut(int mixOut) { animation->setMixOut(mixOut); }
    /** Highlight @param frameIndex because it's became selected frame.
        GREAT CAUTION: this DOES NOT change the current frame of the underlying
        animation. It only highlights frame of this Item. */
    void selectFrame(int frameIndex)
    {
      if(frameIndex<0 || frameIndex>=animation->getNumberOfFrames())
        throw new QString("Argument exception: frame index out of range.");
      _selectedFrame = frameIndex;

//      animation->setFrame(frameIndex);        //FUJ! Troubles with AnimationView (::draw() executed to many times)
    }
    /** Currently highlighted frame of animation. The value has meaning
        only if this item is also highlighted on a timeline. */
    int selectedFrame() { return _selectedFrame; }            //TODO: no, no! The selected frame of underlying animation should be returned
    TrailItem* nextItem() { return next; }
    void setNextItem(TrailItem* nextItem) { this->next=nextItem; }
    TrailItem* previousItem() { return previous; }
    void setPreviousItem(TrailItem* previousItem) { this->previous=previousItem; }


    /** TRUE means this is helper animation made to fill gaps on a trail.
        Such item is not shown to the user in any way. */
    bool isShadow() { return shadow; }              //TODO: there is no place for this in final code.



  protected:
    int begin;
    int _selectedFrame;
    WeightedAnimation* animation;
    TrailItem* next;
    TrailItem* previous;
    bool shadow;
};

#endif // TRAILITEM_C
