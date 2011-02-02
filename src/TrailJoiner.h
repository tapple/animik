#ifndef TRAILJOINER_H
#define TRAILJOINER_H


class BVHNode;
class TrailItem;
class WeightedAnimation;



/** The class to perform animation composition on linear basis (on timeline trail) */
class TrailJoiner
{
public:
  TrailJoiner(TrailItem* firstItem);

  /** The core method, joins weighted animations of one TimelineTrail */
  WeightedAnimation* GetJoinedAnimation();


private:
  TrailItem* firstItem;
  TrailItem* lastItem;

  void fixLastKeyFrames();
  void fixLastKeyFramesHelper(BVHNode* limb, int lastFrameIndex);
  void fillItemGaps();
  void enhanceResultAnimation(WeightedAnimation* destAnim, BVHNode* node);
  void appendNodeKeyFrames(WeightedAnimation* destAnim, int nodeIndex);
};

#endif // TRAILJOINER_H
