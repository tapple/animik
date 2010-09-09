/***************************************************************************
 *   Copyright (C) 2006 by Zi Ree                                          *
 *   Zi Ree @ SecondLife                                                   *
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

#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>

#include "qavimator.h"
#include "animationview.h"
#include "rotation.h"
#include "prop.h"
#include "timeline.h"
#include "timelineview.h"
#include "settings.h"
#include "settingsdialog.h"

#define ANIM_FILTER "Animation Files (*.avm *.bvh)"
#define PROP_FILTER "Props (*.prp)"
#define PRECISION   100

#define SVN_ID      "$Id$"

qavimator::qavimator() : QMainWindow(0)
{
  nodeMapping <<  0                             //edu: position
              <<  1 <<  2 <<  3 <<  4 << 5      //edu: hip, abdomen, chest, neck, head
              <<  7 <<  8 <<  9 << 10           //edu: (left) collar, shoulder, fore-arm, hand
              << 12 << 13 << 14 << 15           //edu: ...
              << 17 << 18 << 19
              << 21 << 22 << 23;

  QCoreApplication::setOrganizationName("DeZiRee");
  QCoreApplication::setOrganizationDomain("qavimator.org");
  QCoreApplication::setApplicationName("QAvinmator");

  setupUi(this);

  setWindowTitle("qavimator");
  setAttribute(Qt::WA_DeleteOnClose);
  frameDataValid=false;
  currentPart=0;
  longestRunningTime=0.0;

  // prepare play button icons
  stopIcon=QIcon(":/icons/icons/stop.png");
  playIcon=QIcon(":/icons/icons/play.png");
  loopIcon=QIcon(":/icons/icons/loop.png");

  // playback stopped by default
  setPlaystate(PLAYSTATE_STOPPED);

  readSettings();

  connect(animationView,SIGNAL(partClicked(BVHNode*,
                                           Rotation,
                                           RotationLimits,
                                           Position)),
                     this,SLOT(partClicked(BVHNode*,
                                           Rotation,
                                           RotationLimits,
                                           Position)));

  connect(animationView,SIGNAL(partDragged(BVHNode*,double,double,double)),
                     this,SLOT(partDragged(BVHNode*,double,double,double)));

  connect(animationView,SIGNAL(propClicked(Prop*)),this,SLOT(propClicked(Prop*)));

  connect(animationView,SIGNAL(propDragged(Prop*,double,double,double)),
                     this,SLOT(propDragged(Prop*,double,double,double)));

  connect(animationView,SIGNAL(propScaled(Prop*,double,double,double)),
                     this,SLOT(propScaled(Prop*,double,double,double)));

  connect(animationView,SIGNAL(propRotated(Prop*,double,double,double)),
                     this,SLOT(propRotated(Prop*,double,double,double)));

  connect(animationView,SIGNAL(backgroundClicked()),this,SLOT(backgroundClicked()));
  connect(animationView,SIGNAL(animationSelected(Animation*)),this,SLOT(selectAnimation(Animation*)));

  connect(this,SIGNAL(enablePosition(bool)),positionGroupBox,SLOT(setEnabled(bool)));
  connect(this,SIGNAL(enableRotation(bool)),rotationGroupBox,SLOT(setEnabled(bool)));

  connect(this,SIGNAL(enableProps(bool)),propPositionGroup,SLOT(setEnabled(bool)));
  connect(this,SIGNAL(enableProps(bool)),propScaleGroup,SLOT(setEnabled(bool)));
  connect(this,SIGNAL(enableProps(bool)),propRotationGroup,SLOT(setEnabled(bool)));

  connect(this,SIGNAL(enableProps(bool)),attachToLabel,SLOT(setEnabled(bool)));
  connect(this,SIGNAL(enableProps(bool)),attachToComboBox,SLOT(setEnabled(bool)));

  connect(this,SIGNAL(enableEaseInOut(bool)),easeInOutGroup,SLOT(setEnabled(bool)));

  connect(&timer,SIGNAL(timeout()),this,SLOT(frameTimeout()));

  connect(this,SIGNAL(resetCamera()),animationView,SLOT(resetCamera()));
  connect(this,SIGNAL(protectFrame(bool)),animationView,SLOT(protectFrame(bool)));

  connect(animationView,SIGNAL(partClicked(int)),timelineView,SLOT(selectTrack(int)));
  connect(animationView,SIGNAL(backgroundClicked()),timelineView,SLOT(backgroundClicked()));

  timeline=timelineView->getTimeline();
  connect(timeline,SIGNAL(positionCenter(int)),timelineView,SLOT(scrollTo(int)));
  connect(timeline,SIGNAL(trackClicked(int)),animationView,SLOT(selectPart(int)));

  xRotationSlider->setPageStep(10*PRECISION);
  yRotationSlider->setPageStep(10*PRECISION);
  zRotationSlider->setPageStep(10*PRECISION);
  xPositionSlider->setPageStep(10*PRECISION);
  yPositionSlider->setPageStep(10*PRECISION);
  zPositionSlider->setPageStep(10*PRECISION);

  currentFrameSlider->setPageStep(1);

  if(qApp->argc()>1)
  {
    fileOpen(qApp->argv()[1]);
  }

  // if opening of files didn't work or no files were specified on the
  // command line, open a new one
  if(openFiles.count()==0) fileNew();

  updateInputs();
}

qavimator::~qavimator()
{
  if(timeline) delete timeline;
  fileExit();
}

// FIXME:: implement a static Settings:: class                              //TODO: Yes, move it there
void qavimator::readSettings()
{
  QSettings settings;
  settings.beginGroup("/qavimator");

  // if no settings found, start up with defaults
  int width=850;
  int height=600;
  int figureType=0;
  bool skeleton=false;
  bool showTimelinePanel=true;

  jointLimits=true;
  loop=true;
  protectFirstFrame=true;
  lastPath=QString::null;

  // OpenGL presets
  Settings::setFog(true);
  Settings::setFloorTranslucency(33);

  // defaults for ease in/ease out
  Settings::setEaseIn(false);
  Settings::setEaseOut(false);

  bool settingsFound=settings.value("/settings").toBool();
  if(settingsFound)
  {
    loop=settings.value("/loop").toBool();
    skeleton=settings.value("/skeleton").toBool();
    jointLimits=settings.value("/joint_limits").toBool();
    protectFirstFrame=settings.value("/protect_first_frame").toBool();
    showTimelinePanel=settings.value("/show_timeline").toBool();

    width=settings.value("/mainwindow_width").toInt();
    height=settings.value("/mainwindow_height").toInt();

    lastPath=settings.value("/last_path").toString();

    // OpenGL settings
    Settings::setFog(settings.value("/fog").toBool());
    Settings::setFloorTranslucency(settings.value("/floor_translucency").toInt());

    // settings for ease in/ease outFrame
    Settings::setEaseIn(settings.value("/ease_in").toBool());
    Settings::setEaseOut(settings.value("/ease_out").toBool());

    // sanity
    if(width<50) width=50;
    if(height<50) height=50;

    figureType=settings.value("/figure").toInt();

    //TODO: if(figureType > Animation::NUM_FIGURES) throw new ConfigurationException();

    settings.endGroup();
  }

  resize(width,height);

  optionsLoopAction->setChecked(loop);
  optionsSkeletonAction->setChecked(skeleton);
  optionsJointLimitsAction->setChecked(jointLimits);
  optionsShowTimelineAction->setChecked(showTimelinePanel);

  if(!showTimelinePanel) timelineView->hide();
  // prevent a signal to be sent to yet uninitialized animation view
  optionsProtectFirstFrameAction->blockSignals(true);
  optionsProtectFirstFrameAction->setChecked(protectFirstFrame);
  optionsProtectFirstFrameAction->blockSignals(false);

  figureCombo->setCurrentIndex(figureType);
  setAvatarShape( (Animation::FigureType)figureType );
}

// slot gets called by AnimationView::mousePressEvent()
void qavimator::partClicked(BVHNode* node,Rotation rot,RotationLimits limits,Position pos)
{
  avatarPropsTab->setCurrentIndex(0);
  emit enableProps(false);

  if(!node)
  {
    qDebug("qavimator::partClicked(node,...): node==0!");
    return;
  }

  if(node)
  {
    qDebug("qavimator::partClicked(node): %s",node->name().toLatin1().constData());
    currentPart=node;

    for(int index=0;index<editPartCombo->count();index++)
      if(editPartCombo->itemText(index)==currentPart->name()) editPartCombo->setCurrentIndex(index);

    // do not send signals for moving sliders while changing the slider limits
    xRotationSlider->blockSignals(true);
    yRotationSlider->blockSignals(true);
    zRotationSlider->blockSignals(true);

    // hip gets a full 360 degree limit, all others according to SL.lim
    // maybe this shouldn't be like this to allow for multi rotation spins
    if(currentPart->type==BVH_ROOT)
    {
      xRotationSlider->setRange(-359*PRECISION, 359*PRECISION);
      yRotationSlider->setRange(-359*PRECISION, 359*PRECISION);
      zRotationSlider->setRange(-359*PRECISION, 359*PRECISION);
    }
    else
    {
      xRotationSlider->setRange((int)(limits.xMin*PRECISION),(int)(limits.xMax*PRECISION));
      yRotationSlider->setRange((int)(limits.yMin*PRECISION),(int)(limits.yMax*PRECISION));
      zRotationSlider->setRange((int)(limits.zMin*PRECISION),(int)(limits.zMax*PRECISION));
    }

    // re-enable signals
    xRotationSlider->blockSignals(false);
    yRotationSlider->blockSignals(false);
    zRotationSlider->blockSignals(false);

    setX(rot.x);
    setY(rot.y);
    setZ(rot.z);

//    emit enablePosition(!protect);
    if(node->type==BVH_POS)
      emit enableRotation(false);
    else
      emit enableRotation(!protect);

    setXPos(pos.x);
    setYPos(pos.y);
    setZPos(pos.z);

    // show the user if this part has a key frame here
    updateKeyBtn();
  }
}

// slot gets called by AnimationView::mouseMoveEvent()
void qavimator::partDragged(BVHNode* node,double x,double y,double z)
{
  if(node)
  {
    // check if this frame is protected
    if(!protect)
    {
      // get animation object
      Animation* anim=animationView->getAnimation();
      // get rotation values for selected part
      Rotation rot=anim->getRotation(node);
      // get rotation limits for part
      RotationLimits rotLimits=anim->getRotationLimits(node);

      // calculate new rotation (x, y, z are the modifiers)
      double newX=rot.x+x;
      double newY=rot.y+y;
      double newZ=rot.z+z;

      double xMin=rotLimits.xMin;
      double yMin=rotLimits.yMin;
      double zMin=rotLimits.zMin;
      double xMax=rotLimits.xMax;
      double yMax=rotLimits.yMax;
      double zMax=rotLimits.zMax;

      if(newX<xMin) newX=xMin;
      if(newX>xMax) newX=xMax;
      if(newY<yMin) newY=yMin;
      if(newY>yMax) newY=yMax;
      if(newZ<zMin) newZ=zMin;
      if(newZ>zMax) newZ=zMax;

      setX(newX);
      setY(newY);
      setZ(newZ);

      animationView->getAnimation()->setRotation(node,newX,newY,newZ);
      animationView->repaint();

      updateKeyBtn();
    }
  }
  else qDebug("qavimator::partDragged(): node==0!");
}

// slot gets called by AnimationView::propClicked()
void qavimator::propClicked(Prop* prop)
{
  avatarPropsTab->setCurrentIndex(1);

  // update prop name combo box
  for(int index=0;index<propNameCombo->count();index++)
    if(propNameCombo->itemText(index)==prop->name()) propNameCombo->setCurrentIndex(index);

  // update prop value spinboxes
  selectProp(prop->name());
}

// slot gets called by AnimationView::mouseMoveEvent()
void qavimator::propDragged(Prop* prop,double x,double y,double z)
{
  prop->x+=x;
  prop->y+=y;
  prop->z+=z;
  updatePropSpins(prop);
}

// slot gets called by AnimationView::mouseMoveEvent()
void qavimator::propScaled(Prop* prop,double x,double y,double z)
{
  prop->xs+=x;
  prop->ys+=y;
  prop->zs+=z;
  updatePropSpins(prop);
}

// slot gets called by AnimationView::mouseMoveEvent()
void qavimator::propRotated(Prop* prop,double x,double y,double z)
{
  prop->xr+=x;
  prop->yr+=y;
  prop->zr+=z;
  updatePropSpins(prop);
}

// slot gets called by AnimationView::mouseButtonClicked()
void qavimator::backgroundClicked()
{
  currentPart=0;

  emit enableRotation(false);
  emit enableProps(false);
  emit enableEaseInOut(false);
  editPartCombo->setCurrentIndex(0);
  updateKeyBtn();
}

// slot gets called by body part dropdown list
void qavimator::partChoice()
{
  // get node number from entry list in combo box
  int nodeNumber=nodeMapping[editPartCombo->currentIndex()];
  // selectPart will fire partClicked signal, so we don't bother
  // about updating controls or currentPart pointer ourselves here
  animationView->selectPart(nodeNumber);
  timelineView->selectTrack(nodeNumber);

  animationView->setFocus();
  emit enableProps(false);
}

// gets called whenever a body part rotation slider is moved
void qavimator::rotationSlider(const QObject* slider)
{
  Animation* anim=animationView->getAnimation();
  Rotation rot=anim->getRotation(currentPart);

  double x=rot.x;
  double y=rot.y;
  double z=rot.z;

  if(slider==xRotationSlider)
  {
    x=getX();
    setX(x);
  }
  else if(slider==yRotationSlider)
  {
    y=getY();
    setY(y);
  }
  else if(slider==zRotationSlider)
  {
    z=getZ();
    setZ(z);
  }

  if(animationView->getSelectedPart())
  {
    anim->setRotation(animationView->getSelectedPart(),x,y,z);
    animationView->repaint();
  }

  updateKeyBtn();
}

// gets called whenever a body part rotation value field gets changed
void qavimator::rotationValue()
{
  double x=xRotationEdit->text().toDouble();
  double y=yRotationEdit->text().toDouble();
  double z=zRotationEdit->text().toDouble();

  double min_x=xRotationSlider->minimum()/PRECISION;
  double min_y=yRotationSlider->minimum()/PRECISION;
  double min_z=zRotationSlider->minimum()/PRECISION;

  double max_x=xRotationSlider->maximum()/PRECISION;
  double max_y=yRotationSlider->maximum()/PRECISION;
  double max_z=zRotationSlider->maximum()/PRECISION;

  if(x<min_x) x=min_x;
  if(y<min_y) y=min_y;
  if(z<min_z) z=min_z;

  if(x>max_x) x=max_x;
  if(y>max_y) y=max_y;
  if(z>max_z) z=max_z;

  setX(x);
  setY(y);
  setZ(z);

  Animation* anim=animationView->getAnimation();
  if(animationView->getSelectedPart())
  {
    anim->setRotation(animationView->getSelectedPart(), x, y, z);
    animationView->repaint();
  }

  updateKeyBtn();
}

void qavimator::positionSlider(const QObject* slider)
{
  Animation* anim=animationView->getAnimation();
  Position pos=anim->getPosition();

  double x=pos.x;
  double y=pos.y;
  double z=pos.z;

  if(slider==xPositionSlider)
  {
    x=getXPos();
    setXPos(x);
  }
  else if(slider==yPositionSlider)
  {
    y=getYPos();
    setYPos(y);
  }
  else if(slider==zPositionSlider)
  {
    z=getZPos();
    setZPos(z);
  }

  animationView->getAnimation()->setPosition(x,y,z);
  animationView->repaint();

  updateKeyBtn();
}

void qavimator::positionValue()
{
  qDebug("qavimator::positionValue()");

  double x=xPositionEdit->text().toDouble();
  double y=yPositionEdit->text().toDouble();
  double z=zPositionEdit->text().toDouble();

  double min_x=xPositionSlider->minimum()/PRECISION;
  double min_y=yPositionSlider->minimum()/PRECISION;
  double min_z=zPositionSlider->minimum()/PRECISION;

  double max_x=xPositionSlider->maximum()/PRECISION;
  double max_y=yPositionSlider->maximum()/PRECISION;
  double max_z=zPositionSlider->maximum()/PRECISION;

  if(x<min_x) x=min_x;
  if(y<min_y) y=min_y;
  if(z<min_z) z=min_z;

  if(x>max_x) x=max_x;
  if(y>max_y) y=max_y;
  if(z>max_z) z= max_z;

  setXPos(x);
  setYPos(y);
  setZPos(z);

  animationView->getAnimation()->setPosition(x,y,z);
  animationView->repaint();

  updateKeyBtn();
}

void qavimator::updateInputs()
{
  // deactivate redraw to reduce interface "jitter" during updating
  setUpdatesEnabled(false);

  Animation* anim=animationView->getAnimation();

  if(anim && currentPart)
  {
    Rotation rot=anim->getRotation(currentPart);

    double x=rot.x;
    double y=rot.y;
    double z=rot.z;

    RotationLimits rotLimits=anim->getRotationLimits(currentPart);

    double xMin=rotLimits.xMin;
    double yMin=rotLimits.yMin;
    double zMin=rotLimits.zMin;
    double xMax=rotLimits.xMax;
    double yMax=rotLimits.yMax;
    double zMax=rotLimits.zMax;

    if(currentPart->type==BVH_ROOT)
    {
      xRotationSlider->setRange(-359*PRECISION, 359*PRECISION);
      yRotationSlider->setRange(-359*PRECISION, 359*PRECISION);
      zRotationSlider->setRange(-359*PRECISION, 359*PRECISION);
    }
    else
    {
      xRotationSlider->setRange((int)(xMin*PRECISION), (int)(xMax*PRECISION));
      yRotationSlider->setRange((int)(yMin*PRECISION), (int)(yMax*PRECISION));
      zRotationSlider->setRange((int)(zMin*PRECISION), (int)(zMax*PRECISION));
    }

    setX(x);
    setY(y);
    setZ(z);
  }
  else
    emit enableRotation(false);

  emit enablePosition(!protect);
  Position pos=anim->getPosition();

  setXPos(pos.x);
  setYPos(pos.y);
  setZPos(pos.z);

// we do that in nextPlaystate() now
//  playButton->setIcon(playing ? stopIcon : playIcon);

  framesSpin->setValue(anim->getNumberOfFrames());
  currentFrameSlider->setMaximum(anim->getNumberOfFrames()-1);
  fpsSpin->setValue(anim->fps());

  // prevent feedback loop
  scaleSpin->blockSignals(true);
  scaleSpin->setValue(anim->getAvatarScale()*100.0+0.1);  // +0.1 to overcome rounding errors
  scaleSpin->blockSignals(false);

  updateKeyBtn();

  if(playstate==PLAYSTATE_STOPPED)
    emit enableInputs(true);
  else
    emit enableInputs(false);

  if (frameDataValid)
    editPasteAction->setEnabled(true);
  else
    editPasteAction->setEnabled(false);

  if(propNameCombo->count())
    emit enableProps(true);
  else
    emit enableProps(false);

  // reactivate redraw
  setUpdatesEnabled(true);
}

void qavimator::updateKeyBtn()
{
  Animation* anim=animationView->getAnimation();
  qDebug("qavimator::updateKeyBtn(): anim=%lx",(unsigned long) anim);

  // make sure we don't send a toggle signal on display update
  keyframeButton->blockSignals(true);

  int frame=anim->getFrame();
  int partIndex=animationView->getSelectedPartIndex();
  bool hasKeyframe=anim->isKeyFrame(partIndex,frame);
  keyframeButton->setDown(hasKeyframe);

  // re-enable toggle signal
  keyframeButton->blockSignals(false);

  if(hasKeyframe && currentPart)
  {
    emit enableEaseInOut(true);
    easeInCheck->setChecked(anim->easeIn(currentPart,anim->getFrame()));
    easeOutCheck->setChecked(anim->easeOut(currentPart,anim->getFrame()));
  }
  else
    emit enableEaseInOut(false);

//  timeline->repaint();
}

void qavimator::enableInputs(bool state)
{
  // protection overrides state for keyframe button
  if(protect) state=false;
  keyframeButton->setEnabled(state);

  // do not enable rotation if we have no part selected
  if(!currentPart) state=false;
  emit enableRotation(state);
}

void qavimator::frameTimeout()
{
  // only if we are still playing
  if(playstate!=PLAYSTATE_STOPPED)
  {
    Animation* anim=animationView->getAnimation();
    if(anim)
    {
      // don't show protected frames color on playback to avoid flicker
      emit protectFrame(false);
      // cycle through frames, restart at looping point
      animationView->stepForward();

      if(anim->getFrame()==(anim->getNumberOfFrames()-1) && playstate==PLAYSTATE_PLAYING)
      {
        timer.stop();
        setPlaystate(PLAYSTATE_STOPPED);
      }

      updateInputs();
      return;
    }
  }
}

void qavimator::nextPlaystate()
{
//  qDebug("qavimator::nextPlaystate(): current playstate %d",(int) playstate);

  Animation* anim=animationView->getAnimation();

  switch(playstate)
  {
    case PLAYSTATE_STOPPED:
    {
      // if we're suposed to loop and the current frame is not past loop out point
      if(loop && anim->getFrame()<anim->getLoopOutPoint())
      {
        // start looping animation
        setPlaystate(PLAYSTATE_LOOPING);
        anim->setLoop(true);
      }
      else
      {
        // start one-shot animation
        setPlaystate(PLAYSTATE_PLAYING);
        anim->setLoop(false);
      }

      timer.start((int)(1.0/anim->fps()*1000.0));
      break;
    }
    case PLAYSTATE_LOOPING:
    {
      setPlaystate(PLAYSTATE_PLAYING);
      anim->setLoop(false);
      break;
    }
    case PLAYSTATE_PLAYING:
    {
      // take care of locks, key frames ...
      frameSlider(currentFrameSlider->value());
      setPlaystate(PLAYSTATE_STOPPED);

      break;
    }
    default:
      qDebug("qavimator::nextPlaystate(): unknown playstate %d",(int) playstate);
  }

  updateInputs();
}

void qavimator::setFPS(int fps)
{
  qDebug("qavimator::setFPS(%d)",fps);

  Animation* anim=animationView->getAnimation();
  if(!anim) return;

  // sanity check
  if(fps<1) fps=1;
  else if(fps>50) fps=50;

  anim->setFPS(fps);
  calculateLongestRunningTime();
}

void qavimator::frameSlider(int position)
{
  // check if we are at the first frame and if it's protected
  if(position==0 && protectFirstFrame) protect=true;
  else protect=false;

  emit protectFrame(protect);
  setPlaystate(PLAYSTATE_STOPPED);
  animationView->setFrame(position);

  updateInputs();
}

void qavimator::setAvatarShape(/*int shape*/ Animation::FigureType shape)                               //TODO: "int shape" to "enum FigureType"?
{
  Animation* anim=animationView->getAnimation();
  if(!anim) return;

/*  if(shape==0)
    anim->setFigureType(Animation::FIGURE_FEMALE);
  else
    anim->setFigureType(Animation::FIGURE_MALE);        */

  anim->setFigureType(shape);

  animationView->repaint();
}

void qavimator::setAvatarScale(int percent)
{
  animationView->getAnimation()->setAvatarScale(percent/100.0);
  animationView->repaint();
}

void qavimator::numFramesChanged(int num)
{
  if(num<1) num=1;
  Animation* anim=animationView->getAnimation();
  anim->setNumberOfFrames(num);
  calculateLongestRunningTime();

  // re-check loop boundaries (if loop set at all)
  if(anim->getLoopInPoint()!=-1)
  {
    setLoopInPoint(anim->getLoopInPoint()+1);
    setLoopOutPoint(anim->getLoopOutPoint()+1);
  }

  updateInputs();
}

void qavimator::easeInChanged(int change)
{
  bool ease=false;
  if(change==Qt::Checked) ease=true;

  Animation* anim=animationView->getAnimation();
  anim->setEaseIn(currentPart,anim->getFrame(),ease);
}

void qavimator::easeOutChanged(int change)
{
  bool ease=false;
  if(change==Qt::Checked) ease=true;

  Animation* anim=animationView->getAnimation();
  anim->setEaseOut(currentPart,anim->getFrame(),ease);
}

// ------ Menu Action Slots (Callbacks) -----------

// Menu action: File / New
void qavimator::fileNew()
{
  clearProps();
  if(!clearOpenFiles()) return;

  Animation* anim=new Animation(animationView->getBVH());

  // set timeline animation first, because ...
  timeline->setAnimation(anim);
  // ... setting animation here will delete all old animations
  animationView->setAnimation(anim);
  selectAnimation(anim);

  // add new animation to internal list
  animationIds.append(anim);
  calculateLongestRunningTime();
  // add new animation to combo box
  addToOpenFiles(UNTITLED_NAME);

  anim->useRotationLimits(jointLimits);

  if(protectFirstFrame)
  {
//    qDebug("qavimator::fileNew(): adding loop points for protected frame 1 animation");
    // skip first frame, since it's protected anyway
    anim->setFrame(1);
    setCurrentFrame(1);
    setLoopInPoint(2);
  }
  else
  {
//    qDebug("qavimator::fileNew(): adding loop points for unprotected frame 1 animation");
    anim->setFrame(0);
    setCurrentFrame(0);
    setLoopInPoint(1);
  }
  setLoopOutPoint(anim->getNumberOfFrames());

  // show frame as unprotected
  emit protectFrame(false);
  protect=false;

  // FIXME: code duplication
  connect(anim,SIGNAL(currentFrame(int)),this,SLOT(setCurrentFrame(int)));

  editPartCombo->setCurrentIndex(1);

  setPlaystate(PLAYSTATE_STOPPED);

  updateInputs();
  updateFps();

  emit enableRotation(false);
  emit enablePosition(true);
  emit enableProps(false);

  anim->setDirty(false);
}

QString qavimator::selectFileToOpen(const QString& caption)
{
   //// For some unknown reason passing "this" locks up the OSX qavimator window. Possibly a QT4 bug, needs investigation
#ifdef __APPLE__
   QString file=QFileDialog::getOpenFileName(NULL,caption,lastPath,ANIM_FILTER);
#else
   QString file=QFileDialog::getOpenFileName(this,caption,lastPath,ANIM_FILTER);
#endif
  if(!file.isEmpty())
  {
    QFileInfo fileInfo(file);
    if(!fileInfo.exists())
    {
      QMessageBox::warning(this,QObject::tr("Load Animation File"),QObject::tr("<qt>Animation file not found:<br />%1</qt>").arg(file));
      file=QString::null;
    }
    else
      lastPath=fileInfo.path();
  }

  return file;
}

// Menu action: File / Open ...
void qavimator::fileOpen()
{
  fileOpen(QString::null);
}

void qavimator::fileOpen(const QString& name)
{
  QString file=name;

  if(file.isEmpty())
    file=selectFileToOpen(tr("Select Animation File to Load"));

  if(!file.isEmpty())
  {
    clearProps();
    if(!clearOpenFiles()) return;
    fileAdd(file);
  }

  // update timeline and animation view with the currently selected body part
  // this helps to sync the visual selections
  partChoice();
}

// Menu action: File / Add New Animation ...
void qavimator::fileAdd()
{
  fileAdd(QString::null);
}

void qavimator::fileAdd(const QString& name)
{
  QString file=name;

  if(file.isEmpty())
    file=selectFileToOpen(tr("Select Animation File to Add"));

  if(!file.isEmpty())
  {
    // handling of non-existant file names
    if(!QFile::exists(file))
    {
      QMessageBox::warning(this,QObject::tr("Load Animation File"),QObject::tr("<qt>Animation file not found:<br />%1</qt>").arg(file));
      return;
    }
    addToOpenFiles(file);
    Animation* anim=new Animation(animationView->getBVH(),file);
    animationIds.append(anim);
    calculateLongestRunningTime();

    setCurrentFile(file);

    animationView->addAnimation(anim);
    timeline->setAnimation(anim);
    selectAnimation(anim);
    anim->useRotationLimits(jointLimits);

//    qDebug("qavimator::fileAdd(): checking for loop points");
    // no loop in point? must be a BVH or an older avm. set a sane default
    if(anim->getLoopInPoint()==-1)
    {
//      qDebug("qavimator::fileAdd(): no loop points, adding new");
      // first set loop out point to avoid clamping of loop in point
      setLoopOutPoint(anim->getNumberOfFrames());

      if(protectFirstFrame)
      {
//        qDebug("qavimator::fileAdd(): adding loop points for protected frame 1 animation");
        anim->setFrame(1);
        setCurrentFrame(1);
        setLoopInPoint(2);
      }
      else
      {
//        qDebug("qavimator::fileAdd(): adding loop points for unprotected frame 1 animation");
        anim->setFrame(0);
        setCurrentFrame(0);
        setLoopInPoint(1);
      }
    }
    else
    {
//      qDebug("qavimator::fileAdd(): reading saved loop points");
      setLoopInPoint(anim->getLoopInPoint()+1);
      setLoopOutPoint(anim->getLoopOutPoint()+1);
    }

    // FIXME: code duplication
    connect(anim,SIGNAL(currentFrame(int)),this,SLOT(setCurrentFrame(int)));

    animationView->selectPart(nodeMapping[editPartCombo->currentIndex()]);
    updateInputs();
    updateFps();
    anim->setDirty(false);

// makeshift tool for new independant playback testing
// anim->setPlaystate(PLAYSTATE_LOOPING);

  }
}

// Menu Action: File / Save
void qavimator::fileSave()
{
  if(currentFile==UNTITLED_NAME)
    fileSaveAs();
  else
    animationView->getAnimation()->saveBVH(currentFile);
}

// Menu Action: File / Save As...
void qavimator::fileSaveAs()
{
   //// For some unknown reason passing "this" locks up the OSX qavimator window. Possibly a QT4 bug, needs investigation
#ifdef __APPLE__
   QString file=QFileDialog::getSaveFileName(NULL,tr("Save Animation File"),currentFile,ANIM_FILTER,0,QFileDialog:: DontConfirmOverwrite);
#else
   QString file=QFileDialog::getSaveFileName(this,tr("Save Animation File"),currentFile,ANIM_FILTER,0,QFileDialog:: DontConfirmOverwrite);
#endif

  if(!file.isEmpty())
  {
    QFileInfo fileInfo(file);

    // make sure file has proper extension (either .bvh or .avm)
    QString extension=fileInfo.suffix().toLower();
    if(extension!="avm" && extension!="bvh")
      file+=".avm";

    // if the file didn't exist yet or the user accepted to overwrite it, save it.
    if(checkFileOverwrite(fileInfo))
    {
      setCurrentFile(file);
      lastPath=fileInfo.path();
      animationView->getAnimation()->saveBVH(file);
      // update animation selector combo box
      selectAnimationCombo->setItemText(selectAnimationCombo->currentIndex(),fileInfo.baseName());
      openFiles[selectAnimationCombo->currentIndex()]=file;
      fileExportForSecondLifeAction->setEnabled(true);
    }
  }
}

// Menu Action: File / Export For Second Life
void qavimator::fileExportForSecondLife()
{
  // FIXME: think of a sensible thing to do when the animation has not been saved
  //        as .avm yet
  if(currentFile!=UNTITLED_NAME)
  {
    QFileInfo fileInfo(currentFile);
    QString exportName=fileInfo.path()+"/"+fileInfo.baseName()+".bvh";

    qDebug("qavimator::fileExportForSecondLife(): exporting animation as '%s'.",exportName.toLatin1().constData());
    animationView->getAnimation()->saveBVH(exportName);
    QMessageBox::information(this,QObject::tr("Export for Second Life"),QObject::tr("Animation was exported for Second Life as:\n%1").arg(exportName));
  }
}

// Menu Action: File / Load Props...
void qavimator::fileLoadProps()
{
   //// For some unknown reason passing "this" locks up the OSX qavimator window. Possibly a QT4 bug, needs investigation
#ifdef __APPLE__
   QString fileName=QFileDialog::getOpenFileName(NULL,QString(),lastPath,PROP_FILTER);
#else
   QString fileName=QFileDialog::getOpenFileName(this,QString(),lastPath,PROP_FILTER);
#endif

  if(!fileName.isEmpty())
  {
    QFileInfo fileInfo(fileName);

    if(fileInfo.exists())
    {
      clearProps();
      QFile file(fileName);
      if(file.open(QIODevice::ReadOnly))
      {
        QString line;
        bool tooOldWarning=false;
        while(!file.atEnd())
        {
          qDebug("qavimator::fileLoadProps(): loading prop");

          line=file.readLine(2048);
          QStringList props=line.split(' ');

          // make sure we don't crash with an assert if the array is too short (e.g. attachment point missing)
          if(props.count()!=11)
          {
            // show this warning only once per .prp file
            if(!tooOldWarning)
            {
              qDebug("qavimator::fileLoadProps(): Prop file either too old or corrupted. Trying to recover.");
              tooOldWarning=true;
            }
            // append the necessary number of "0" to the array
            while(props.count()<11) props.append("0");
          }

          const Prop* prop=animationView->addProp((Prop::PropType) props[0].toInt(),
                                            props[1].toDouble(),
                                            props[2].toDouble(),
                                            props[3].toDouble(),
                                            props[4].toDouble(),
                                            props[5].toDouble(),
                                            props[6].toDouble(),
                                            props[7].toDouble(),
                                            props[8].toDouble(),
                                            props[9].toDouble(),
                                            props[10].toInt()
          );
          if(prop)
          {
            propNameCombo->addItem(prop->name());
            propNameCombo->setCurrentIndex(propNameCombo->count()-1);
            selectProp(prop->name());
          }
        } // while
      }
    }
  }
}

// Menu Action: File / Save Props...
void qavimator::fileSaveProps()
{
   //// For some unknown reason passing "this" locks up the OSX qavimator window. Possibly a QT4 bug, needs investigation
#ifdef __APPLE__
   QString fileName=QFileDialog::getSaveFileName(NULL,tr("Save Props"),currentFile,PROP_FILTER);
#else
   QString fileName=QFileDialog::getSaveFileName(this,tr("Save Props"),currentFile,PROP_FILTER);
#endif

  if(!fileName.isEmpty())
  {
    QFileInfo fileInfo(fileName);
    // make sure file has proper extension (.prp)
    if(fileInfo.suffix().toLower()!="prp")
      fileName+=".prp";

    // check if file exists
    // Obsolete for Qt 4.x
    if(!checkFileOverwrite(fileInfo)) return;

    QFile file(fileName);
    if(file.open(QIODevice::WriteOnly))
    {
      for(int index=0;index<propNameCombo->count();index++)
      {
        Prop* prop=animationView->getPropByName(propNameCombo->itemText(index));
        QStringList properties;
        properties.append(QString::number(prop->type));
        properties.append(QString::number(prop->x));
        properties.append(QString::number(prop->y));
        properties.append(QString::number(prop->z));
        properties.append(QString::number(prop->xs));
        properties.append(QString::number(prop->ys));
        properties.append(QString::number(prop->zs));
        properties.append(QString::number(prop->xr));
        properties.append(QString::number(prop->yr));
        properties.append(QString::number(prop->zr));
        properties.append(QString::number(prop->isAttached()));
        QString line=properties.join(" ")+"\n";
        file.write(line.toLatin1().constData(),line.length());
      } // for
    }
  }
}

// Menu Action: File / Exit
void qavimator::fileExit()
{
  if(!clearOpenFiles())
    return;

  QSettings settings;
  settings.beginGroup("/qavimator");

  // make sure we know next time, that there actually was a settings file
  settings.setValue("/settings",true);

  settings.setValue("/loop",loop);
  settings.setValue("/skeleton",optionsSkeletonAction->isChecked());
  settings.setValue("/joint_limits",optionsJointLimitsAction->isChecked());
  settings.setValue("/protect_first_frame",optionsProtectFirstFrameAction->isChecked());
  settings.setValue("/show_timeline",optionsShowTimelineAction->isChecked());

  settings.setValue("/figure",figureCombo->currentIndex());
  settings.setValue("/mainwindow_width",size().width());
  settings.setValue("/mainwindow_height",size().height());

  settings.setValue("/last_path",lastPath);

  // OpenGL settings
  settings.setValue("/fog",Settings::fog());
  settings.setValue("/floor_translucency",Settings::floorTranslucency());

  // settings for ease in/ease outFrame
  settings.setValue("/ease_in",Settings::easeIn());
  settings.setValue("/ease_out",Settings::easeOut());

  settings.endGroup();

  // remove all widgets and close the main form
  qApp->exit(0);
}

// Menu Action: Edit / Cut
void qavimator::editCut()
{
//  qDebug("qavimator::editCut()");
  animationView->getAnimation()->cutFrame();
  frameDataValid=true;
  updateInputs();
}

// Menu Action: Edit / Copy
void qavimator::editCopy()
{
  animationView->getAnimation()->copyFrame();
  frameDataValid=true;
  updateInputs();
}

// Menu Action: Edit / Paste
void qavimator::editPaste()
{
  if(frameDataValid)
  {
    animationView->getAnimation()->pasteFrame();
    animationView->repaint();
    updateInputs();
  }
}

// Menu Action: Edit / Paste
void qavimator::toolsOptimizeBVH()
{
  animationView->getAnimation()->optimize();
  updateInputs();
}

// Menu Action: Options / Skeleton
void qavimator::showSkeleton(bool on)
{
  if(on)
    animationView->showSkeleton();
  else
    animationView->hideSkeleton();
}

// Menu Action: Options / Loop
void qavimator::setLoop(bool on)
{
  loop=on;

  // update play state
  if(playstate==PLAYSTATE_LOOPING)
    setPlaystate(PLAYSTATE_PLAYING);
  else
    // just update play button icon (in case we're stopped it changes from/to looping icon)
    setPlaystate(playstate);
}

// Menu Action: Options / Joint Limits
void qavimator::setJointLimits(bool on)
{
  jointLimits=on;
  Animation* anim=animationView->getAnimation();
  if(anim)
  {
    anim->useRotationLimits(on);
    animationView->repaint();
    updateInputs();
  }
}

// Menu Action: Options / Protect First Frame
void qavimator::setProtectFirstFrame(bool on)
{
  protectFirstFrame=on;
  if(on && currentFrameSlider->value()==0) protect=true;
  else protect=false;

  emit protectFrame(protect);
  updateInputs();
}

// Menu Action: Options / Show Timeline
void qavimator::showTimeline(bool on)
{
  if(on)
    timelineView->show();
  else
    timelineView->hide();

  // hack to get 3D view back in shape
  qApp->processEvents();
  QSize oldSize=animationView->size();
  animationView->resize(oldSize.width(),oldSize.height()-1);
  qApp->processEvents();
  animationView->resize(oldSize);
}

// Menu Action: Options / Configure QAvimator
void qavimator::configure()
{
  SettingsDialog* dialog=new SettingsDialog(this);
  connect(dialog,SIGNAL(configChanged()),this,SLOT(configChanged()));

  dialog->exec();

  delete dialog;
}

void qavimator::configChanged()
{
  animationView->repaint();
}

// Menu Action: Help / About ...
void qavimator::helpAbout()
{
  QMessageBox::about(this,QObject::tr("About QAvimator"),QObject::tr("QAvimator - Animation editor for Second Life<br />%1").arg(SVN_ID));
}

// checks if a file already exists at the given path and displays a warning message
// returns true if it's ok to save/overwrite, else returns false
bool qavimator::checkFileOverwrite(const QFileInfo& fileInfo)
{
  // get file info
  if(fileInfo.exists())
  {
    int answer=QMessageBox::question(this,tr("File Exists"),tr("A file with the name \"%1\" does already exist. Do you want to overwrite it?").arg(fileInfo.fileName()),QMessageBox::Yes,QMessageBox::No,QMessageBox::NoButton);
    if(answer==QMessageBox::No) return false;
  }
  return true;
}

void qavimator::setX(float x)
{
  setSliderValue(xRotationSlider,xRotationEdit,x);
}

void qavimator::setY(float y)
{
  setSliderValue(yRotationSlider,yRotationEdit,y);
}

void qavimator::setZ(float z)
{
  setSliderValue(zRotationSlider,zRotationEdit,z);
}

float qavimator::getX()
{
  return xRotationSlider->value()/PRECISION;
}

float qavimator::getY()
{
  return yRotationSlider->value()/PRECISION;
}

float qavimator::getZ()
{
  return zRotationSlider->value()/PRECISION;
}

void qavimator::setXPos(float x)
{
  setSliderValue(xPositionSlider,xPositionEdit,x);
}

void qavimator::setYPos(float y)
{
  setSliderValue(yPositionSlider,yPositionEdit,y);
}

void qavimator::setZPos(float z)
{
  setSliderValue(zPositionSlider,zPositionEdit,z);
}

float qavimator::getXPos()
{
  return xPositionSlider->value()/PRECISION;
}

float qavimator::getYPos()
{
  return yPositionSlider->value()/PRECISION;
}

float qavimator::getZPos()
{
  return zPositionSlider->value()/PRECISION;
}

// helper function to prevent feedback between the two widgets
void qavimator::setSliderValue(QSlider* slider,QLineEdit* edit,float value)
{
  slider->blockSignals(true);
  edit->blockSignals(true);
  slider->setValue((int)(value*PRECISION));
  edit->setText(QString::number(value));
  edit->blockSignals(false);
  slider->blockSignals(false);
}

void qavimator::updateFps()
{
  int fps=animationView->getAnimation()->fps();

  // guard against division by zero
  if(fps)
  {
    // don't send FPS change back to Animation object
    framesSpin->blockSignals(true);
    fpsSpin->setValue(fps);
    // re-enable FPS signal
    framesSpin->blockSignals(false);
  }
}

// Adds a file to the open files list, and adds the entry in the combo box
void qavimator::addToOpenFiles(const QString& fileName)
{
    openFiles.append(fileName);

    QString fixedName=fileName;
    QRegExp pattern(".*/");
    fixedName.remove(pattern);
    pattern.setPattern("(\\.bvh|\\.avm)");
    fixedName.remove(pattern);

    selectAnimationCombo->addItem(fixedName);
}

void qavimator::removeFromOpenFiles(unsigned int which)
{
  if(which>= (unsigned int) openFiles.count()) return;
  openFiles.removeAt(which);
  selectAnimationCombo->removeItem(which);
}

// empty out the open files list
bool qavimator::clearOpenFiles()
{
  for(unsigned int index=0;index< (unsigned int) animationIds.count();index++)
  {
    if(animationIds.at(index)->dirty())
    {
      int answer=QMessageBox::question(this,tr("Unsaved Changes"),tr("There are some unsaved changes. Are you sure you want to continue and lose all unsaved data?"),QMessageBox::Yes,QMessageBox::No,QMessageBox::NoButton);
      if(answer==QMessageBox::No)
        return false;
      else
        break;
    }
  }

  timeline->setAnimation(0);
  animationView->clear();
  openFiles.clear();
  selectAnimationCombo->clear();
  animationIds.clear();
  setCurrentFile(UNTITLED_NAME);
  longestRunningTime=0.0;

  return true;
}

// convenience function to set window title in a defined way
void qavimator::setCurrentFile(const QString& fileName)
{
  currentFile=fileName;
  setWindowTitle("qavimator ["+currentFile+"]");
}

// this slot gets called from Animation::setFrame(int)
void qavimator::setCurrentFrame(int frame)
{
  // make sure current frame is only updated when no animation is playing (manual change,
  // program startup) or that only the currently selected animation updates the frame
  // position, so we don't have jumping back and forth while playing multiple animations
  if(playstate==PLAYSTATE_STOPPED || sender()==animationIds.at(selectAnimationCombo->currentIndex()))
  {
    currentFrameSlider->blockSignals(true);
    currentFrameSlider->setValue(frame);
    currentFrameSlider->blockSignals(false);
    currentFrameLabel->setText(QString::number(frame+1));

    timeline->setCurrentFrame(frame);
//  animationView->setFrame(frame);
    // check if we are at the first frame and if it's protected
    if(frame==0 && protectFirstFrame) protect=true;
    else protect=false;
    emit protectFrame(protect);
    updateInputs();
    updateKeyBtn();
  }
}

// this slot gets called when someone clicks one of the "New Prop" buttons
void qavimator::newProp(Prop::PropType type)
{
  const Prop* prop=animationView->addProp(type,10,40,10, 10,10,10, 0,0,0, 0);

  if(prop)
  {
    propNameCombo->addItem(prop->name());
    propNameCombo->setCurrentIndex(propNameCombo->count()-1);
    selectProp(prop->name());
    attachToComboBox->setCurrentIndex(0);
  }
}

void qavimator::selectProp(const QString& propName)
{
  const Prop* prop=animationView->getPropByName(propName);
  if(prop)
  {
    emit enableProps(true);
    emit enableRotation(false);
    emit enablePosition(false);
    propNameCombo->setEnabled(true);
    deletePropButton->setEnabled(true);

    updatePropSpins(prop);
    animationView->selectProp(prop->name());
    attachToComboBox->setCurrentIndex(prop->isAttached());
  }
  else
  {
    emit enableProps(false);
    propNameCombo->setEnabled(false);
    deletePropButton->setEnabled(false);
  }
}

void qavimator::attachProp(int attachmentPoint)
{
  // FIXME: find better solution for filtering endpoint for joints
  if(attachToComboBox->currentText()=="-") attachmentPoint=0;
  QString propName=propNameCombo->currentText();
  Prop* prop=animationView->getPropByName(propName);
  prop->attach(attachmentPoint);
  updatePropSpins(prop);
  animationView->repaint();
}

void qavimator::updatePropSpins(const Prop* prop)
{
  propXPosSpin->blockSignals(true);
  propYPosSpin->blockSignals(true);
  propZPosSpin->blockSignals(true);

  propXPosSpin->setValue((int)(prop->x));
  propYPosSpin->setValue((int)(prop->y));
  propZPosSpin->setValue((int)(prop->z));

  propXPosSpin->blockSignals(false);
  propYPosSpin->blockSignals(false);
  propZPosSpin->blockSignals(false);

  propXRotSpin->blockSignals(true);
  propYRotSpin->blockSignals(true);
  propZRotSpin->blockSignals(true);

  propXRotSpin->setValue((int)(360+prop->xr) % 360);
  propYRotSpin->setValue((int)(360+prop->yr) % 360);
  propZRotSpin->setValue((int)(360+prop->zr) % 360);

  propXRotSpin->blockSignals(false);
  propYRotSpin->blockSignals(false);
  propZRotSpin->blockSignals(false);

  propXScaleSpin->blockSignals(true);
  propYScaleSpin->blockSignals(true);
  propZScaleSpin->blockSignals(true);

  propXScaleSpin->setValue((int)(prop->xs));
  propYScaleSpin->setValue((int)(prop->ys));
  propZScaleSpin->setValue((int)(prop->zs));

  propXScaleSpin->blockSignals(false);
  propYScaleSpin->blockSignals(false);
  propZScaleSpin->blockSignals(false);
}

// gets called whenever one of the prop position values gets changed
void qavimator::propPositionChanged()
{
  QString propName=propNameCombo->currentText();
  Prop* prop=animationView->getPropByName(propName);
  if(prop)
  {
    prop->setPosition(propXPosSpin->value(),propYPosSpin->value(),propZPosSpin->value());
    animationView->repaint();
  }
}

// gets called whenever one of the prop scale values gets changed
void qavimator::propScaleChanged()
{
  QString propName=propNameCombo->currentText();
  Prop* prop=animationView->getPropByName(propName);
  if(prop)
  {
    prop->setScale(propXScaleSpin->value(),propYScaleSpin->value(),propZScaleSpin->value());
    animationView->repaint();
  }
}

// gets called whenever one of the prop rotation values gets changed
void qavimator::propRotationChanged()
{
  QString propName=propNameCombo->currentText();
  Prop* prop=animationView->getPropByName(propName);
  if(prop)
  {
    prop->setRotation(propXRotSpin->value(),propYRotSpin->value(),propZRotSpin->value());
    animationView->repaint();
  }
}

void qavimator::deleteProp()
{
  QString propName=propNameCombo->currentText();
  Prop* prop=animationView->getPropByName(propName);
  if(prop)
  {
    animationView->deleteProp(prop);
    for(int index=0;index<propNameCombo->count();index++)
      if(propNameCombo->itemText(index)==propName)
    {
      propNameCombo->removeItem(index);
      selectProp(propNameCombo->currentText());
    }
  }
}

void qavimator::clearProps()
{
  animationView->clearProps();
  propNameCombo->clear();
  selectProp(QString::null);
}

// gets called by selecting an animation from the animation combo box
void qavimator::animationChanged(int which)
{
  qDebug("qavimator::animationChanged(%d)",which);

  // safety to check if "which" is out of bounds of loaded animations
  if(which>=openFiles.count()) return;

  // get animation pointer
  Animation* anim=animationIds.at(which);
  // select animation (will also update combo box, but better than duplicate code)
  selectAnimation(anim);
}

// gets called from AnimationView::animationSelected()
void qavimator::selectAnimation(Animation* animation)
{
  // find animation index in list of open files
  for(unsigned int index=0;index< (unsigned int) animationIds.count();index++)
  {
    // index found
    if(animationIds.at(index)==animation)
    {
      // prevent signal looping
      animationView->blockSignals(true);
      // update animation combo box
      selectAnimationCombo->setCurrentIndex(index);
      // update window title
      setCurrentFile(openFiles.at(index));
      // update animation view (might already be active, depending on how this function was called)
      animationView->selectAnimation(index);
      // re-enable signals
      animationView->blockSignals(false);
    }
  } // for

  // update avatar figure combo box
  int figure=0;
  if(animation->getFigureType()==Animation::FIGURE_MALE) figure=1;
  figureCombo->setCurrentIndex(figure);

  // update timeline
  timeline->setAnimation(animation);
  updateInputs();

  // enable export to second life if current file name is not the default untitled name
  fileExportForSecondLifeAction->setEnabled(!(currentFile==UNTITLED_NAME));
}

// set loop in point (user view, so always +1)
void qavimator::setLoopInPoint(int inFrame)
{
  Animation* anim=animationView->getAnimation();
  int numOfFrames=anim->getNumberOfFrames();
  int outFrame=anim->getLoopOutPoint();
  qDebug("qavimator::setLoopInPoint(%d) (%d frames)",inFrame,numOfFrames);

  if(inFrame>numOfFrames) inFrame=numOfFrames;
  if(inFrame>outFrame) inFrame=outFrame+1;
  if(inFrame<1) inFrame=1;

  anim->setLoopInPoint(inFrame-1);

  loopInSpinBox->blockSignals(true);
  loopInSpinBox->setValue(inFrame);
  loopInSpinBox->blockSignals(false);

  loopInPercentLabel->setText(QString("(%1%)").arg(inFrame*100/numOfFrames));
}

// set loop out point (user view, so always +1)
void qavimator::setLoopOutPoint(int outFrame)
{
  Animation* anim=animationView->getAnimation();
  int numOfFrames=anim->getNumberOfFrames();
  int inFrame=anim->getLoopInPoint();
  qDebug("qavimator::setLoopOutPoint(%d) (%d frames)",outFrame,numOfFrames);

  if(outFrame>numOfFrames) outFrame=numOfFrames;
  if((outFrame-1)<inFrame) outFrame=inFrame+1;
  if(outFrame<1) outFrame=1;

  anim->setLoopOutPoint(outFrame-1);

  loopOutSpinBox->blockSignals(true);
  loopOutSpinBox->setValue(outFrame);
  loopOutSpinBox->blockSignals(false);

  loopOutPercentLabel->setText(QString("(%1%)").arg(outFrame*100/numOfFrames));
}

void qavimator::setPlaystate(PlayState state)
{
//  qDebug("qavimator::setPlaystate(): setting playstate %d",(int) state);
  // set state
  playstate=state;

  // set play button icons according to play state
  if(state==PLAYSTATE_STOPPED)
    playButton->setIcon(loop ? loopIcon : playIcon);
  else if(state==PLAYSTATE_LOOPING)
    playButton->setIcon(playIcon);
  else if(state==PLAYSTATE_PLAYING)
    playButton->setIcon(stopIcon);
  else
    qDebug("qavimator::setPlaystate(): unknown playstate %d",(int) state);
}

// prevent closing of main window if there are unsaved changes
void qavimator::closeEvent(QCloseEvent* event)
{
  if(!clearOpenFiles())
    event->ignore();
  else
    event->accept();
}

// calculates the longest running time of all animations
double qavimator::calculateLongestRunningTime()
{
  qDebug("qavimator::calculateLongestRunningTime()");
  longestRunningTime=0.0;
  for(unsigned int index=0;index< (unsigned int) animationIds.count();index++)
  {
    Animation* anim=animationIds.at(index);
    double time=anim->getNumberOfFrames()/((double) anim->fps());
    if(time>longestRunningTime) longestRunningTime=time;
  }
  qDebug("qavimator::calculateLongestRunningTime(): longestRunningTime now: %f seconds",longestRunningTime);
  return longestRunningTime;
}

// -------------------------------------------------------------------------
// autoconnection from designer UI

// ------- Menu Action Slots --------

void qavimator::on_fileNewAction_triggered()
{
  fileNew();
}

void qavimator::on_fileOpenAction_triggered()
{
  fileOpen();
}

void qavimator::on_fileAddAction_triggered()
{
  fileAdd();
}

void qavimator::on_fileSaveAction_triggered()
{
  fileSave();
}

void qavimator::on_fileSaveAsAction_triggered()
{
  fileSaveAs();
}

void qavimator::on_fileExportForSecondLifeAction_triggered()
{
  fileExportForSecondLife();
}

void qavimator::on_fileLoadPropsAction_triggered()
{
  fileLoadProps();
}

void qavimator::on_fileSavePropsAction_triggered()
{
  fileSaveProps();
}

void qavimator::on_fileExitAction_triggered()
{
  fileExit();
}

void qavimator::on_editCutAction_triggered()
{
  editCut();
}

void qavimator::on_editCopyAction_triggered()
{
  editCopy();
}

void qavimator::on_editPasteAction_triggered()
{
  editPaste();
}

void qavimator::on_toolsOptimizeBVHAction_triggered()
{
  toolsOptimizeBVH();
}

void qavimator::on_toolsMirrorAction_triggered()
{
  Animation* anim=animationView->getAnimation();
  anim->mirror(currentPart);
  updateInputs();
}

void qavimator::on_optionsSkeletonAction_toggled(bool on)
{
  showSkeleton(on);
}

void qavimator::on_optionsJointLimitsAction_toggled(bool on)
{
  setJointLimits(on);
}

void qavimator::on_optionsLoopAction_toggled(bool on)
{
  setLoop(on);
}

void qavimator::on_optionsProtectFirstFrameAction_toggled(bool on)
{
  setProtectFirstFrame(on);
}

void qavimator::on_optionsShowTimelineAction_toggled(bool on)
{
  showTimeline(on);
}

void qavimator::on_optionsConfigureQAvimatorAction_triggered()
{
  configure();
}

void qavimator::on_helpAboutAction_triggered()
{
  helpAbout();
}

// ------- Additional Toolbar Element Slots --------

void qavimator::on_resetCameraAction_triggered()
{
  emit resetCamera();
}

// ------- UI Element Slots --------

void qavimator::on_selectAnimationCombo_activated(int which)
{
  animationChanged(which);
}

void qavimator::on_figureCombo_activated(int newShape)
{
    setAvatarShape((Animation::FigureType)newShape);
}

void qavimator::on_scaleSpin_valueChanged(int newValue)
{
  setAvatarScale(newValue);
}

void qavimator::on_editPartCombo_activated(int)
{
  partChoice();
}

void qavimator::on_xRotationEdit_returnPressed()
{
  rotationValue();
}

void qavimator::on_xRotationEdit_lostFocus()
{
  rotationValue();
}

void qavimator::on_xRotationSlider_valueChanged(int)
{
  rotationSlider(sender());
}

void qavimator::on_yRotationEdit_returnPressed()
{
  rotationValue();
}

void qavimator::on_yRotationEdit_lostFocus()
{
  rotationValue();
}

void qavimator::on_yRotationSlider_valueChanged(int)
{
  rotationSlider(sender());
}

void qavimator::on_zRotationEdit_returnPressed()
{
  rotationValue();
}

void qavimator::on_zRotationEdit_lostFocus()
{
  rotationValue();
}

void qavimator::on_zRotationSlider_valueChanged(int)
{
  rotationSlider(sender());
}

void qavimator::on_xPositionEdit_returnPressed()
{
  positionValue();
}

void qavimator::on_xPositionEdit_lostFocus()
{
  positionValue();
}

void qavimator::on_xPositionSlider_valueChanged(int)
{
  positionSlider(sender());
}

void qavimator::on_yPositionEdit_returnPressed()
{
  positionValue();
}

void qavimator::on_yPositionEdit_lostFocus()
{
  positionValue();
}

void qavimator::on_yPositionSlider_valueChanged(int)
{
  positionSlider(sender());
}

void qavimator::on_zPositionEdit_returnPressed()
{
  positionValue();
}

void qavimator::on_zPositionEdit_lostFocus()
{
  positionValue();
}

void qavimator::on_zPositionSlider_valueChanged(int)
{
  positionSlider(sender());
}

void qavimator::on_easeInCheck_stateChanged(int newState)
{
  easeInChanged(newState);
}

void qavimator::on_easeOutCheck_stateChanged(int newState)
{
  easeOutChanged(newState);
}

void qavimator::on_newBoxPropButton_clicked()
{
  newProp(Prop::Box);
}

void qavimator::on_newSpherePropButton_clicked()
{
  newProp(Prop::Sphere);
}

void qavimator::on_newConePropButton_clicked()
{
  newProp(Prop::Cone);
}

void qavimator::on_newTorusPropButton_clicked()
{
  newProp(Prop::Torus);
}

void qavimator::on_propNameCombo_activated(const QString& name)
{
  selectProp(name);
}

void qavimator::on_deletePropButton_clicked()
{
  deleteProp();
}

void qavimator::on_attachToComboBox_activated(int attachmentPoint)
{
  attachProp(attachmentPoint);
}

void qavimator::on_propXPosSpin_valueChanged(int)
{
  propPositionChanged();
}

void qavimator::on_propYPosSpin_valueChanged(int)
{
  propPositionChanged();
}

void qavimator::on_propZPosSpin_valueChanged(int)
{
  propPositionChanged();
}

void qavimator::on_propXScaleSpin_valueChanged(int)
{
  propScaleChanged();
}

void qavimator::on_propYScaleSpin_valueChanged(int)
{
  propScaleChanged();
}

void qavimator::on_propZScaleSpin_valueChanged(int)
{
  propScaleChanged();
}

void qavimator::on_propXRotSpin_valueChanged(int)
{
  propRotationChanged();
}

void qavimator::on_propYRotSpin_valueChanged(int)
{
  propRotationChanged();
}

void qavimator::on_propZRotSpin_valueChanged(int)
{
  propRotationChanged();
}

void qavimator::on_currentFrameSlider_valueChanged(int newValue)
{
  frameSlider(newValue);
}

void qavimator::on_playButton_clicked()
{
  nextPlaystate();
}

void qavimator::on_keyframeButton_toggled(bool on)
{
  qDebug("on_keyframeButton_toggled(%d)",(int) on);
  animationView->getAnimation()->toggleKeyFrame(currentPart); // (animationView->getSelectedPart());
  animationView->repaint();
}

void qavimator::on_loopInSpinBox_valueChanged(int newValue)
{
  setLoopInPoint(newValue);
}

void qavimator::on_loopOutSpinBox_valueChanged(int newValue)
{
  setLoopOutPoint(newValue);
}

void qavimator::on_framesSpin_valueChanged(int newValue)
{
  numFramesChanged(newValue);
}

void qavimator::on_fpsSpin_valueChanged(int newValue)
{
  setFPS(newValue);
}

// end autoconnection from designer UI
// -------------------------------------------------------------------------
