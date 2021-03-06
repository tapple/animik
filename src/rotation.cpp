/***************************************************************************
 *   Copyright (C) 2006 by Zi Ree   *
 *   Zi Ree @ SecondLife   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "rotation.h"

Rotation::Rotation()
{
  x=0;
  y=0;
  z=0;
}

Rotation::Rotation(double rx,double ry, double rz)
{
  x=rx;
  y=ry;
  z=rz;
}

Rotation Rotation::difference(const Rotation& rot1,const Rotation& rot2)
{
  return Rotation(rot2.x-rot1.x,rot2.y-rot1.y,rot2.z-rot1.z);
}

Rotation::~Rotation()
{
}

void Rotation::Add(Rotation increment)
{
  x += increment.x;
  y += increment.y;
  z += increment.z;
}

void Rotation::Multiply(double mult)
{
  x *= mult;
  y *= mult;
  z *= mult;
}


RotationLimits::RotationLimits(const QString& joint,double rxMin,double rxMax,
                                                    double ryMin,double ryMax,
                                                    double rzMin,double rzMax)
{
  jointName=joint;

  xMin=rxMin;
  yMin=ryMin;
  zMin=rzMin;
  xMax=rxMax;
  yMax=ryMax;
  zMax=rzMax;
}

RotationLimits::~RotationLimits()
{
}

Position::Position()
{
  x=0;
  y=0;
  z=0;
}

Position::Position(double px,double py, double pz)
{
  x=px;
  y=py;
  z=pz;
}

Position::~Position()
{
}

void Position::Add(Position increment)
{
  x += increment.x;
  y += increment.y;
  z += increment.z;
}

void Position::Multiply(double mult)
{
  x *= mult;
  y *= mult;
  z *= mult;
}


Position Position::difference(const Position& pos1,const Position& pos2)
{
  return Position(pos2.x-pos1.x, pos2.y-pos1.y, pos2.z-pos1.z);
}
