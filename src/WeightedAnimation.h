#ifndef WEIGHTEDANIMATION_H
#define WEIGHTEDANIMATION_H

#include "animation.h"

class QString;
class BVH;



class WeightedAnimation : public Animation
{
public:
  WeightedAnimation(BVH* newBVH, const QString& bvhFile);
  ~WeightedAnimation();

  int getFrameWeight(int frameIndex);
  int currentFrameWeight();
  void setCurrentFrameWeight(int weight);
  void setFrameWeight(int frameIndex, int weight);

  /** If TRUE, the first frame is key-frame and the avatar stands in T-pose (location doesn't matter) */
  bool isFirstFrameTPose() { return _tPosed; }
  int mixIn() { return _mixIn; }
  int mixOut() { return _mixOut; }
  void setMixIn(int mixIn) { _mixIn = mixIn; }
  void setMixOut(int mixOut) { _mixOut = mixOut; }

private:
  bool checkTPosed(BVHNode* limb);
  /** Array of weights, all must fall into <0, 100> */
  int* frameWeights;
  bool _tPosed;
  int _mixIn;
  int _mixOut;
};

#endif // WEIGHTEDANIMATION_H
