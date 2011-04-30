/*
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Copyright (C) 2006 by Vinay Pulim.
 * All rights reserved.
 *
 */

#ifdef __APPLE__
#include <QApplication.h>
#endif

#include <iostream>
#include <stdio.h>
#include <string.h>
// #include "main.h"

#include "animation.h"
#include "rotation.h"
#include "bvh.h"
#include "settings.h"


/** The 'model' class for an avatar animation. Maintains the motion data
    and related parameters (loop/ease in/out, rotation limits...), solves IK etc.
  */

Animation::Animation(BVH* newBVH,const QString& bvhFile) :
  frame(0),totalFrames(0),mirrored(false)
{
  qDebug("Animation::Animation(%lx)",(unsigned long) this);

  bvh=newBVH;
  if(!bvh)
  {
    qDebug("Animation::Animation(): BVH initialisation failed.");
    return;
  }

  QString fileName;

  // pick up path from src.pro qmake file's DEFINES section, i.e. usr/share/qavimator
  //dataPath=QAVIMATOR_DATAPATH;
#ifdef __APPLE__
  dataPath=QApplication::applicationDirPath() + "/../Resources";
#else
  dataPath=QAVIMATOR_DATAPATH;
#endif

  // load BVH that defines motion
  if (!bvhFile.isEmpty())
    fileName=bvhFile;
  else
    fileName=dataPath+"/"+DEFAULT_POSE;

  loadBVH(fileName);
  calcPartMirrors();
  useRotationLimits(true);
  setNumberOfFrames(bvh->lastLoadedNumberOfFrames);
  setAvatarScale(bvh->lastLoadedAvatarScale);
  setFigureType(bvh->lastLoadedFigureType);
  setLoopInPoint(bvh->lastLoadedLoopIn);
  setLoopOutPoint(bvh->lastLoadedLoopOut);
  setFrameTime(bvh->lastLoadedFrameTime);
  positionNode=bvh->lastLoadedPositionNode;
  addKeyFrameAllJoints();

  ikTree.set(frames);
  setIK(IK_LHAND, false);
  setIK(IK_RHAND, false);
  setIK(IK_LFOOT, false);
  setIK(IK_RFOOT, false);

  setLoop(false);
  setDirty(false);

  currentPlayTime=0.0;
  setPlaystate(PLAYSTATE_STOPPED);

  connect(&timer,SIGNAL(timeout()),this,SLOT(playbackTimeout()));
}


Animation::~Animation()
{
  qDebug("Animation::~Animation(%lx)",(unsigned long) this);

  // remove rotation keyframes
  if(frames) bvh->bvhDelete(frames);
  // remove position keyframes
  if(positionNode) delete positionNode;
}

void Animation::loadBVH(const QString& bvhFile)
{
  qDebug("Animation::loadBVH(%s)",bvhFile.toLatin1().constData());
  QString limFile=dataPath+"/"+LIMITS_FILE;
  frames=bvh->animRead(bvhFile,limFile);
  setFrame(0);
}

void Animation::loadBVHFromString(const QString& bvhData)
{
  frames = bvh->bvhReadFromString(bvhData);
  positionNode = bvh->lastLoadedPositionNode;
  bvh->parseLimFile(frames, dataPath + "/" + LIMITS_FILE);
  setFrame(0);
}


void Animation::saveBVH(const QString& bvhFile)
{
  qDebug("Animation::saveBVH(%s)",bvhFile.toLatin1().constData());
  bvh->animWrite(this,bvhFile);
  setDirty(false);
}

int Animation::fps() const
{
  if(frames)
    return framesPerSecond;
  else
    return 0;
}

void Animation::setFPS(int fps)
{
  if(frames)
  {
    framesPerSecond=fps;
    setDirty(true);
  }
}

// convenience
void Animation::setFrameTime(double frameTime)
{
  framesPerSecond=(int) (1.0/frameTime+0.5);
}

// convenience
double Animation::frameTime() const
{
  return 1.0/framesPerSecond;
}

int Animation::getNumberOfFrames()
{
// qDebug("Animation::getNumberOfFrames()=%d",totalFrames);
  return totalFrames;
}

void Animation::setNumberOfFrames(int num)
{
  qDebug("Animation::setNumberOfFrames(%d)",num);
  totalFrames=num;
  setDirty(true);
  emit numberOfFrames(num);
}

int Animation::getFrame()
{
  return frame;
}

void Animation::setFrame(int frameNumber)
{
  if(frameNumber>=0 && frameNumber<totalFrames && frame!=frameNumber)
  {
//    for (int i=0; i<NUM_IK; i++) {
//      setIK((IKPartType)i, false);
//    }
    frame=frameNumber;

    for(int i=0;i<NUM_IK;i++)
    {
      if(ikOn[i])
      {
        solveIK();
        break;
      }
    }

    emit currentFrame(frame);
    emit frameChanged();
  }
}

void Animation::setMirrored(bool mirror)
{
  mirrored=mirror;
}

bool Animation::getMirrored()
{
  return mirrored;
}

// get next frame and take care of looping
int Animation::stepForward()
{
  if(frames)
  {
    int nextFrame=(frame+1) % totalFrames;

    if(loop)
    {
      if(!nextFrame || nextFrame>loopOutPoint) nextFrame=loopInPoint;
    }
    else
    {
      // do not wrap to the beginning when not looping
      if(!nextFrame) nextFrame=frame;
    }
    setFrame(nextFrame);

    return nextFrame;
  }
  return 0;
}

void Animation::setEaseIn(BVHNode* node,int frameNum,bool state)
{
  if(!node)
  {
    qDebug("Animation::setEaseIn(): node==0!");
    return;
  }

  if(node->isKeyframe(frameNum))
  {
    setDirty(true);
    node->setEaseIn(frameNum,state);
    // tell main class that the keyframe has changed
    emit redrawTrack(getPartIndex(node));
  }
}

void Animation::setEaseOut(BVHNode* node,int frameNum,bool state)
{
  if(!node)
  {
    qDebug("Animation::setEaseOut(): node==0!");
    return;
  }

  if(node->isKeyframe(frameNum))
  {
    setDirty(true);
    node->setEaseOut(frameNum,state);
    // tell main class that the keyframe has changed
    emit redrawTrack(getPartIndex(node));
  }
}

bool Animation::easeIn(BVHNode* node,int frameNum)
{
  if(!node)
  {
    qDebug("Animation::easeIn(): node==0!");
    return false;
  }

  if(node->isKeyframe(frameNum))
  {
    return node->easeIn(frameNum);
  }
  qDebug("Animation::easeIn(%s): requested easeIn for non-keyframe!",node->name().toLatin1().constData());
  return false;
}

bool Animation::easeOut(BVHNode* node,int frameNum)
{
  if(!node)
  {
    qDebug("Animation::easeOut(): node==0!");
    return false;
  }

  if(node->isKeyframe(frameNum))
  {
    return node->easeOut(frameNum);
  }
  qDebug("Animation::easeOut(%s): requested easeOut for non-keyframe!",node->name().toLatin1().constData());
  return false;
}

void Animation::setLoopInPoint(int inFrame)
{
//  qDebug("Animation::setLoopInPoint(%d)",inFrame);
  loopInPoint=inFrame;
  setDirty(true);
}

int Animation::getLoopInPoint()
{
  return loopInPoint;
}

void Animation::setLoopOutPoint(int outFrame)
{
//  qDebug("Animation::setLoopOutPoint(%d)",outFrame);
  loopOutPoint=outFrame;
  setDirty(true);
}

int Animation::getLoopOutPoint()
{
  return loopOutPoint;
}

void Animation::applyIK(const QString& name)
{
  BVHNode* node=bvh->bvhFindNode(frames,name);

  Rotation rot=node->frameData(frame).rotation();

  if(node)
  {
//    for (int i=0; i<3; i++) {
    rot.x+=node->ikRot.x;
    rot.y+=node->ikRot.y;
    rot.z+=node->ikRot.z;

    node->ikRot.x=0;
    node->ikRot.y=0;
    node->ikRot.z=0;
/*
      node->frame[frame][i] += node->ikRot[i];
      node->ikRot[i] = 0;
*/
//      node->ikOn = false;

    setDirty(true);
    addKeyFrame(node);
    node->setKeyframeRotation(frame,rot);
    emit redrawTrack(getPartIndex(node));
  }
}

void Animation::setIK(IKPartType part,bool flag)
{
  if(ikOn[part]==flag) return;

  ikOn[part]=flag;

  if(flag)
  {
    switch(part)
    {
      case IK_LHAND: ikTree.setGoal(frame,"lHand"); break;
      case IK_RHAND: ikTree.setGoal(frame,"rHand"); break;
      case IK_LFOOT: ikTree.setGoal(frame,"lFoot"); break;
      case IK_RFOOT: ikTree.setGoal(frame,"rFoot"); break;
      default: break;
    }
  }
  else
  {
    switch(part)
    {
      case IK_LHAND:
        applyIK("lHand");
        applyIK("lForeArm");
        applyIK("lShldr");
        applyIK("lCollar");
        if(!ikOn[IK_RHAND])
        {
          applyIK("chest");
          applyIK("abdomen");
        }
        break;
      case IK_RHAND:
        applyIK("rHand");
        applyIK("rForeArm");
        applyIK("rShldr");
        applyIK("rCollar");
        if (!ikOn[IK_LHAND])
        {
          applyIK("chest");
          applyIK("abdomen");
        }
        break;
      case IK_LFOOT:
        applyIK("lThigh");
        applyIK("lShin");
        applyIK("lFoot");
        break;
      case IK_RFOOT:
        applyIK("rThigh");
        applyIK("rShin");
        applyIK("rFoot");
        break;
      default:
        break;
    }
  }
}

void Animation::setIK(BVHNode* node,bool flag)
{
  QString jointName=node->name();

  if(jointName=="lHand" ||
     jointName=="lForeArm" ||
     jointName=="lShldr" ||
     jointName=="lCollar")
  {
    setIK(IK_LHAND, flag);
  }
  else if(jointName=="rHand" ||
          jointName=="rForeArm" ||
          jointName=="rShldr" ||
          jointName=="rCollar")
  {
    setIK(IK_RHAND, flag);
  }
  else if(jointName=="lThigh" ||
          jointName=="lShin" ||
          jointName=="lFoot")
  {
    setIK(IK_LFOOT, flag);
  }
  else if(jointName=="rThigh" ||
          jointName=="rShin" ||
          jointName=="rFoot")
  {
    setIK(IK_RFOOT, flag);
  }
}

bool Animation::getIK(BVHNode* node)
{
  QString jointName=node->name();

  if(jointName=="lHand" ||
     jointName=="lForeArm" ||
     jointName=="lShldr" ||
     jointName=="lCollar")
  {
    return getIK(IK_LHAND);
  }
  else if(jointName=="rHand" ||
          jointName=="rForeArm" ||
          jointName=="rShldr" ||
          jointName=="rCollar")
  {
    return getIK(IK_RHAND);
  }
  else if(jointName=="lThigh" ||
          jointName=="lShin" ||
          jointName=="lFoot")
  {
    return getIK(IK_LFOOT);
  }
  else if(jointName=="rThigh" ||
          jointName=="rShin" ||
          jointName=="rFoot")
  {
    return getIK(IK_RFOOT);
  }
  return false;
}

bool Animation::getIK(IKPartType part)
{
  return ikOn[part];
}

void Animation::solveIK()
{
  bvh->bvhResetIK(frames);
  if(ikOn[IK_LFOOT]) getEndSite("lFoot")->ikOn=true;
  if(ikOn[IK_RFOOT]) getEndSite("rFoot")->ikOn=true;
  if(ikOn[IK_LHAND]) getEndSite("lHand")->ikOn=true;
  if(ikOn[IK_RHAND]) getEndSite("rHand")->ikOn=true;

//  ikTree.setJointLimits(true);
  ikTree.solve(frame);
}

void Animation::setRotation(BVHNode* node,double x,double y,double z)
{
  if (node)
  {
    //qDebug(QString("Animation::setRotation(")+jointName+")");

    for(int i=0;i<NUM_IK;i++)
    {
      if(ikOn[i])
      {
        solveIK();
        break;
      }
    }

    if(node->isKeyframe(frame))
    {
      node->setKeyframeRotation(frame, Rotation(x,y,z));
    }
    else
    {
      node->addKeyframe(frame, node->frameData(frame).position(), Rotation(x,y,z));
      setEaseIn(node, frame, Settings::Instance()->easeIn());
      setEaseOut(node, frame, Settings::Instance()->easeOut());
    }

    //      node->dumpKeyframes();
    BVHNode* mirrorNode=node->getMirror();
    if(mirrored && mirrorNode)
    {
      // new keyframe system
      if(mirrorNode->isKeyframe(frame))
        mirrorNode->setKeyframeRotation(frame,Rotation(x,-y,-z));
      else
      {
        mirrorNode->addKeyframe(frame,node->frameData(frame).position(),Rotation(x,-y,-z));
        setEaseIn(mirrorNode,frame,Settings::Instance()->easeIn());
        setEaseOut(mirrorNode,frame,Settings::Instance()->easeOut());
      }

      // tell timeline that this mirrored keyframe has changed (added or changed is the same here)
      emit redrawTrack(getPartIndex(mirrorNode));
    }
    setDirty(true);
    // tell timeline that this keyframe has changed (added or changed is the same here)
    emit redrawTrack(getPartIndex(node));
    emit frameChanged();
  }
  else
  {
    qDebug("Animaiton::setRotation(): node==0!");
  }
}

Rotation Animation::getRotation(BVHNode* node)
{
  if(node)
  {
    return node->frameData(frame).rotation();
  }
  qDebug("Animation::getRotation(): node==0!");
  return Rotation();
}

//edu
double Animation::getRelWeight(BVHNode* node)
{
  if(node != NULL)
  {
    FrameData _debug = node->frameData(frame);
    return _debug.relativeWeight();
  }
  return -1.0;
}

//edu
/*! Global limb rotation as sum of relative rotations of parent limbs up to the root !*/
Rotation Animation::getGlobalRotation(BVHNode* node)
{
  if(node!=NULL)
  {
    Rotation relRot = node->frameData(frame).rotation();
    Rotation parentGlobRot = getGlobalRotation(node->Parent());
    return Rotation(relRot.x+parentGlobRot.x, relRot.y+parentGlobRot.y, relRot.z+parentGlobRot.z);
  }
  qDebug("Animation::getGlobalRotation(): node==0! Returning Rotation(0, 0, 0)");
  return Rotation(0, 0, 0);
}


void Animation::useRotationLimits(bool flag)
{
  limits=flag;
  ikTree.setJointLimits(flag);
}

RotationLimits Animation::getRotationLimits(BVHNode* node)
{
  if(node)
  {
    double xMin,yMin,zMin,xMax,yMax,zMax;

    if(node->type==BVH_POS)
    {
      xMin=yMin=zMin=xMax=yMax=zMax=0;
    }
    else if(limits)
    {
      bvh->bvhGetChannelLimits(node,BVH_XROT,&xMin,&xMax);
      bvh->bvhGetChannelLimits(node,BVH_YROT,&yMin,&yMax);
      bvh->bvhGetChannelLimits(node,BVH_ZROT,&zMin,&zMax);
    }
    else
    {
      xMin=yMin=zMin=-180;
      xMax=yMax=zMax=180;
    }

    RotationLimits rotLimit(node->name(),xMin,xMax,yMin,yMax,zMin,zMax);
    return rotLimit;
  }
  qDebug("Animation::getRotationLimits(): node==0!");
  return RotationLimits(QString::null,0,0,0,0,0,0);
}

int Animation::getRotationOrder(const QString& jointName)
{
  BVHNode* node=bvh->bvhFindNode(frames,jointName);
  if(node)
  {
    return node->channelOrder;
  }
  return 0;
}

void Animation::setPosition(double x,double y,double z)
{
  for(int i=0;i<NUM_IK;i++)
  {
    if(ikOn[i])
    {
      solveIK();
      break;
    }
  }
  // new keyframe system
  if(positionNode->isKeyframe(frame))
  {
    positionNode->setKeyframePosition(frame, Position(x,y,z));
  }
  else
  {
    positionNode->addKeyframe(frame, Position(x,y,z), Rotation());
    setEaseIn(positionNode, frame, Settings::Instance()->easeIn());
    setEaseOut(positionNode, frame, Settings::Instance()->easeOut());
  }


  //edu: make position changes also to root (hip) to solve the bug of position changes not being saved
  BVHNode* root = frames;
  if(root->isKeyframe(frame))
    root->setKeyframePosition(frame, Position(x, y, z));
  else
  {
    root->addKeyframe(frame, Position(x,y,z), root->frameData(frame).rotation());
    setEaseIn(root, frame, Settings::Instance()->easeIn());
    setEaseOut(root, frame, Settings::Instance()->easeOut());
  }


  setDirty(true);
  // tell timeline that this keyframe has changed (added or changed is the same here)
  emit redrawTrack(0);
  emit frameChanged();
}

Position Animation::getPosition()
{
  return positionNode->frameData(frame).position();
}


Position Animation::getGlobalPosition(BVHNode* node)
{
  if(node==NULL)
    return Position();
  return node->frameData(frame).position();
}


const QString Animation::getPartName(int index) const
{
  //  exception fot position pseudonode
  if(index==0) return positionNode->name();
  return bvh->bvhGetName(frames,index);
}

int Animation::getPartIndex(BVHNode* node)
{
  if(node==positionNode) return 0;
  return bvh->bvhGetIndex(frames,node->name());
}

BVHNode* Animation::getMotion()
{
  return frames;
}

BVHNode* Animation::getEndSite(const QString& rootName)
{
  BVHNode* node=bvh->bvhFindNode(frames,rootName);
  while(node && node->numChildren()>0)
  {
    node=node->child(0);
  }
  return node;
}

void Animation::recursiveAddKeyFrame(BVHNode* joint)
{
  if(joint->type!=BVH_END)
    addKeyFrame(joint);

  for(int i=0;i<joint->numChildren();i++)
    recursiveAddKeyFrame(joint->child(i));

  setDirty(true);
}

void Animation::addKeyFrameAllJoints()
{
  addKeyFrame(getNode(0));
  recursiveAddKeyFrame(frames);
}

void Animation::addKeyFrame(BVHNode* joint)
{
  joint->addKeyframe(frame,getPosition(),getRotation(joint));

  setEaseIn(joint, frame, Settings::Instance()->easeIn());
  setEaseOut(joint, frame, Settings::Instance()->easeOut());

  setDirty(true);

  emit redrawTrack(getPartIndex(joint));
  emit frameChanged();
}

bool Animation::isKeyFrameHelper(BVHNode *joint)
{
  if(joint->isKeyframe(frame))
    return true;

  for (int i=0;i<joint->numChildren();i++)
  {
    if(isKeyFrameHelper(joint->child(i)))
    {
      return true;
    }
  }

  return false;
}

bool Animation::isKeyFrame(const QString& jointName)
{
  if(jointName.isEmpty())
     return isKeyFrame();
  else
  {
    BVHNode* node=bvh->bvhFindNode(frames,jointName);
    return node->isKeyframe(frame);
  }
  qDebug("Animation::isKeyFrame('%s'): no node found.",jointName.toLatin1().constData());
}

bool Animation::isKeyFrame()
{
  if(positionNode->isKeyframe(frame)) return true;
  return isKeyFrameHelper(frames);
}

bool Animation::isKeyFrame(int jointNumber,int frame)
{
  const BVHNode* joint=getNode(jointNumber);
  return joint->isKeyframe(frame);
}

void Animation::deleteKeyFrame(BVHNode* joint,int frameNum,bool silent)
{
  // never delete first keyframe
  if(frameNum)
  {
    joint->deleteKeyframe(frameNum);
    setDirty(true);
  }

  // if silent is true then only send a signal to the timeline but not to the animation view
  if(!silent) emit frameChanged();
  emit redrawTrack(getPartIndex(joint));
}

void Animation::deleteKeyFrame(int jointNumber,int frameNum)
{
  // frames should always be current frame, but better play safe for future enhancements
  setFrame(frameNum);
  if(jointNumber)
  {
     BVHNode* joint=getNode(jointNumber);
     if(joint->isKeyframe(frameNum))
       deleteKeyFrame(joint,frameNum);
  }
  else if(isKeyFrame()) deleteKeyFrameAllJoints();

  // tell main class that the keyframe has changed
  emit currentFrame(frameNum);
}

void Animation::recursiveDeleteKeyFrame(BVHNode* joint)
{
  deleteKeyFrame(joint,frame);

  for(int i=0;i<joint->numChildren();i++)
    recursiveDeleteKeyFrame(joint->child(i));
}

void Animation::deleteKeyFrameAllJoints()
{
  // never delete the first keyframe
  if(frame==0) return;
  deleteKeyFrame(getNode(0),frame);
  recursiveDeleteKeyFrame(frames);
}

bool Animation::toggleKeyFrame(BVHNode* node)
{
//  qDebug("Animation::toggleKeyFrame(node): node %ld",(unsigned long) node);
  if(!node)
    return toggleKeyFrameAllJoints();
  else
  {
    if (node->isKeyframe(frame))
    {
      deleteKeyFrame(node,frame);
      return false;
    }
    else
    {
      addKeyFrame(node);
      return true;
    }
  }
}

// returns TRUE if frame is now a keyframe for entire animation, FALSE if not
bool Animation::toggleKeyFrameAllJoints()
{
  if(frame==0)
    return true;  // first frame will always stay keyframe

  if(isKeyFrame())
  {
    deleteKeyFrameAllJoints();
    return false;
  }
  else
  {
    addKeyFrameAllJoints();
    return true;
  }
}

void Animation::cutFrame()
{
  // copy frame data into copy buffer
  copyFrame();
  // always delete frame from all tracks
  deleteFrame(0,frame);
}

void Animation::copyFrame()
{
  bvh->bvhGetFrameData(frames,frame);
}

void Animation::pasteFrame()
{
  bvh->bvhSetFrameData(frames,frame);
  addKeyFrameAllJoints();
}

void Animation::calcPartMirrors()
{
  QString name;
  QString n;

  // start at node index 1
  int i=1;
  // go through all nodes by index, stop if a node has no name (=does not exist =end of list)
  while(!(n=bvh->bvhGetName(frames,i)).isEmpty())
  {
    // find node by name
    BVHNode* node=bvh->bvhFindNode(frames,n);
    // copy the name
    name=n;
    // create a mirrored name (first letter r becomes l, otherwise first letter becomes r)
    if(n.startsWith("r")) name[0]='l';
    else name[0]='r';

    // check if mirrored name is valid, get the node with that name
    BVHNode* m=bvh->bvhFindNode(frames,name);
    if(m)
    {
      // name was valid, record this node as mirror of the current node
      // keep the index number for une in AnimationView later
      node->setMirror(m,bvh->bvhGetIndex(frames,m->name()));
    }
    // next node
    i++;
  }
}

int Animation::numKeyFrames(int jointNumber)
{
  BVHNode* node=bvh->bvhFindNode(frames,getPartName(jointNumber));
//  qDebug(QString("Animation::numKeyFrames(): joint number %1 has %2 keyframes").arg(jointNumber).arg(node->numKeyFrames));
  return node->numKeyframes();
}

// copies the position and rotation data of one body part to another key frame position
void Animation::copyKeyFrame(int jointNumber,int from,int to)
{
  // move keyframe in copy mode
  moveKeyFrame(jointNumber,from,to,true);
}

// moves the position and rotation data of one body part to another key frame position
void Animation::moveKeyFrame(int jointNumber,int from,int to,bool copy)
{
  qDebug("Animation::moveKeyFrame(): jointNumber: %d",jointNumber);

  // make sure we don't drag a trail of mirror keys behind
  setMirrored(false);

  // set frame pointer to source frame position
  setFrame(from);

  // get the joint structure
  BVHNode* joint=getNode(jointNumber);
  const FrameData& frameData=joint->frameData(from);
//  frameData.dump();

  // block all further signals to avoid flickering
  blockSignals(true);

  // silently (true) delete key frame if not copy mode
  // we do copy mode here to avoid code duplication
  if(!copy) deleteKeyFrame(joint,from,true);

  setFrame(to);

  // move rotation or position of the body part
  if(joint->type==BVH_POS)
  {
    Position pos=frameData.position();
    setPosition(pos.x,pos.y,pos.z);
  }
  else
  {
    Rotation rot=frameData.rotation();
    setRotation(joint,rot.x,rot.y,rot.z);
  }
  // only now set ease in/out, because setRotation/setPosition sets to default when the
  // target position has no keyframe yet
  joint->setEaseIn(to,frameData.easeIn());
  joint->setEaseOut(to,frameData.easeOut());
  // now re-enable signals so we get updates on screen
  blockSignals(false);

  // tell timeline where we are now
  emit currentFrame(frame);
}

bool Animation::compareFrames(const BVHNode* node,int key1,int key2) const
{
  if(node) return node->compareFrames(key1,key2);
  qDebug("Animation::compareFrames(node): node==0!");
  return false;
}

/*edu: Is this ever used?
const FrameData Animation::keyframeDataByIndex(int jointNumber,int index)
{
  BVHNode* joint=getNode(jointNumber);
  return joint->keyframeDataByIndex(index);
}     */

BVHNode* Animation::getNode(int jointNumber)
{
  //####
  // joint number 0 needs to return the hip position pseudonode
  if(jointNumber==0) return positionNode;
  // get the joint structure
  return bvh->bvhFindNode(frames,getPartName(jointNumber));
}


BVHNode* Animation::getNodeByName(QString name) const
{
  if(name == positionNode->name())
    return positionNode;
  return bvh->bvhFindNode(frames, name);
}


void Animation::insertFrameHelper(BVHNode* joint,int frame)
{
  joint->insertFrame(frame);
  for(int i=0;i<joint->numChildren();i++)
    insertFrameHelper(joint->child(i),frame);
}

void Animation::insertFrame(int track,int pos)
{
  if(track==-1)
  {
    // insert positional frame
    BVHNode* joint=getNode(0);
    if(joint) joint->insertFrame(pos);
    // insert all rotational frames
    insertFrameHelper(frames,pos);
  }
  else
  {
    BVHNode* joint=getNode(track);
    if(joint) joint->insertFrame(frame);
  }
  setDirty(true);
  emit frameChanged();
}

// recursively remove frames from joint and all its children
void Animation::deleteFrameHelper(BVHNode* joint,int frame)
{
//  qDebug("Animation::deleteFrameHelper(joint %s,frame %d)",joint->name().toLatin1().constData(),frame);
  joint->deleteFrame(frame);
  for(int i=0;i<joint->numChildren();i++)
    deleteFrameHelper(joint->child(i),frame);
  emit redrawTrack(getPartIndex(joint));
}

// delete frame from a joint, if track==0 recursively delete from all joints
void Animation::deleteFrame(int track,int pos)
{
//  qDebug("Animation::deleteFrame(joint %d,frame %d)",track,frame);

  if(track==-1)
  {
    // delete positional frame
    BVHNode* joint=getNode(0);
    if(joint) joint->deleteFrame(pos);
    // delete all rotational frames
    deleteFrameHelper(frames,pos);
  }
  else
  {
    BVHNode* joint=getNode(track);
    if(joint) joint->deleteFrame(pos);
  }
  setDirty(true);
  emit frameChanged();
}

void Animation::optimizeHelper(BVHNode* joint)
{
  if(joint->type!=BVH_END)
  {
    joint->optimize();
    emit redrawTrack(getPartIndex(joint));
  }

  for(int i=0;i<joint->numChildren();i++)
    optimizeHelper(joint->child(i));
}

void Animation::optimize()
{
  optimizeHelper(positionNode);
  optimizeHelper(frames);
  setDirty(true);
}

void Animation::mirrorHelper(BVHNode* joint)
{
  // make sure only to mirror one side of l/r joints, and joints that have no mirror node
  if(!joint->name().startsWith("l"))
  {
    joint->mirror();
    emit redrawTrack(getPartIndex(joint));
    if(joint->getMirror())
      emit redrawTrack(joint->getMirrorIndex());
  }

  for(int i=0;i<joint->numChildren();i++)
    mirrorHelper(joint->child(i));
}

void Animation::mirror(BVHNode* joint)
{
  if(!joint)
  {
    positionNode->mirror();
    mirrorHelper(frames);
  }
  else
  {
    joint->mirror();
    emit redrawTrack(getPartIndex(joint));
    if(joint->getMirror())
      emit redrawTrack(joint->getMirrorIndex());
  }
  setDirty(true);
}

bool Animation::dirty() const
{
  return isDirty;
}

void Animation::setDirty(bool state)
{
  isDirty=state;
  emit animationDirty(state);
}

void Animation::setLoop(bool on)
{
  loop=on;
}

float Animation::getAvatarScale()
{
  return avatarScale;
}

void Animation::setAvatarScale(float newScale)
{
  avatarScale=newScale;
}

Animation::FigureType Animation::getFigureType()
{
  return figureType;
}

void Animation::setFigureType(FigureType type)
{
  // safety check if figure is valid
  if(type>=0 && type<NUM_FIGURES)
  {
    figureType=type;
    setDirty(true);
  }
}

void Animation::nextPlaystate()
{
}

void Animation::setPlaystate(PlayState state)
{
  switch(state)
  {
    case PLAYSTATE_LOOPING:
    case PLAYSTATE_PLAYING:
      timer.start(PLAYBACK_RESOLUTION);
      break;
    case PLAYSTATE_STOPPED:
      timer.stop();
      break;
    default:
      break;
  }

  playstate=state;
}

// slot
void Animation::playbackTimeout()
{
//  qDebug("Animation::playbackTimeout()");

  currentPlayTime+=PLAYBACK_RESOLUTION/1000.0;

  while(currentPlayTime>=frameTime())
  {
//    qDebug("Animation::playbackTimeout(%lx): !!! currentPlayTime=%f frameTime=%f",this,currentPlayTime,frameTime);
    currentPlayTime-=frameTime();
    stepForward();
  }
}
