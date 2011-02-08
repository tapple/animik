#ifndef BLENDER_H
#define BLENDER_H


class QList;
class TrailItem;
class WeightedAnimation;



class Blender
{
public:
  Blender();
  ~Blender();

  /** Returns overall blended animation */
  WeightedAnimation* GetResultingAnimation();

  /** Blends together weighted animations of given TrailItems.
      @param trails array of pointers to first TrailItem in linked list.
      @param trailsCount number of pointers passed in the first argument. */
  WeightedAnimation* BlendTrails(TrailItem** trails, int trailsCount);

private:
  QList<TrailItem*> rearangeItemsByBeginIndex(TrailItem** trails, int trailsCount);

  QList<TrailItem*> disassembleTimelineTrails(TrailItem** trails, int trailsCount);
  QList<TrailItem*> createMixInsImpliedShadowItems(QList<TrailItem*> items);
  QList<TrailItem*> createMixOutsImpliedShadowItems(QList<TrailItem*> items);
};

#endif // BLENDER_H
