#ifndef BLENDER_H
#define BLENDER_H


class QList;
class BVHNode;
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
    QList<TrailItem*> disassembleTimelineTrails(TrailItem** trails, int trailsCount);
  QList<TrailItem*> createMixInsImpliedShadowItems(QList<TrailItem*> items);
  QList<TrailItem*> createMixOutsImpliedShadowItems(QList<TrailItem*> items);
  void interpolatePosture(WeightedAnimation* anim1, int frame1, WeightedAnimation* anim2, int frame2,
                          WeightedAnimation* targetAnim);
  void interpolatePostureHelper(WeightedAnimation* anim1, int frame1, WeightedAnimation* anim2, int frame2,
                                int nodeIndex, WeightedAnimation* targetAnim);
  QList<TrailItem*> mergeAndSortItemsByBeginIndex(QList<TrailItem*> realItems,
                                                  QList<TrailItem*> mixInItems,
                                                  QList<TrailItem*> mixOutItems);
  void copyKeyFrames(WeightedAnimation* fromAnim, WeightedAnimation* toAnim);
  int findHighestEndIndex(QList<TrailItem*> items);
  void blend(QList<TrailItem*> sortedItems, WeightedAnimation* result);
  int findFirstItemAfter(QList<TrailItem*> sortedItems, int afterPosition);
};

#endif // BLENDER_H
