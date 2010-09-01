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

#ifndef CAMERA_H
#define CAMERA_H

#include "math3d.h"

#include <QObject>

using namespace Math3D;

class CameraPosition
{
  public:
    CameraPosition(float px,float py,float pz,float rx,float ry);
    ~CameraPosition();

    float panX;
    float panY;
    float panZ;
    float rotX;
    float rotY;
};

class Camera : public QObject
{
  Q_OBJECT

  public:
    Camera();
    ~Camera();

    void rotate(float x, float y);
    void pan(float x, float y, float z);
    void setModelView();
    void reset();

    float xRotation() const;
    float yRotation() const;

  public slots:
    void storeCameraPosition(int num);
    void restoreCameraPosition(int num);

  protected:
    float rotX,rotY;
    float panX,panY,panZ;

    QList<CameraPosition*> positions;
};

#endif
