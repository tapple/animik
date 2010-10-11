#ifndef BLENDERTIMELINE_H
#define BLENDERTIMELINE_H

#include <QFrame>

#include "animation.h"



/** Wrapper for an Animation item on BlenderTimeline. TimelineItems should build
    a linked list order by first frame number. */
class TimelineItem
{
  public:
    TimelineItem(Animation* animation, QString name, int begin)
    {
      this->name = name;
      this->animation = animation;
      this->begin = begin;
      previous = 0;
      next = 0;
    }

    int beginIndex() { return begin; }
    void shiftBeginIndex(int beginOffset) { begin += beginOffset; }
    int endIndex() { return begin + frames() -1; }
    Animation* getAnimation() { return animation; }
    /** Number of frames of containing animation */
    int frames() { return animation->getNumberOfFrames(); }
    TimelineItem* nextItem() { return next; }
    void setNextItem(TimelineItem* nextItem) { this->next=nextItem; }
    TimelineItem* previousItem() { return previous; }
    void setPreviousItem(TimelineItem* previousItem) { this->previous=previousItem; }

  private:
    QString name;
    int begin;
    Animation* animation;
    TimelineItem* next;
    TimelineItem* previous;
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
    bool AddAnimation(Animation* anim, QString title);
    int animationCount() { return _animationCount; }
    TimelineItem* firstItem() { return _firstItem; }
    TimelineItem* lastItem() { return _lastItem; }

  signals:
    void resized(const QSize& newSize);

  protected:
    QPixmap* offscreen;

    /** Finds first avaliable space for animation with @param frames frames.
        Returns foregoing TimelineItem* on success or throws QString if there is not enough space.
        Returning 0 indicates the space is available on beginning of a track. */
    TimelineItem* FindFreeSpace(int track, int frames);

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

    void updateTimelineItemsIndices();
};


#endif // BLENDERTIMELINE_H
