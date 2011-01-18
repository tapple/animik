#include "WeightedAnimation.h"
#include "bvh.h"



WeightedAnimation::WeightedAnimation(BVH* newBVH,const QString& bvhFile)
  : Animation(newBVH, bvhFile)
{
  int frames_count = newBVH->lastLoadedNumberOfFrames;
  frameWeights = new int[frames_count];

  for(int i=0; i<frames_count; i++)
    frameWeights[i] = 50;
}

WeightedAnimation::~WeightedAnimation()     //edu: ~Animation() called automatically
{
  delete [] frameWeights;
}



int WeightedAnimation::getFrameWeight(int frameIndex)
{
  if(frameIndex >= totalFrames)
  {
    throw new QString("Argument exception: frameIndex exceeds frames count");
  }
  return frameWeights[frameIndex];
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
