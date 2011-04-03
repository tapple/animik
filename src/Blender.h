#ifndef BLENDER_H
#define BLENDER_H

#include <QList>
#include "rotation.h"

class BVHNode;
class TimelineTrail;
class TrailItem;
class WeightedAnimation;



class Blender
{
public:
  Blender();
  ~Blender();

  /** Returns overall blended animation */
  WeightedAnimation* GetResultingAnimation();           //edu: pfuuh, rubish

  /*! Passes through time-line and for every item, frame and limb evaluates its relative weight compared
      to limb weights of frame it'll blend with. This method must be called befor BlendTrails. !*/              //TODO: it should be called as first in BlendTrails
  void EvaluateRelativeLimbWeights(QList<TimelineTrail*>* trails, int trailsCount);

  /*! Blends together weighted animations of given TrailItems.
      @param trails array of pointers to first TrailItem in linked list.
      @param trailsCount number of pointers passed in the first argument. !*/
  WeightedAnimation* BlendTrails(TrailItem** trails, int trailsCount);

private:
  QList<TrailItem*> lineUpTimelineTrails(TrailItem** trails, int trailsCount);
  void clearShadowItems(TrailItem* firstItem);
  QList<TrailItem*> createMixInsImpliedShadowItems(QList<TrailItem*> items);
  QList<TrailItem*> createMixOutsImpliedShadowItems(QList<TrailItem*> items);
  void interpolatePosture(WeightedAnimation* anim1, int frame1, WeightedAnimation* anim2, int frame2,
                          WeightedAnimation* targetAnim);
  void interpolatePostureHelper(WeightedAnimation* anim1, int frame1, WeightedAnimation* anim2, int frame2,
                                int nodeIndex, WeightedAnimation* targetAnim);
  QList<TrailItem*> mergeAndSortItemsByBeginIndex(QList<TrailItem*> realItems,
                                                  QList<TrailItem*> mixInItems,
                                                  QList<TrailItem*> mixOutItems);
  void cloneAnimation(WeightedAnimation* fromAnim, WeightedAnimation* toAnim);
  void cloneAnimationHelper(int limbIndex, WeightedAnimation* fromAnim, WeightedAnimation * toAnim);

  int findHighestEndIndex(QList<TrailItem*> items);
  void blend(QList<TrailItem*> sortedItems, WeightedAnimation* result, int lastFrameIndex);

  void combineKeyFrames(QList<TrailItem*> sortedItems, QList<int> itemIndices, int fromTimeLineFrame,
                        int sectionLength, WeightedAnimation* target, int targetFrame);
  void combineKeyFramesHelper(QList<TrailItem*> sortedItems, QList<int> itemIndices, BVHNode* limb,
                              int timeLineFrame, WeightedAnimation* target, int targetFrame);

  int findFirstItemAfter(QList<TrailItem*> sortedItems, int afterPosition);
  TrailItem* findLastItemBefore(QList<TrailItem*> sortedItems, int beforePosition);
  void copyKeyFrame(BVHNode* limb, int fromFrame, int toFrame);

  double static absolut(double val) { return val>0.0 ? val : (-1 * val); }
};

#endif // BLENDER_H
