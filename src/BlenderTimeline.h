#ifndef BLENDERTIMELINE_H
#define BLENDERTIMELINE_H

#include <QFrame>

#include "animation.h"



/** Wrapper for an Animation item on BlenderTimeline. TimelineItems should build
    a linked list order by first frame number. */
class TimelineItem
{
  public:
    TimelineItem(Animation* animation, int begin)
    {
      this->animation = animation;
      this->begin = begin;
    }

    int beginIndex() { return begin; }
    int endIndex() { return begin + frames() -1; }
    Animation* getAnimation() { return animation; }
    /** Number of frames of containing animation */
    int frames() { return animation->getNumberOfFrames(); }
    TimelineItem* nextItem() { return next; }
    void setNextItem(TimelineItem* nextItem) { this->next=nextItem; }

  private:
    int begin;
    Animation* animation;
    TimelineItem* next;
};



class QPixmap;
class QSize;


class BlenderTimeline : public QFrame
{
  Q_OBJECT

  public:
    BlenderTimeline(QWidget* parent=0, Qt::WindowFlags f=0);
//    ~BlenderTimeline();

    /** Add new animation to a track where fits first. Returns TRUE on success */
    bool AddAnimation(Animation* anim);
    int animationCount() { return _animationCount; }
    TimelineItem* firstItem() { return _firstItem; }
    TimelineItem* lastItem() { return _lastItem; }

  signals:
    void resized(const QSize& newSize);

  protected:
    QPixmap* offscreen;

    /** Finds first avaliable space (foregoing TimelineItem) for animation with
        @param frames frames. Return FALSE if there is not enough space */
    bool FindFreeSpace(int track, int frames, TimelineItem* outPredecessor);

    void drawBackground();
    void drawItem(TimelineItem* item);
    virtual void paintEvent(QPaintEvent* event);

  protected slots:
    void setNumberOfFrames(int frames);

  private:
    int framesCount;
    int _animationCount;
    TimelineItem* _firstItem;
    TimelineItem* _lastItem;
};


#endif // BLENDERTIMELINE_H
