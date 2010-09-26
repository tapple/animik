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
 ***************************************************************************

 * Copyright (C) 2006 by Vinay Pulim.
 * ported to Qt by Zi Ree
 * features added by Darkside Eldrich
 * All rights reserved.             */

/**
  @author Vinay Pulim
  @author Zi Ree <Zi Ree @ SecondLife>
  @author Darkside Eldrich
*/

#ifndef ANIMATIONVIEW_H
#define ANIMATIONVIEW_H

#include <QtOpenGL/QGLWidget>

#include "animation.h"
#include "camera.h"
#include "rotation.h"
#include "prop.h"
#include "bvh.h"

#define MALE_BVH   "data/SLMale.bvh"
#define FEMALE_BVH "data/SLFemale.bvh"

// defines where we start counting opengl ids for parts with multiple animations
// first animation counts 0-ANIMATION_INCREMENT-1, next ANIMATION_INCREMENT++
#define ANIMATION_INCREMENT 100

#define OBJECT_START      8000

#define DRAG_HANDLE_START OBJECT_START+1000
#define DRAG_HANDLE_X     DRAG_HANDLE_START
#define DRAG_HANDLE_Y     DRAG_HANDLE_START+1
#define DRAG_HANDLE_Z     DRAG_HANDLE_START+2
#define SCALE_HANDLE_X    DRAG_HANDLE_START+3
#define SCALE_HANDLE_Y    DRAG_HANDLE_START+4
#define SCALE_HANDLE_Z    DRAG_HANDLE_START+5
#define ROTATE_HANDLE_X   DRAG_HANDLE_START+6
#define ROTATE_HANDLE_Y   DRAG_HANDLE_START+7
#define ROTATE_HANDLE_Z   DRAG_HANDLE_START+8

class QEvent;
class QMouseEvent;



class AnimationView : public QGLWidget
{
  Q_OBJECT

  public:
    AnimationView(QWidget* parent=0,const char* name=0,Animation* anim=0);
    ~AnimationView();

    // exports the BVH class handler (ugly, need to find a better way)
    BVH* getBVH() const;

    // Sets an animation "active"
    void selectAnimation(unsigned int index);

    // this is for setting a single-pose animation.  It will clear all other
    // current animations, and become the only active one
    void setAnimation(Animation *anim);

    // this is for adding subsequent animations after the first call to
    // setAnimation
    void addAnimation(Animation *anim);

    // This function clears the animations
    void clear();

    // These functions are re-implemented here so that every animation's
    // frame data can be changed at once
    void setFrame(int frame);
    void stepForward();
    void setFPS(int fps);

    // getAnimation returns the *current* animation
    Animation* getAnimation() { return animation; }

    //edu: TODO: it should be that one is primary and other auxiliary
    Animation* getAnimation(unsigned int index) { return animList.at(index); }
    Animation* getLastAnimation() { return animList.last(); }
    bool isSkeletonOn() { return skeleton; }
    void showSkeleton() { skeleton = true; }
    void hideSkeleton() { skeleton = false; }
    void selectPart(BVHNode* node);
    void selectProp(const QString& prop);
    BVHNode* getSelectedPart();
    unsigned int getSelectedPartIndex();
    // const QString getPartName(int index);
    const QString getSelectedPropName();

    const Prop* addProp(Prop::PropType type,double x,double y,double z,double xs,double ys,double zs,double xr,double yr,double zr,int attach);
    void deleteProp(Prop* prop);
    void clearProps();
    Prop* getPropByName(const QString& name);
    Prop* getPropById(unsigned int id);

  signals:
    void partClicked(BVHNode* node,Rotation rot,RotationLimits rotLimit,Position pos);
    void partClicked(int part);
    void propClicked(Prop* prop);

    void partDragged(BVHNode* node,double changeX,double changeY,double changeZ);

    void propDragged(Prop* prop,double changeX,double changeY,double changeZ);
    void propRotated(Prop* prop,double changeX,double changeY,double changeZ);
    void propScaled(Prop* prop,double changeX,double changeY,double changeZ);

    void backgroundClicked();
    void animationSelected(Animation* animation);

    void storeCameraPosition(int num);
    void restoreCameraPosition(int num);

  public slots:
    void resetCamera();
    void protectFrame(bool on);
    void selectPart(int part);

  protected slots:
    void draw();

  protected:
    enum
    {
      MODE_PARTS,
      MODE_SKELETON,
      MODE_ROT_AXES
    };


    virtual void paintGL();
    virtual void paintOverlayGL();
    virtual void initializeGL();
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void mouseDoubleClickEvent(QMouseEvent* event);
    virtual void wheelEvent(QWheelEvent* event);
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void keyReleaseEvent(QKeyEvent* event);
    virtual void resizeEvent(QResizeEvent* newSize);

    void drawFloor();
    void drawAnimations();
    void drawFigure(Animation* anim, unsigned int index);
    void drawPart(Animation* anim, unsigned int index, int frame, BVHNode *motion,
                  BVHNode *joints, int mode);
    void drawProps();
    void drawProp(const Prop* prop) const;
    void drawDragHandles(const Prop* prop) const;

    BVH* bvh;

    bool leftMouseButton;
    bool frameProtected;
    char modifier;
    unsigned int nextPropId;

    QList<Prop*> propList;
    QPoint clickPos;           // holds the mouse click position for dragging
    QPoint returnPos;          // holds the mouse position to return to after dragging

    QStringList figureFiles;   // holds the names of the BVH files for male/female skeleton models

    QList<Animation*> animList;
    Animation* animation; // this is the "currently selected" animation
    Camera camera;
    double changeX, changeY, changeZ;
    BVHNode* joints[Animation::NUM_FIGURES];

    bool skeleton;
    bool selecting;
    unsigned int selectName;
    unsigned int partHighlighted;
    unsigned int partSelected;
    unsigned int mirrorSelected;
    unsigned int propSelected;  // needs an own variable, because we will drag the handle, not the prop
    unsigned int propDragging;  // holds the actual drag handle id

    int dragX, dragY;           // holds the last mouse drag offset
    int oldDragX, oldDragY;     // holds the mouse position before the last drag

    int drawMode;
    bool xSelect, ySelect, zSelect;

    bool inAnimList(Animation *anim);
    void setProjection();
    void setModelView();
    void setBodyMaterial();
    void clearSelected();
    int pickPart(int x, int y);
    void drawCircle(int axis, float radius, int width);
};

#endif
