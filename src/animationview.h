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


typedef enum
{
  SINGLE_PART,
  MULTI_PART,
  NO_PICKING                      //picking disabled
} PickingMode;



class AnimationView : public QGLWidget
{
  Q_OBJECT

  public:
    AnimationView(QWidget* parent=0, const char* name=0, Animation* anim=0);
    ~AnimationView();


    //DEBUG so far
    void WriteText(QString text);
    void ClearText();


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

    //Additional flags determining on/off state of some optional features. The difference is mainly
    //implied by different usages of this widget in blending and key-framing GUIs.
    /** When FALSE, circle drags are not drawn around selected limb of figure. */
    bool useRotationHelpers() const                          { return _useRotationHelpers; }
    void setUseRotationHelpers(bool rotHelpers)              { _useRotationHelpers = rotHelpers; }
    /** TRUE means it's possible to lock an effector (by double-click) in further IK calculation */
    bool useIK() const                                       { return _useIK; }
    void setUseIK(bool useIK)                                { _useIK = useIK; }
    bool pickingMode() const                                 { return _pickMode; }
    /*! Sets picking mode and reset any currently picked parts if the mode is "lower" than actual !*/
    void setPickingMode(PickingMode mode);
    /** Determines whether verbose debugging info about currently picked part should be shown.
        The information will be accessed only if DEBUG mode was also activated in Settings,
        and will be shown in the middle of the scene. */
    bool showingPartInfo() const                             { return _partInfo; }
    void setShowingPartInfo(bool partInfo)                   { _partInfo = partInfo; }
    bool useMirroring() const                                { return _useMirror; }
    void setUseMirroring(bool mirror)                        { _useMirror = mirror; }
    void setRelativeJointWeights(QMap<QString, double>* weights);
    bool highlightLimbsWeight() const                        { return _highlightLimbWeight; }
    void setHighlightsLimbsWeight(bool highlight)            { _highlightLimbWeight = highlight; }


    // getAnimation returns the *current* animation
    Animation* getAnimation()                         { return currentAnimation; }

    //edu: TODO: it should be that one is primary and other auxiliary
    Animation* getAnimation(unsigned int index)       { return animList.at(index); }
    Animation* getLastAnimation()                     { return animList.last(); }
    bool isSkeletonOn()                               { return skeleton; }
    void showSkeleton()                               { skeleton = true; }
    void hideSkeleton()                               { skeleton = false; }
    void selectPart(BVHNode* node);
    void selectProp(const QString& prop);
    BVHNode* getSelectedPart();
    QList<int>* getSelectedPartIndices()              { return &selectedParts; }
    unsigned int getSelectedPartIndex();
    // const QString getPartName(int index);
    const QString getSelectedPropName();

    const Prop* addProp(Prop::PropType type,double x,double y,double z,double xs,double ys,double zs,double xr,double yr,double zr,int attach);
    void deleteProp(Prop* prop);
    void clearProps();
    Prop* getPropByName(const QString& name);
    Prop* getPropById(unsigned int id);

  signals:
    void partClicked(BVHNode* node, Rotation rot, Rotation globRot, RotationLimits rotLimit, Position pos, Position partPos);
    void partClicked(int part);
    void propClicked(Prop* prop);
    void partDoubleClicked(int selectedJoint);

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
    virtual void contextMenuEvent(QContextMenuEvent *event);

    void drawFloor();
    void drawAnimations();
    void drawFigure(Animation* anim, unsigned int index);
    void drawPart(Animation* anim, /*unsigned int index,*/ int frame, BVHNode *motion, BVHNode *joints, int mode);
    void drawProps();
    void drawProp(const Prop* prop) const;
    void drawDragHandles(const Prop* prop) const;

    //edu
    void drawRotationHelpers(int frame, BVHNode* motion, BVHNode* joints);

    BVH* bvh;

    bool leftMouseButton;
    bool frameProtected;
    char modifier;
    unsigned int nextPropId;

    QList<Prop*> propList;
    QPoint clickPos;               // holds the mouse click position for dragging
    QPoint returnPos;              // holds the mouse position to return to after dragging

    QStringList figureFiles;       // holds the names of the BVH files for male/female skeleton models

    QList<Animation*> animList;
    Animation* currentAnimation;   // this is the "currently selected" animation
    Camera camera;
    double changeX, changeY, changeZ;
    BVHNode* joints[Animation::NUM_FIGURES];      //edu: general BVH joints taken from default (male/female) figures

    bool skeleton;
    bool selecting;
    unsigned int selectName;                      //edu: seems it's index of part currently being drawn
    unsigned int partHighlighted;
    unsigned int partSelected;
    /** If _multiPartPicking is TRUE, this holds indices of selected body parts. Must always contain partSelected **/
    QList<int> selectedParts;
    unsigned int mirrorSelected;
    unsigned int propSelected;     // needs an own variable, because we will drag the handle, not the prop
    unsigned int propDragging;     // holds the actual drag handle id

    int dragX, dragY;              // holds the last mouse drag offset
    int oldDragX, oldDragY;        // holds the mouse position before the last drag

    int drawMode;
    bool xSelect, ySelect, zSelect;

    bool inAnimList(Animation *anim);
    void setProjection();
    void setModelView();
    void setBodyMaterial();
    void clearSelected();
    int pickPart(int x, int y);
    void drawCircle(int axis, float radius, int width);


  private:
    bool _useRotationHelpers;
    bool _useIK;
    PickingMode _pickMode;
    bool _partInfo;
    bool _useMirror;
    bool _highlightLimbWeight;
    /** Storage for relative weights (between 0.0 and 1.0 including) of joints in current frame
        of an animation. Key is joing name.
        CAUTION: It doesn't have to be (and mostly isn't) the underlying animation of this AnimationView.
        So the QMap must always be externaly set via setRelativeJointWeights() so that it's correctly
        related to current frame of our animation. **/
    QMap<QString, double>* relativeJointWeights;

    QStringList innerTextLines;
    QFont* textFont;

  private slots:
    void debugWrite();
};

#endif
