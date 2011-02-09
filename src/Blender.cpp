/*
  Offers computations related to blending of multiple animations.
*/

#include <QList>

#include "Blender.h"
#include "bvh.h"
#include "TrailItem.cpp"
#include "WeightedAnimation.h"


Blender::Blender()
{

}



WeightedAnimation* Blender::BlendTrails(TrailItem** trails, int trailsCount)
{
  int beginIndex = 999999999;
  int endIndex = -999999999;

  for(int i=0; i<trailsCount; i++)
  {
    if(trails[i]->beginIndex() < beginIndex)
      beginIndex = trails[i]->beginIndex();

    if(trails[i]->endIndex() > endIndex)
      endIndex = trails[i]->endIndex();
  }

  int beginOffset = beginIndex;     //number of empty frame positions before the result animation
  int newFramesCont = endIndex-beginIndex;
  //TODO...



}


/** Returns list of TrailItems such that its size is equal to overall TrailItems number (one Item
    at one list position, means original linked lists are broken). */
QList<TrailItem*> Blender::disassembleTimelineTrails(TrailItem** trails, int trailsCount)
{
  QList<TrailItem*> result;

  for(int trail=0; trail<trailsCount; trail++)
  {
    TrailItem* currentItem = trails[trail];   //take first item in current trail

    while(currentItem!=0)       //disconnect it from original linked list
    {
      TrailItem* toBeNext = currentItem->nextItem();

      if(currentItem->nextItem()!=0)                        //edu:
        currentItem->nextItem()->setPreviousItem(0);        //actually this shouldn't
      currentItem->setNextItem(0);                          //be needed at all

      result.append(currentItem);
      currentItem = toBeNext;
    }
  }
}


/** Returns list of new artificial 'shadow' TrailItems. These are created to accomplish the mix-in
    functionality of two overlaping animations (the former is "blended in" the latter one). The shadow
    helpers will have lineary increasing frame weights to achieve gradual in-blending.
    They are ready to be blended into overall animation. */
QList<TrailItem*> Blender::createMixInsImpliedShadowItems(QList<TrailItem*> items)
{
  if(items.size() < 2)
    throw new QString("Argument exception: there must be at least two items in the list 'sortedItems'");

  QList<TrailItem*> result;

  for(int item=1; item<items.size(); item++)
  {
    for(int i=0; i<item; i++)       //for all previous items: resolve if crossing current item
    {                               //implies creating mix-in shadow item
      TrailItem* currentItem = items[item];

      if(currentItem->endIndex() <= items[i]->endIndex())
        continue;         //wrong overlap order

      //overlaping or touching
      if(items[i]->endIndex()+1 >= currentItem->beginIndex() && items[i]->mixIn() > 0)
      {
        int beginsDiff = currentItem->beginIndex() - items[i]->beginIndex();
        int framesNum = items[i]->mixIn() > beginsDiff ? beginsDiff
                                                             : items[i]->mixIn();

        WeightedAnimation* mixInShadow = new WeightedAnimation(new BVH(), "");
        mixInShadow->setNumberOfFrames(framesNum);

        //adjust shadow skeleton posture
        int crossPoint = items[i]->frames() - (items[i]->endIndex() - currentItem->beginIndex());
        interpolatePosture(items[i]->getAnimation(), crossPoint, currentItem->getAnimation(), 0, mixInShadow);

        //set shadows frame weights (going backwards from last frame to first)
        for(int n=framesNum-1; n>=0; n--)
        {
          int value = (n+1 + (items[i]->mixIn() - framesNum) ) / items[i]->mixIn();
          mixInShadow->setFrameWeight(n, value);
        }

        TrailItem* shadowItem = new TrailItem(mixInShadow, "shadow",
                                              currentItem->beginIndex()-framesNum, true);
        result.append(shadowItem);
      }
      //Gap between end of previous and begin of current. But mix-in of the previous is bigger than the gap
      else if(items[i]->mixIn() > (currentItem->beginIndex() - items[i]->endIndex() - 1))
      {
        int gap = currentItem->beginIndex() - items[i]->endIndex() - 1;
        int framesNum = items[i]->mixIn() - gap;

        WeightedAnimation* mixInShadow = new WeightedAnimation(new BVH(), "");
        mixInShadow->setNumberOfFrames(framesNum);

        //adjust shadow skeleton posture
        interpolatePosture(items[i]->getAnimation(), items[i]->frames()-1, currentItem->getAnimation(),
                           0, mixInShadow);


        for(int n=0; n<framesNum; n++)
        {
          int value = (n+1) / framesNum;
          mixInShadow->setFrameWeight(n, value);
        }

        TrailItem* shadowItem = new TrailItem(mixInShadow, "shadow",
                                              items[i]->endIndex()-framesNum+1, true);
        result.append(shadowItem);
      }
      //else gap is too big.
    }
  }

  return result;
}


QList<TrailItem*> Blender::createMixOutsImpliedShadowItems(QList<TrailItem*> items)
{
  if(items.size() < 2)
    throw new QString("Argument exception: there must be at least two items in the list 'sortedItems'");

  QList<TrailItem*> result;

  for(int item=1; item<items.size(); item++)
  {
    for(int i=0; i<item; i++)
    {
      TrailItem* currentItem = items[item];

      if(currentItem->endIndex() <= items[i]->endIndex())
        continue;   //wrong overlap order

      //overlapping or touching
      if(items[i]->endIndex()+1 >= currentItem->beginIndex() && currentItem->mixOut() > 0)
      {
        int endsDiff = currentItem->endIndex() - items[i]->endIndex();
        int framesNum = currentItem->mixOut() > endsDiff ? endsDiff
                                                         : currentItem->mixOut();

        WeightedAnimation* mixInShadow = new WeightedAnimation(new BVH(), "");
        mixInShadow->setNumberOfFrames(framesNum);

        //adjust shadow skeleton posture
        int crossPoint = items[i]->endIndex() - currentItem->beginIndex() - 1;
        interpolatePosture(items[i]->getAnimation(), items[i]->frames()-1, currentItem->getAnimation(),
                           crossPoint, mixInShadow);

        for(int n=0; n<framesNum; n++)
        {
          int value = (currentItem->mixOut()-n) / currentItem->mixOut();
          mixInShadow->setFrameWeight(n, value);
        }

        TrailItem* shadowItem = new TrailItem(mixInShadow, "shadow",
                                              items[i]->endIndex()+1, true);
        result.append(shadowItem);
      }
      //Gap smaller than mix-out
      else if(currentItem->mixOut() > (currentItem->beginIndex() - items[i]->endIndex() - 1))
      {
        int gap = currentItem->beginIndex()-items[i]->endIndex()-1;
        int framesNum = currentItem->mixOut() - gap;

        WeightedAnimation* mixInShadow = new WeightedAnimation(new BVH(), "");
        mixInShadow->setNumberOfFrames(framesNum);

        //adjust shadow skeleton posture
        interpolatePosture(items[i]->getAnimation(), items[i]->frames()-1, currentItem->getAnimation(),
                           0, mixInShadow);

        for(int n=1; n<=framesNum; n++)
        {
          int value = n / currentItem->mixOut();
          mixInShadow->setFrameWeight(framesNum-n, value);
        }

        TrailItem* shadowItem = new TrailItem(mixInShadow, "shadow",
                                              currentItem->beginIndex(), true);
        result.append(shadowItem);
      }
      //else gap is too big
    }
  }

  return result;
}


/** Create an BVH frame data for an animation. The animation will have only one posture (initial
    key frame) made as linear interpolation of two given frames.
    Frame weights are not considered in interpolation. */
void Blender::interpolatePosture(WeightedAnimation* anim1, int frame1,
                                 WeightedAnimation* anim2, int frame2,
                                 WeightedAnimation* targetAnim)     //TODO: no LERP for position, rather Kovar's approach
{
  interpolatePostureHelper(anim1, frame1, anim2, frame2, 0, targetAnim);      //position pseudo-node first

  int rootIndex1 = anim1->getPartIndex(anim1->getMotion());
  interpolatePostureHelper(anim1, frame1, anim2, frame2, rootIndex1, targetAnim);
}


void Blender::interpolatePostureHelper(WeightedAnimation* anim1, int frame1,
                                       WeightedAnimation* anim2, int frame2,
                                       int nodeIndex, WeightedAnimation* targetAnim)
{
  BVHNode* node1 = anim1->getNode(nodeIndex);
  FrameData data1 = node1->frameData(frame1);
  FrameData data2 = anim2->getNode(nodeIndex)->frameData(frame2);
  Position newPos( (data1.position().x + data2.position().x) / 2,
                   (data1.position().y + data2.position().y) / 2,
                   (data1.position().z + data2.position().z) / 2);
  Rotation newRot( (data1.rotation().x + data2.rotation().x) / 2,
                   (data1.rotation().y + data2.rotation().y) / 2,
                   (data1.rotation().z + data2.rotation().z) / 2);

  targetAnim->getNode(nodeIndex)->addKeyframe(0, newPos, newRot);

  for(int i=0; i<node1->numChildren(); i++)
  {
    int limbIndex = anim1->getPartIndex(node1->child(i));
    interpolatePostureHelper(anim1, frame1, anim2, frame2, limbIndex, targetAnim);
  }
}



/** Returns list of TrailItems such that:
    1.) its size is sum of overall number of TrailItems created by user (one Item at list position, original
        linked list is broken) + automatically created mix-in/mix-out helper items
    2.) it is sorted by first frame-position index (lowest index on list's position 0) */
QList<TrailItem*> Blender::mergeAndSortItemsByBeginIndex(QList<TrailItem*> realItems,
                                                         QList<TrailItem*> mixInItems,
                                                         QList<TrailItem*> mixOutItems)
{
  QList<TrailItem*> result;

  while(true)
  {
    int lowestIndex = 999999999;             //begin index of left-most item
    QList<TrailItem*>* lowestItemList = 0;   //list that contains the item

    for(int r=0; r<realItems.size(); r++)
    {
      if(realItems.at(r)->beginIndex() < lowestIndex)
      {
        lowestItemList = &realItems;
        lowestIndex = realItems.at(r)->beginIndex();
      }
    }
    for(int i=0; i<mixInItems.size(); i++)             //BLEH! code repetition. TODO: think of it
    {
      if(mixInItems.at(i)->beginIndex() < lowestIndex)
      {
        lowestItemList = &mixInItems;
        lowestIndex = mixInItems.at(i)->beginIndex();
      }
    }
    for(int o=0; o<mixOutItems.size(); o++)             //here as well
    {
      if(mixOutItems.at(o)->beginIndex() < lowestIndex)
      {
        lowestItemList = &mixOutItems;
        lowestIndex = mixOutItems.at(o)->beginIndex();
      }
    }

    if(lowestItemList != 0)
    {
      result.append(lowestItemList->at(lowestIndex));
      lowestItemList->removeAt(lowestIndex);
    }
    else break;
  }

  return result;
}


/** Performs the actual blending.
    All animations are blended together depending on their placement on time-line and weight parameters.
    Gaps between items are filled.
    @param sortedItems - unlinked TrailItems sorted by their begin index (from time-line) */
WeightedAnimation* Blender::blend(QList<TrailItem*> sortedItems)
{
  //TODO
}
