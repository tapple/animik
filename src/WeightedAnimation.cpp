#include "Announcer.h"
#include "bvh.h"
#include "WeightedAnimation.h"


#define MIX_IN_OUT        3


WeightedAnimation::WeightedAnimation(BVH* newBVH,const QString& bvhFile) : Animation(newBVH, bvhFile)
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

  linearBones = new QMap<QString, BVHNode*>();
  linearBones->insert(positionNode->name(), positionNode);
  initializeLinearBonesHelper(getMotion());
}

WeightedAnimation::~WeightedAnimation()     //edu: ~Animation() called automatically
{
  delete [] frameWeights;
}


void WeightedAnimation::setNumberOfFrames(int num)
{
  frameWeights = (int*) realloc(frameWeights, num * sizeof(int));
  Animation::setNumberOfFrames(num);
}

int WeightedAnimation::getFrameWeight(int frameIndex)
{
  if(frameIndex < 0 || frameIndex >= totalFrames)
  {
    Announcer::Exception(NULL, "(WeightedAnimation::getFrameWeight) Argument exception: frameIndex exceeds frames count");
    return -1;
//    throw new QString("(WeightedAnimation::getFrameWeight) Argument exception: frameIndex exceeds frames count");
  }
  return frameWeights[frameIndex];
}

void WeightedAnimation::setFrameWeight(int frameIndex, int weight)
{
  if(frameIndex < 0 || frameIndex >= totalFrames)
  {
    Announcer::Exception(NULL, "(WeightedAnimation::setFrameWeight) Argument exception: frameIndex exceeds frames count");
    return;
//    throw new QString("(WeightedAnimation::setFrameWeight) Argument exception: frameIndex exceeds frames count");
  }

  else if(weight<0 || weight>100)
  {
    Announcer::Exception(NULL, "Argument exception: frame weight out of range: " +
                         QString::number(weight)+ ". Must fall in <0; 100>");
    return;
//    throw new QString("Argument exception: frame weight out of range: " +QString::number(weight) + ". Must fall in <0; 100>");
  }
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


void WeightedAnimation::initializeLinearBonesHelper(BVHNode* bone)
{
  if(!linearBones->contains(bone->name()))
    linearBones->insert(bone->name(), bone);
  for(int i=0; i<bone->numChildren(); i++)
    initializeLinearBonesHelper(bone->child(i));
}


/** Resolve if first (key-)frame is T-pose */
bool WeightedAnimation::checkTPosed(BVHNode* limb)
{
  Rotation rot = limb->frameData(0).rotation();
  bool unchanged = rot.x==0 && rot.y==0 && rot.z==0;

  for(int i=0; i<limb->numChildren(); i++)
    unchanged = unchanged && checkTPosed(limb->child(i));

  return unchanged;
}
