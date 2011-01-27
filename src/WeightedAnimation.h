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

protected:
  /** Array of weights, all must fall into <0, 100> */
  int* frameWeights;
};

#endif // WEIGHTEDANIMATION_H
