#include "WeightedAnimation.h"
#include "bvh.h"


#define MIX_IN_OUT        3


WeightedAnimation::WeightedAnimation(BVH* newBVH,const QString& bvhFile)
  : Animation(newBVH, bvhFile)
{
  int frames_count = newBVH->lastLoadedNumberOfFrames;
  _tPosed = checkTPosed(frames);

  if(frames_count < 2*MIX_IN_OUT)
    _mixIn = _mixOut = frames_count/2;
  else
    _mixIn = _mixOut = MIX_IN_OUT;

  frameWeights = new int[frames_count];
  for(int i=0; i<frames_count; i++)
    frameWeights[i] = 50;
}


/** Learn if first (key-)frame is T-pose */
bool WeightedAnimation::checkTPosed(BVHNode* limb)
{
  Rotation rot = limb->frameData(0).rotation();
  bool unchanged = rot.x==0 && rot.y==0 && rot.z==0;

  for(int i=0; i<limb->numChildren(); i++)
    unchanged = unchanged && checkTPosed(limb->child(i));

  return unchanged;
}

WeightedAnimation::~WeightedAnimation()     //edu: ~Animation() called automatically
{
  delete [] frameWeights;
}



int WeightedAnimation::getFrameWeight(int frameIndex)
{
  if(frameIndex < 0 || frameIndex >= totalFrames)
  {
    throw new QString("(WeightedAnimation::getFrameWeight) Argument exception: frameIndex exceeds frames count");
  }
  return frameWeights[frameIndex];
}

void WeightedAnimation::setFrameWeight(int frameIndex, int weight)
{
  if(frameIndex < 0 || frameIndex >= totalFrames)
    throw new QString("(WeightedAnimation::setFrameWeight) Argument exception: frameIndex exceeds frames count");

  else if(weight<0 || weight>100)
    throw new QString("Argument exception: frame weight out of range: " +
                      QString::number(weight) + ". Must fall in <0; 100>");
  else
    frameWeights[frameIndex] = weight;
}

int WeightedAnimation::currentFrameWeight()
{
  return frameWeights[frame];
}

void WeightedAnimation::setCurrentFrameWeight(int weight)
{
  if(weight<0 || weight>100)
    throw new QString("Argument exception: frame weight out of range: " +
                      QString::number(weight) + ". Must fall in <0; 100>");
  else
    frameWeights[frame] = weight;
}
