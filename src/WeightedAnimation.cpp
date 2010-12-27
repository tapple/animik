#include "WeightedAnimation.h"
#include "bvh.h"


#include "time.h"   //DEBUG, ok?



WeightedAnimation::WeightedAnimation(BVH* newBVH,const QString& bvhFile)
  : Animation(newBVH, bvhFile)
{
  int frames_count = newBVH->lastLoadedNumberOfFrames;
  frameWeights = new int[frames_count];

  //DEBUG here
  for(int i=0; i<frames_count; i++)
  {
    clock_t ticks = clock();
    frameWeights[i] = ticks % frames_count;
  }
}

WeightedAnimation::~WeightedAnimation()     //edu: ~Animation() called automatically
{
  delete [] frameWeights;
}



int WeightedAnimation::getFrameWeight(int frameIndex)
{
  if(frameIndex >= totalFrames)
    throw "Argument exception: frameIndex exceeds frames count";
  return frameWeights[frameIndex];
}
