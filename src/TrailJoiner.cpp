#include <QList>
#include "TrailJoiner.h"
#include "bvh.h"
#include "TrailItem.cpp"
#include "WeightedAnimation.h"




TrailJoiner::TrailJoiner(TrailItem *firstItem)
{
  this->firstItem = firstItem;

  TrailItem* currentItem = firstItem;
  while(currentItem->nextItem() != 0)
    currentItem = currentItem->nextItem();

  lastItem = currentItem;
}


WeightedAnimation* TrailJoiner::GetJoinedAnimation()
{
  if(firstItem==0)           //no animations left at this trail
    return 0;

  fixLastKeyFrames();
  fillItemGaps();

  //NOTE: if in tempetation not to do the composition with only one Item, DON'T DO so! It'll cause nasty bug.
  WeightedAnimation* result = new WeightedAnimation(new BVH(), "");
  result->setNumberOfFrames(lastItem->endIndex()-firstItem->beginIndex() +1);

  //TODO: count frame weights here

  //copy rotations to the result
  BVHNode* root = firstItem->getAnimation()->getMotion();
  enhanceResultAnimation(result, root);
  appendNodeKeyFrames(result, 0);       //finally, the position pseudo-node

  return result;
}


/** Make last frame of all animations on a trail a key-frame, so they'll keep their original
    motion (given in KeyFramerTab) intact. Otherwise there would occur an interpolation from (old) last
    key-frame to the first one of following animation, which will surely confuse user a lot. */
void TrailJoiner::fixLastKeyFrames()
{
  TrailItem* currentItem = firstItem;

  while(currentItem->nextItem() != 0)   //no need to do it for last one
  {
    int framesCount = currentItem->getAnimation()->getNumberOfFrames();

    BVHNode* position = currentItem->getAnimation()->getNode(0);
    fixLastKeyFramesHelper(position, framesCount-1);

    BVHNode* root = currentItem->getAnimation()->getMotion();
    fixLastKeyFramesHelper(root, framesCount-1);

    currentItem = currentItem->nextItem();
  }
}

void TrailJoiner::fixLastKeyFramesHelper(BVHNode* limb, int lastFrameIndex)
{
  if(!limb->isKeyframe(lastFrameIndex))
  {
    int oldLastKeyFrameIndex = limb->getKeyframeNumberBefore(lastFrameIndex);
    Position pos = limb->frameData(oldLastKeyFrameIndex).position();
    Rotation rot = limb->frameData(oldLastKeyFrameIndex).rotation();
    limb->addKeyframe(lastFrameIndex, pos, rot);
  }

  for(int i=0; i<limb->numChildren(); i++)
    fixLastKeyFramesHelper(limb->child(i), lastFrameIndex);
}


/** First sub-step in getting overall result. Gaps on this trail are filled
    with auxiliary ('shadow') animation wrappers. */
void TrailJoiner::fillItemGaps()
{
  TrailItem* currentItem = firstItem;

  while(currentItem->nextItem() != 0)
  {
    int gap = currentItem->nextItem()->beginIndex() - 1 - currentItem->endIndex();
    if(gap>0)  //aren't side-by-side
    {
      WeightedAnimation* gapFillAnim = new WeightedAnimation(new BVH(), "");
      gapFillAnim->setNumberOfFrames(gap);

      TrailItem* gapFillItem = new TrailItem(gapFillAnim, "shadow", currentItem->endIndex()+1, true);
      gapFillItem->setPreviousItem(currentItem);
      gapFillItem->setNextItem(currentItem->nextItem());
      currentItem->setNextItem(gapFillItem);
      gapFillItem->nextItem()->setPreviousItem(gapFillItem);
    }

    currentItem = currentItem->nextItem();
  }
}


/** Traverses through skeleton tree of to-be result animation and asks
    trails' animations to copy their counterpart key frames there */
void TrailJoiner::enhanceResultAnimation(WeightedAnimation* destAnim, BVHNode* node)
{
  int limbIndex = destAnim->getPartIndex(node);
  appendNodeKeyFrames(destAnim, limbIndex);
  for(int i=0; i<node->numChildren(); i++)
    enhanceResultAnimation(destAnim, node->child(i));
}


/** Get all keyframes of given limb of all animations on this trail
    (in time order) and append them to given destination animation */
void TrailJoiner::appendNodeKeyFrames(WeightedAnimation* destAnim, int nodeIndex)
{
  TrailItem* currentItem = firstItem;
  int frameOffset = 0;

  while(currentItem!=0)
  {
    BVHNode* limb = currentItem->getAnimation()->getNode(nodeIndex);
    QList<int> indices = limb->keyframeList();
    int count = indices.count();
    for(int i=0; i<count; i++)
    {
      FrameData oldData = limb->frameData(indices[i]);
      destAnim->getNode(nodeIndex)->addKeyframe(indices[i]+frameOffset, oldData.position(), oldData.rotation());
    }
    frameOffset += currentItem->frames();
    currentItem = currentItem->nextItem();
  }
}

