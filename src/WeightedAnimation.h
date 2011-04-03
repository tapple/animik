#ifndef WEIGHTEDANIMATION_H
#define WEIGHTEDANIMATION_H

#include "animation.h"
#include "QMap"

class QString;
class BVH;
class BVHNode;


class WeightedAnimation : public Animation
{
public:
  WeightedAnimation(BVH* newBVH, const QString& bvhFile);
  ~WeightedAnimation();

  virtual void setNumberOfFrames(int num);

  /*! Return skeleton of BVHNode 'bones' lined up to map. Key is node name (like 'lArm').
      Usefull when passing through skeleton to use iterative approach instead recursive.
      No particular order is granted. !*/
  QMap<QString, BVHNode*>* bones() const   { return linearBones; }
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
  void initializeLinearBonesHelper(BVHNode* bone);
  bool checkTPosed(BVHNode* limb);
  /** Array of weights, all must fall into <0, 100> */
  int* frameWeights;
  bool _tPosed;
  int _mixIn;
  int _mixOut;
  Position pOffset;
  QMap<QString, BVHNode*>* linearBones;
};

#endif // WEIGHTEDANIMATION_H
