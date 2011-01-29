#include "WeightedAnimation.h"
#include "bvh.h"


#define MIX_IN_OUT        3


WeightedAnimation::WeightedAnimation(BVH* newBVH,const QString& bvhFile)
  : Animation(newBVH, bvhFile)
{
  int frames_count = newBVH->lastLoadedNumberOfFrames;

  if(frames_count < 2*MIX_IN_OUT)
    _mixIn = _mixOut = frames_count/2;
  else
    _mixIn = _mixOut = MIX_IN_OUT;

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
  if(frameIndex < 0 || frameIndex >= totalFrames)
  {
    throw new QString("Argument exception: frameIndex exceeds frames count");
  }
  return frameWeights[frameIndex];
}

void WeightedAnimation::setFrameWeight(int frameIndex, int weight)
{
  if(frameIndex < 0 || frameIndex >= totalFrames)
    throw new QString("Argument exception: frameIndex exceeds frames count");

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
