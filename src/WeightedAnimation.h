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
  bool isFirstFrameTPose() const  { return _tPosed; }
  int mixIn() const               { return _mixIn; }
  int mixOut() const              { return _mixOut; }
  void setMixIn(int mixIn)        { _mixIn = mixIn; }
  void setMixOut(int mixOut)      { _mixOut = mixOut; }

  /*! Position offset to align this animation when joining to another in process of blending. !*/
  Position getOffset() const      { return pOffset; }
  void setOffset(Position offset) { pOffset = offset; }

private:
  bool checkTPosed(BVHNode* limb);
  /** Array of weights, all must fall into <0, 100> */
  int* frameWeights;
  bool _tPosed;
  int _mixIn;
  int _mixOut;
  Position pOffset;
};

#endif // WEIGHTEDANIMATION_H
