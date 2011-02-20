/*
  Offers computations related to blending of multiple animations.
*/

#include "Blender.h"
#include "bvh.h"
#include "settings.h"
#include "TrailItem.cpp"
#include "WeightedAnimation.h"


Blender::Blender() { }



WeightedAnimation* Blender::BlendTrails(TrailItem** trails, int trailsCount)
{
  QList<TrailItem*> items;
  QList<TrailItem*> origItems = lineUpTimelineTrails(trails, trailsCount);

  if(origItems.size() == 1)       //only one animation
    items = origItems;
  else
  {
    QList<TrailItem*> mixIns = createMixInsImpliedShadowItems(origItems);
    QList<TrailItem*> mixOuts = createMixOutsImpliedShadowItems(origItems);
    items = mergeAndSortItemsByBeginIndex(origItems, mixIns, mixOuts);
  }

  int endIndex = findHighestEndIndex(items);
  int newFramesCont = endIndex - items.at(0)->beginIndex() + 1;
  WeightedAnimation* result = new WeightedAnimation(new BVH(), "");
  result->setNumberOfFrames(newFramesCont);

  if(origItems.size() == 1)                 //for a single animation I MUST do it like this.
    cloneAnimation(origItems.at(0)->getAnimation(), result);     //Otherwise signal/slot apocalypse begins
  else
    blend(items, result, endIndex);

  return result;
}


/** Returns list of TrailItems such that its size is equal to overall TrailItems number (one Item
    at one list position, original lists are merged to one array).
    Original linked list references are preserved. Also, possible shadow items are cleared. */
QList<TrailItem*> Blender::lineUpTimelineTrails(TrailItem** trails, int trailsCount)
{
  QList<TrailItem*> result;

  for(int trail=0; trail<trailsCount; trail++)
  {
    TrailItem* currentItem = trails[trail];   //take first item in current trail
    clearShadowItems(currentItem);            //lose all previous helper items

    while(currentItem!=0)
    {
      result.append(currentItem);
      currentItem = currentItem->nextItem();
    }
  }

  return result;
}


void Blender::clearShadowItems(TrailItem* firstItem)
{
  if(firstItem==NULL)
    return;

  TrailItem* currentItem = firstItem;
  while(currentItem != NULL)
  {
    if(currentItem->isShadow())
    {
      if(currentItem->previousItem() != NULL)
        currentItem->previousItem()->setNextItem(currentItem->nextItem());
      if(currentItem->nextItem() != NULL)
        currentItem->nextItem()->setPreviousItem(currentItem->previousItem());

      delete currentItem;
    }

    currentItem = currentItem->nextItem();
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

  for(int item=0; item<items.size(); item++)
  {
    for(int i=0; i<items.size(); i++)       //for all previous items: resolve if crossing current item
    {                                       //implies creating mix-in shadow item
      TrailItem* currentItem = items[item];

      if(i==item)                                             //won't combine with self
        continue;
      if(currentItem->beginIndex() <= items[i]->beginIndex())     //wrong overlap order
        continue;

      //overlaping or touching
      if(items[i]->endIndex()+1 >= currentItem->beginIndex() && items[i]->mixIn() > 0)
      {
        int beginsDiff = currentItem->beginIndex() - items[i]->beginIndex();
        int framesNum = items[i]->mixIn() > beginsDiff ? beginsDiff
                                                       : items[i]->mixIn();

        WeightedAnimation* mixInShadow = new WeightedAnimation(new BVH(), "");
        mixInShadow->setNumberOfFrames(framesNum);

        //adjust shadow skeleton posture
        int crossPoint = items[i]->frames() - (items[i]->endIndex() - currentItem->beginIndex());                 //TODO: possible BUG. What if they just touch?
        interpolatePosture(items[i]->getAnimation(), crossPoint, currentItem->getAnimation(), 0,
                           mixInShadow);

        //set shadows frame weights (going backwards from last frame to first)
        for(int n=framesNum-1; n>=0; n--)
        {
          double value = (double)(n+1 + (items[i]->mixIn() - framesNum) ) / (double)items[i]->mixIn();
          mixInShadow->setFrameWeight(n, (int)(value*100));
        }

        TrailItem* shadowItem = new TrailItem(mixInShadow, "(1)mix in shadow for "+currentItem->Name,
                                              currentItem->beginIndex()-framesNum, true);
        result.append(shadowItem);

        //When in user invoked DEBUG mode, shadow items are embeded in time-line, so user can check them visualy
        if(Settings::Instance()->Debug())
        {
          if(currentItem->previousItem() != NULL)         //currentItem isn't the first in linked list
          {
            currentItem->previousItem()->setNextItem(shadowItem);
            shadowItem->setPreviousItem(currentItem->previousItem());
            currentItem->setPreviousItem(shadowItem);
            shadowItem->setNextItem(currentItem);
          }
          else                    //dirty tricksh*t. Cause it absolutelly doesn't matter how an item
          {                       //is linked. Its beginIndex() tells how it'll be drawn
            if(currentItem->nextItem() != NULL)
              currentItem->nextItem()->setPreviousItem(shadowItem);
            shadowItem->setNextItem(currentItem->nextItem());
            shadowItem->setPreviousItem(currentItem);
            currentItem->setNextItem(shadowItem);
          }
        }
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
          double value = (double)(n+1) / (double)framesNum;
          mixInShadow->setFrameWeight(n, (int)(value*100));
        }

        TrailItem* shadowItem = new TrailItem(mixInShadow, "(2)mix in shadow for "+currentItem->Name,
                                              items[i]->endIndex()-framesNum+1, true);
        result.append(shadowItem);

        if(Settings::Instance()->Debug())
        {
          if(currentItem->previousItem() != NULL)       //else it's _firstItem => I'm unable to reset that
          {
            currentItem->previousItem()->setNextItem(shadowItem);
            shadowItem->setPreviousItem(currentItem->previousItem());
            currentItem->setPreviousItem(shadowItem);
            shadowItem->setNextItem(currentItem);
          }
          else                    //dirty tricksh*t
          {
            if(currentItem->nextItem() != NULL)
              currentItem->nextItem()->setPreviousItem(shadowItem);
            shadowItem->setNextItem(currentItem->nextItem());
            shadowItem->setPreviousItem(currentItem);
            currentItem->setNextItem(shadowItem);
          }
        }
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

  for(int item=0; item<items.size(); item++)
  {
    for(int i=0; i<items.size(); i++)
    {
      TrailItem* currentItem = items[item];

      if(i==item)                                           //won't combine with self
        continue;
      if(currentItem->endIndex() <= items[i]->endIndex())   //wrong overlap order
        continue;

      //overlapping or touching
      if(items[i]->endIndex()+1 >= currentItem->beginIndex() && currentItem->mixOut() > 0)
      {
        int endsDiff = currentItem->endIndex() - items[i]->endIndex();
        int framesNum = currentItem->mixOut() > endsDiff ? endsDiff
                                                         : currentItem->mixOut();

        WeightedAnimation* mixOutShadow = new WeightedAnimation(new BVH(), "");
        mixOutShadow->setNumberOfFrames(framesNum);

        //adjust shadow skeleton posture
        int curAnimFrame;
        if(items[i]->endIndex()+1 == currentItem->beginIndex())     //they're touching. TODO: shouldn't this case be actually in the latter branch? (touching+gap)
          curAnimFrame = 0;
        else
          curAnimFrame = items[i]->endIndex() - currentItem->beginIndex() + 1;

        interpolatePosture(items[i]->getAnimation(), items[i]->frames()-1, currentItem->getAnimation(),
                           curAnimFrame, mixOutShadow);

        for(int n=0; n<framesNum; n++)
        {
          double value = (double)(currentItem->mixOut()-n) / (double)currentItem->mixOut();
          mixOutShadow->setFrameWeight(n, (int)(value*100));
        }

        TrailItem* shadowItem = new TrailItem(mixOutShadow, "(1)mix out shadow for "+currentItem->Name,
                                              items[i]->endIndex()+1, true);
        result.append(shadowItem);

        //show shadow items on time-line for debug purposes
        if(Settings::Instance()->Debug())
        {
          if(items[i]->nextItem() != NULL)            //isn't _lastItem
            items[i]->nextItem()->setPreviousItem(shadowItem);
          shadowItem->setPreviousItem(items[i]);
          shadowItem->setNextItem(items[i]->nextItem());
          items[i]->setNextItem(shadowItem);
        }
      }
      //Gap smaller than mix-out
      else if(currentItem->mixOut() > (currentItem->beginIndex() - items[i]->endIndex() - 1))
      {
        int gap = currentItem->beginIndex()-items[i]->endIndex()-1;
        int framesNum = currentItem->mixOut() - gap;

        WeightedAnimation* mixOutShadow = new WeightedAnimation(new BVH(), "");
        mixOutShadow->setNumberOfFrames(framesNum);

        //adjust shadow skeleton posture
        interpolatePosture(items[i]->getAnimation(), items[i]->frames()-1, currentItem->getAnimation(),
                           0, mixOutShadow);

        for(int n=1; n<=framesNum; n++)
        {
          double value = (double)n / (double)currentItem->mixOut();
          mixOutShadow->setFrameWeight(framesNum-n, (int)(value*100));
        }

        TrailItem* shadowItem = new TrailItem(mixOutShadow, "(2)mix out shadow for "+currentItem->Name,
                                              currentItem->beginIndex(), true);
        result.append(shadowItem);

        if(Settings::Instance()->Debug())
        {
          if(items[i]->nextItem() != NULL)            //isn't _lastItem
            items[i]->nextItem()->setPreviousItem(shadowItem);
          shadowItem->setPreviousItem(items[i]);
          shadowItem->setNextItem(items[i]->nextItem());
          items[i]->setNextItem(shadowItem);
        }
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
    1.) its size is sum of overall number of TrailItems created by user (one Item at list position)
        + automatically created mix-in/mix-out helper items
    2.) it is sorted by first frame-position index (lowest index on list's position 0) */
QList<TrailItem*> Blender::mergeAndSortItemsByBeginIndex(QList<TrailItem*> realItems,
                                                         QList<TrailItem*> mixInItems,
                                                         QList<TrailItem*> mixOutItems)
{
  QList<TrailItem*> result;

  while(true)
  {
    int lowestBeginIndex = 999999999;           //begin index of left-most item
    QList<TrailItem*>* lowestItemList = NULL;   //list that contains the item
    int itemIndex = -1;                         //index of an item in the list

    for(int r=0; r<realItems.size(); r++)
    {
      if(realItems.at(r)->beginIndex() < lowestBeginIndex)
      {
        lowestItemList = &realItems;
        lowestBeginIndex = realItems.at(r)->beginIndex();
        itemIndex = r;
      }
    }
    for(int i=0; i<mixInItems.size(); i++)             //BLEH! code repetition. TODO: think of it
    {
      if(mixInItems.at(i)->beginIndex() < lowestBeginIndex)
      {
        lowestItemList = &mixInItems;
        lowestBeginIndex = mixInItems.at(i)->beginIndex();
        itemIndex = i;
      }
    }
    for(int o=0; o<mixOutItems.size(); o++)             //here as well
    {
      if(mixOutItems.at(o)->beginIndex() < lowestBeginIndex)
      {
        lowestItemList = &mixOutItems;
        lowestBeginIndex = mixOutItems.at(o)->beginIndex();
        itemIndex = o;
      }
    }

    if(lowestItemList != NULL)
    {
      result.append(lowestItemList->at(itemIndex));
      lowestItemList->removeAt(itemIndex);
    }
    else break;
  }

  return result;
}


void Blender::cloneAnimation(WeightedAnimation* fromAnim, WeightedAnimation* toAnim)
{
  cloneAnimationHelper(0, fromAnim, toAnim);        //clone position pseudo-node
  int rootIndex = fromAnim->getPartIndex(fromAnim->getMotion());
  cloneAnimationHelper(rootIndex, fromAnim, toAnim);
}


void Blender::cloneAnimationHelper(int limbIndex, WeightedAnimation* fromAnim, WeightedAnimation * toAnim)
{
  QList<int> keys = fromAnim->getNode(limbIndex)->keyframeList();
  foreach(int i, keys)
  {
    FrameData data = fromAnim->getNode(limbIndex)->frameData(i);
    Position pos(data.position().x, data.position().y, data.position().z);        //TODO: FUJ! why don't you just use copy constructor (=) ?
    Rotation rot(data.rotation().x, data.rotation().y, data.rotation().z);
    toAnim->getNode(limbIndex)->addKeyframe(i, pos, rot);
  }
}


/** In given list of TrailItems, finds the one that reaches further on the time-line
    and returns it's last frame index on the line */
int Blender::findHighestEndIndex(QList<TrailItem*> items)
{
  int result = -999999999;

  foreach(TrailItem* item, items)
  {
    if(item->endIndex() > result)
      result = item->endIndex();
  }

  return result;
}


/** Performs the actual blending.
    All animations are blended together depending on their placement on time-line and weight parameters.
    Gaps between items are filled.
    @param sortedItems - TrailItems sorted by their begin index (position on time-line)
    @param result - the target animation which will host the blended postures
    @param lastFrameIndex - time-line index of the last frame of right-most reaching animation */
void Blender::blend(QList<TrailItem*> sortedItems, WeightedAnimation* result, int lastFrameIndex)
{
  if(sortedItems.size() < 2)
    throw new QString("Argument exception: the argument 'sortedItems' contains to few items.");

  int currentItemIndex = 0;
  int intervalStartFrame = 999999999;
  int intervalEndFrame = sortedItems.at(currentItemIndex)->beginIndex() - 1;
  QList<int> itemsInInterval;                   //indices (to sortedItems) of items involved in current interval

  //initial section fill with items that begin on the same leftmost time-line position
  for(int i=0; ; i++)
  {
    if(sortedItems.at(i)->beginIndex() > (intervalEndFrame+1))
      break;
    itemsInInterval.append(i);
  }

  int frameOffset = sortedItems.first()->beginIndex();

  while(intervalEndFrame != lastFrameIndex)
  {
    intervalStartFrame = intervalEndFrame+1;        //we shift to the next section

    //remove indices of all items that ended in previous section
    for(int cur=0; cur<itemsInInterval.size(); cur++)
    {
      if(sortedItems[itemsInInterval[cur]]->endIndex() < intervalStartFrame)   //the difference should be exactly 1
      {
        itemsInInterval.removeAt(cur);
        cur--;          //dirty trick not to skip an element in for loop
      }
    }

    //Add to the section all items, that are beginning right with it
    for(int i=0; i<sortedItems.size(); i++)
    {
      if(sortedItems.at(i)->beginIndex() == intervalStartFrame)
        itemsInInterval.append(i);
      else if(sortedItems.at(i)->beginIndex() > intervalStartFrame)
        break;
    }

    if(!itemsInInterval.isEmpty())
    {
      //find section's end
      int minEndFramePos = 999999999;            //first find next end of items currently inside section
      for(int a=0; a<itemsInInterval.size(); a++)
      {
        if(sortedItems[itemsInInterval[a]]->endIndex() < minEndFramePos)
          minEndFramePos = sortedItems[itemsInInterval[a]]->endIndex();
      }

      int nextIndex = itemsInInterval.last()+1;
      while(true)
      {
        if(nextIndex >= sortedItems.size())                                 //there are no more items after
          break;                                                            //this section

        if(sortedItems[nextIndex]->beginIndex() > minEndFramePos)           //next item is to far
          break;
/*Done few lines above        if(sortedItems[nextIndex]->beginIndex() == intervalStartFrame)      //the item begins right with this
        {                                                                   //section (probably was a reason
          itemsInInterval.append(nextIndex);                                //to end the previous)
        }*/
        if(sortedItems[nextIndex]->endIndex() < minEndFramePos)             //we've already fully included this
        {                                                                   //animation in blending
          nextIndex++;
          continue;
        }
        if(sortedItems[nextIndex]->beginIndex() <= minEndFramePos)          //item's begin index delimits
        {                                                                   //the section (will be processed
          minEndFramePos = sortedItems[nextIndex]->beginIndex() - 1;        //in next section pass)
          break;
        }
      }
      intervalEndFrame = minEndFramePos;
    }
    else                                      //no items in this section = we have an empty gap!
    {
      int itemIndex = findFirstItemAfter(sortedItems, intervalStartFrame);
      intervalEndFrame = sortedItems.at(itemIndex)->beginIndex() - 1;
    }


    //THE BLENDING ITSELF
    if(itemsInInterval.isEmpty())         //we need to fill the gap
    {
      int toFrame = intervalEndFrame - frameOffset;
      BVHNode* position = result->getNode(0);
      copyKeyFrame(position, intervalStartFrame-frameOffset-1, toFrame);
      BVHNode* root = result->getMotion();
      copyKeyFrame(root, intervalStartFrame-frameOffset-1, toFrame);
    }
    else if(itemsInInterval.size() == 1)  //only one animation to blend. we can afford to take
    {                                     //just its first frame as key frame of result
      int resultStartFrame = intervalStartFrame - frameOffset;

      BVHNode* position = result->getNode(0);
      combineKeyFramesHelper(sortedItems, itemsInInterval, position, intervalStartFrame, result,
                             resultStartFrame);
      BVHNode* root = result->getMotion();
      combineKeyFramesHelper(sortedItems, itemsInInterval, root, intervalStartFrame, result,
                             resultStartFrame);
    }
    else
    {
      combineKeyFrames(sortedItems, itemsInInterval, intervalStartFrame,
                       intervalEndFrame-intervalStartFrame+1, result, intervalStartFrame-frameOffset);
    }

  }//while
}


/** The very core method of blending functionality. Combines postures into resulting animation.
    @param sortedItems - list of all TrailItems to be blended
    @param itemIndices - indices to the first list argument denoting the animations to be blended now
    @param fromTimeLineFrame - frame position number on time-line where to start blending
    @param sectionLength - number of frames to be blended
    @param target - resulting animation to host mixed postures
    @param targetFrame - first frame for result posture in the target animation */
void Blender::combineKeyFrames(QList<TrailItem*> sortedItems, QList<int> itemIndices, int fromTimeLineFrame,
                               int sectionLength, WeightedAnimation* target, int targetFrame)
{
  if(itemIndices.size() < 1)
    throw new QString("Argument exception: no animation indices given to be combined.");


  int lastTimeLineFrame = fromTimeLineFrame + sectionLength - 1;
  int timeLineFrame = fromTimeLineFrame;

  while(timeLineFrame < lastTimeLineFrame)
  {
    BVHNode* position = target->getNode(0);
    combineKeyFramesHelper(sortedItems, itemIndices, position, timeLineFrame, target, targetFrame);

    BVHNode* root = target->getMotion();
    combineKeyFramesHelper(sortedItems, itemIndices, root, timeLineFrame, target, targetFrame);

    timeLineFrame++;
    targetFrame++;
  }//while
}


void Blender::combineKeyFramesHelper(QList<TrailItem*> sortedItems, QList<int> itemIndices, BVHNode* limb,
                                     int timeLineFrame, WeightedAnimation* target, int targetFrame)
{
  int partIndex = target->getPartIndex(limb);

  Position sumPos(0.0, 0.0, 0.0);
  Rotation sumRot(0.0, 0.0, 0.0);

  int count = itemIndices.size();
  for(int i=0; i<count; i++)
  {
    TrailItem* currentItem = sortedItems[itemIndices[i]];
    int currentFrame = timeLineFrame - currentItem->beginIndex();
    BVHNode* node = currentItem->getAnimation()->getNode(partIndex);
    FrameData data = node->frameData(currentFrame);
    sumPos.Add(data.position());
    sumRot.Add(data.rotation());
  }

  Position pos(sumPos.x/count, sumPos.y/count, sumPos.z/count);     //At present no weights are involved. TODO
  Rotation rot(sumRot.x/count, sumRot.y/count, sumRot.z/count);
  limb->addKeyframe(targetFrame, pos, rot);

  for(int x=0; x<limb->numChildren(); x++)
    combineKeyFramesHelper(sortedItems, itemIndices, limb->child(x), timeLineFrame, target, targetFrame);
}


/** Finds first TrailItem that begins after given time-line position. Returns its index in containing
    list (also given as argument) **/
int Blender::findFirstItemAfter(QList<TrailItem*> sortedItems, int afterPosition)
{
  int min = 999999999;
  int result = -1;

  for(int i=0; i < sortedItems.size(); i++)
  {
    if(sortedItems[i]->beginIndex() > afterPosition && sortedItems[i]->beginIndex() < min)
    {
      min = sortedItems[i]->beginIndex();
      result = i;
    }
  }

  return result;
}


/** A helper to clone all BVHNode frame data inside one animation from one frame to another.
    Meant to be used to fill time-line gaps */
void Blender::copyKeyFrame(BVHNode* limb, int fromFrame, int toFrame)
{
  Position pos = limb->frameData(fromFrame).position();
  Rotation rot = limb->frameData(fromFrame).rotation();
  limb->addKeyframe(toFrame, pos, rot);

  for(int i=0; i<limb->numChildren(); i++)
    copyKeyFrame(limb->child(i), fromFrame, toFrame);
}
