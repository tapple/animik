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

  fillItemGaps();

  //NOTE: if in tempetation not to do the composition with only one Item, DON'T DO so! It'll cause nasty bug.
  WeightedAnimation* result = new WeightedAnimation(new BVH(), "");
  result->setNumberOfFrames(lastItem->endIndex()-firstItem->beginIndex() +1);

  //TODO: count frame weights here

  //copy rotations to the result
  BVHNode* root = firstItem->getAnimation()->getMotion();
  enhanceResultAnimation(result, root);

  return result;
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
    trais' animations to copy their counterpart key frames there */
void TrailJoiner::enhanceResultAnimation(WeightedAnimation* destAnim, BVHNode* node)
{
  int limbIndex = destAnim/*_firstItem->getAnimation()*/->getPartIndex(node);
  appendNodeKeyFrames(destAnim, limbIndex);
  for(int i=0; i<node->numChildren(); i++)
    enhanceResultAnimation(destAnim, node->child(i));
}


/** Get all keyframes of given limb of all animations on this trail
    (in time order) and append them to given destination animation */
void TrailJoiner::appendNodeKeyFrames(WeightedAnimation* destAnim, int nodeIndex)
{
  TrailItem* currentItem = firstItem;//->nextItem();
  int frameOffset = 0;//_firstItem->frames();

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

