#include "WeightedAnimation.h"
#include "bvh.h"


#include "time.h"   //DEBUG, ok?



WeightedAnimation::WeightedAnimation(BVH* newBVH,const QString& bvhFile)
  : Animation(newBVH, bvhFile)
{
  int frames_count = newBVH->lastLoadedNumberOfFrames;
  frameWeights = new int[frames_count];

  //DEBUG here
  srand(time(NULL));
  for(int i=0; i<frames_count; i++)
  {
    frameWeights[i] = rand() % 101;
  }
}

WeightedAnimation::~WeightedAnimation()     //edu: ~Animation() called automatically
{
  delete [] frameWeights;
}



int WeightedAnimation::getFrameWeight(int frameIndex)
{
  if(frameIndex >= totalFrames)
    throw new QString("Argument exception: frameIndex exceeds frames count");
  return frameWeights[frameIndex];
}
