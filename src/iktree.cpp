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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "MT_Transform.h"
#include "iktree.h"

int display = 0;

static const MT_Vector3 origin(0,0,0);
static const MT_Vector3 xAxis(1,0,0);
static const MT_Vector3 yAxis(0,1,0);
static const MT_Vector3 zAxis(0,0,1);
static const MT_Quaternion identity(0,0,0,1);

IKTree::IKTree(BVHNode *root) :
  jointLimits(true)
{
  set(root);
}

void IKTree::set(BVHNode *root)
{
  numBones = 0;
  if (root)
    addJoint(root);
}

void IKTree::setGoal(int frame,const QString& name)
{
  reset(frame);

  for (int i=0; i<numBones; i++) {
    if (bone[i].node->name()==name) {
      int j = bone[i].child[0];
      BVHNode *n = bone[j].node;
      n->ikGoalPos[0] = bone[j].pos[0];
      n->ikGoalPos[1] = bone[j].pos[1];
      n->ikGoalPos[2] = bone[j].pos[2];
      n->ikGoalDir[0] = bone[j].pos[0] - bone[i].pos[0];
      n->ikGoalDir[1] = bone[j].pos[1] - bone[i].pos[1];
      n->ikGoalDir[2] = bone[j].pos[2] - bone[i].pos[2];
    }
  }
}

void IKTree::reset(int frame)
{
  MT_Quaternion q;
  BVHNode *n;

  for (int i=0; i<numBones; i++)
  {
    bone[i].pos = origin;
    bone[i].lRot = identity;
    bone[i].gRot = identity;
    n = bone[i].node;
    Rotation rot=n->frameData(frame).rotation();
    Position pos=n->frameData(frame).position();

    for (int k=0; k<n->numChannels; k++)    // rotate each axis in order
    {
      q = identity;
      switch (n->channelType[k]) {
        case BVH_XROT:
          q.setRotation(xAxis, rot.x * M_PI / 180);
        break;
        case BVH_YROT:
          q.setRotation(yAxis, rot.y * M_PI / 180);
        break;
        case BVH_ZROT:
          q.setRotation(zAxis, rot.z * M_PI / 180);
        break;
        case BVH_XPOS: bone[i].pos[0] = pos.x; break;
        case BVH_YPOS: bone[i].pos[1] = pos.y; break;
        case BVH_ZPOS: bone[i].pos[2] = pos.z; break;
      }
      bone[i].lRot = q * bone[i].lRot;
    }
/*
    for (int k=0; k<3; k++) {  // rotate each axis in order
      rad = n->frame[frame][k] * M_PI / 180;
      q = identity;
      switch (n->channelType[k]) {
      case BVH_XROT: q.setRotation(xAxis, rad); break;
      case BVH_YROT: q.setRotation(yAxis, rad); break;
      case BVH_ZROT: q.setRotation(zAxis, rad); break;
      case BVH_XPOS: bone[i].pos[0] = n->frame[frame][k]; break;
      case BVH_YPOS: bone[i].pos[1] = n->frame[frame][k]; break;
      case BVH_ZPOS: bone[i].pos[2] = n->frame[frame][k]; break;
      }
      bone[i].lRot = q * bone[i].lRot;
    }
*/
  }
  updateBones(0);
}

void IKTree::addJoint(BVHNode *node)
{
  bone[numBones].node = node;
  bone[numBones].offset.setValue((float)node->offset[0], (float)node->offset[1], (float)node->offset[2]);
  bone[numBones].weight = node->ikWeight;
  bone[numBones].numChildren = 0;

  int myNumBones = numBones++;
  for (int i=0; i<node->numChildren(); i++)
  {
    bone[myNumBones].child[bone[myNumBones].numChildren++] = numBones;
    addJoint(node->child(i));
  }
}

void IKTree::toEuler(MT_Quaternion &q, BVHOrderType order, double &x, double &y, double &z)
{
  double q0=q[0],q1=q[1],q2=q[2],q3=q[3];

  switch(order) {
  case BVH_ZYX:
    x = -atan2(2*(q1*q2-q3*q0),1-2*(q0*q0+q1*q1))*180/M_PI;
    y = -asin(-2*(q0*q2+q3*q1))*180/M_PI;
    z = -atan2(2*(q0*q1-q3*q2),1-2*(q1*q1+q2*q2))*180/M_PI;
    break;
  case BVH_XYZ:
    x = -atan2(-2*(q1*q2+q3*q0),1-2*(q0*q0+q1*q1))*180/M_PI;
    y = -asin(2*(q0*q2-q3*q1))*180/M_PI;
    z = -atan2(-2*(q0*q1+q3*q2),1-2*(q1*q1+q2*q2))*180/M_PI;
    break;
  case BVH_YZX:
    x = -atan2(-2*(q1*q2+q3*q0),1-2*(q0*q0+q2*q2))*180/M_PI;
    y = -atan2(-2*(q0*q2+q3*q1),1-2*(q1*q1+q2*q2))*180/M_PI;
    z = -asin(2*(q0*q1-q3*q2))*180/M_PI;
    break;
  case BVH_XZY:
    x = -atan2(2*(q1*q2-q3*q0),1-2*(q0*q0+q2*q2))*180/M_PI;
    y = -atan2(2*(q0*q2-q3*q1),1-2*(q1*q1+q2*q2))*180/M_PI;
    z = -asin(-2*(q0*q1+q3*q2))*180/M_PI;
    break;
  case BVH_YXZ:
    x = -asin(-2*(q1*q2+q3*q0))*180/M_PI;
    y = -atan2(2*(q0*q2-q3*q1),1-2*(q0*q0+q1*q1))*180/M_PI;
    z = -atan2(2*(q0*q1-q3*q2),1-2*(q0*q0+q2*q2))*180/M_PI;
    break;
  case BVH_ZXY:
    x = -asin(2*(q1*q2-q3*q0))*180/M_PI;
    y = -atan2(-2*(q0*q2+q3*q1),1-2*(q0*q0+q1*q1))*180/M_PI;
    z = -atan2(-2*(q0*q1+q3*q2),1-2*(q0*q0+q2*q2))*180/M_PI;
    break;
  }
}

void IKTree::updateBones(int i)
{
  IKBone theBone = bone[i];

  for (int chld=0; chld<theBone.numChildren; chld++)
  {
    int k = theBone.child[chld];
    IKBone theChild = bone[k];
    theChild.gRot = theBone.gRot * theBone.lRot;
    MT_Transform rot(MT_Point3(0,0,0), theChild.gRot);
    theChild.pos = theBone.pos + rot * theChild.offset;
    updateBones(k);
  }
}

void IKTree::solveJoint(int frame, int i, IKEffectorList &effList)
{
  double x, y, z;
  double ang = 0;

  MT_Quaternion q;
  MT_Quaternion totalPosRot = MT_Quaternion(0,0,0,0);
  MT_Quaternion totalDirRot = MT_Quaternion(0,0,0,0);
  MT_Vector3 axis(0,0,0);
  BVHNode *n;
  int numPosRot = 0, numDirRot = 0;

  if (bone[i].numChildren == 0)       // reached end site
  {
    if (bone[i].node->ikOn)
      effList.index[effList.num++] = i;
    return;
  }

  for (int j=0; j<bone[i].numChildren; j++)
  {
    IKEffectorList el;
    el.num = 0;
    solveJoint(frame, bone[i].child[j], el);
    for (int k=0; k<el.num; k++) {
      effList.index[effList.num++] = el.index[k];
    }
  }

  updateBones(i);

  for (int j=0; j<effList.num; j++)
  {
    int effIndex = effList.index[j];
    n = bone[effIndex].node;
    MT_Vector3 effGoalPos(n->ikGoalPos[0], n->ikGoalPos[1], n->ikGoalPos[2]);
    const MT_Vector3 pC = (bone[effIndex].pos - bone[i].pos).safe_normalized();
    const MT_Vector3 pD = (effGoalPos - bone[i].pos).safe_normalized();
    MT_Vector3 rotAxis = pC.cross(pD);
    if (rotAxis.length2() > MT_EPSILON)
    {
      totalPosRot += MT_Quaternion(rotAxis, bone[i].weight * acos(pC.dot(pD)));
      numPosRot++;
    }

    const MT_Vector3 uC = (bone[effIndex].pos - bone[effIndex-1].pos).safe_normalized();
    const MT_Vector3 uD = (MT_Vector3(n->ikGoalDir[0], n->ikGoalDir[1], n->ikGoalDir[2])).safe_normalized();
    rotAxis = uC.cross(uD);
    if (rotAxis.length2() > MT_EPSILON)
    {
      double weight = 0.0;
      if (i == effIndex-1) weight = 0.5;
      totalDirRot += MT_Quaternion(rotAxis, weight * acos(uC.dot(uD)));
      numDirRot++;
    }
  }

  if ((numPosRot + numDirRot) > MT_EPSILON)
  {
    n = bone[i].node;
    n->ikOn = true;
    // average the quaternions from all effectors
    if (numPosRot)
      totalPosRot /= numPosRot;
    else
      totalPosRot = identity;
    if (numDirRot)
      totalDirRot /= numDirRot;
    else
      totalDirRot = identity;
    MT_Quaternion targetRot = 0.9 * totalPosRot + 0.1 * totalDirRot;
    targetRot = targetRot * bone[i].lRot;
    toEuler(targetRot, n->channelOrder, x, y, z);
    if (jointLimits)
    {
      bone[i].lRot = identity;
      for (int k=0; k<n->numChannels; k++)    // clamp each axis in order
      {
        switch (n->channelType[k]) {
          case BVH_XROT: ang = x; axis = xAxis; break;
          case BVH_YROT: ang = y; axis = yAxis; break;
          case BVH_ZROT: ang = z; axis = zAxis; break;
          default: break;
        }
        // null axis leads to crash in q.setRotation(), so check first
        if(axis.length())
        {
          if (ang < n->channelMin[k]) ang = n->channelMin[k];
          else if (ang > n->channelMax[k]) ang = n->channelMax[k];
          q.setRotation(axis, ang * M_PI / 180);
          bone[i].lRot = q * bone[i].lRot;
        }
      }
    }
    else
      bone[i].lRot = targetRot;
  }
}

void IKTree::solve(int frame)
{
  double x, y, z;
  reset(frame);

  IKEffectorList effList;

  for (int i=0; i<20; i++) {
    effList.num = 0;
    solveJoint(frame, 0, effList);
  }

  for (int i=0; i<numBones-1; i++)
  {
    BVHNode *n = bone[i].node;
    Rotation rot = n->frameData(frame).rotation();
    toEuler(bone[i].lRot, n->channelOrder, x, y, z);

    for (int j=0; j<n->numChannels; j++)    // rotate each axis in order
    {
      switch (n->channelType[j])
      {
        case BVH_XROT: n->ikRot.x = x - rot.x; break;
        case BVH_YROT: n->ikRot.y = y - rot.y; break;
        case BVH_ZROT: n->ikRot.z = z - rot.z; break;
        default: break;
      }
    }

/*
    for (int j=0; j<3; j++) {  // rotate each axis in order
      switch (n->channelType[j]) {
        case BVH_XROT: n->ikRot[j] = x - n->frame[frame][j]; break;
        case BVH_YROT: n->ikRot[j] = y - n->frame[frame][j]; break;
        case BVH_ZROT: n->ikRot[j] = z - n->frame[frame][j]; break;
        default: break;
      }
    } */
  }

  display = 1;
  updateBones(0);
  display = 0;
}
