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
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#include "camera.h"

#include <QDebug>

CameraPosition::CameraPosition(float px,float py,float pz,float rx,float ry)
{
  panX=px;
  panY=py;
  panZ=pz;
  rotX=rx;
  rotY=ry;
}

CameraPosition::~CameraPosition()
{
}

Camera::Camera()
{
  positions.append(new CameraPosition(0.0, 40.0, 100.0, 40.0, 37.0));
  positions.append(new CameraPosition(0.0, 40.0, 100.0, 48.0, 320.0));
  positions.append(new CameraPosition(0.0, 40.0, 100.0, 17.0, 180.0));
  positions.append(new CameraPosition(0.0, 15.0, 100.0, 80.0, 360.0));
  reset();
}

Camera::~Camera()
{
  while(!positions.isEmpty())
    delete positions.takeFirst();
}

void Camera::rotate(float x, float y)
{
  rotX += x;
  if (rotX < 2) rotX = 2;
  if (rotX > 80) rotX = 80;
  rotY += y;
  if (rotY < 0) rotY += 360;
  if (rotY > 360) rotY -= 360;
}

void Camera::pan(float x, float y, float z)
{
  panX -= x/2;
  if (panX < -500) panX = -500;
  if (panX > 500) panX = 500;
  panY += y/2;
  if (panY < 5) panY = 5;
  if (panY > 500) panY = 500;
  panZ += z;
  if (panZ < 10) panZ = 10;
  if (panZ > 1000) panZ = 1000;
}

void Camera::setModelView()
{
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(panX, panY, panZ, panX, panY, 0, 0, 1, 0);
  glRotatef(rotX, 1, 0, 0);
  glRotatef(rotY, 0, 1, 0);
}

void Camera::reset()
{
  rotX = 10;
  rotY = 0;
  panX = 0;
  panY = 40;
  panZ = 100;
}

float Camera::xRotation() const
{
  return rotX;
}

float Camera::yRotation() const
{
  return rotY;
}

void Camera::storeCameraPosition(int num)
{
  CameraPosition* pos=positions[num];
  pos->panX=panX;
  pos->panY=panY;
  pos->panZ=panZ;
  pos->rotX=rotX;
  pos->rotY=rotY;
}

void Camera::restoreCameraPosition(int num)
{
  const CameraPosition* pos=positions[num];
  panX=pos->panX;
  panY=pos->panY;
  panZ=pos->panZ;
  rotX=pos->rotX;
  rotY=pos->rotY;
}
