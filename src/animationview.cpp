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

#ifdef __APPLE__
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <QApplication.h>
#else


#include <GL/glu.h>
#include <GL/glut.h>
#endif


#include <QMouseEvent>

#include "animationview.h"
#include "bvh.h"
#include "slparts.h"

#include "settings.h"

#define SHIFT 1
#define CTRL  2
#define ALT   4

AnimationView::AnimationView(QWidget* parent,const char* /* name */,Animation* anim)
 : QGLWidget(parent)
{
  figureFiles << MALE_BVH << FEMALE_BVH;

  bvh=new BVH();
  if(!bvh)
  {
    qDebug("AnimationView::AnimationView(): BVH initialisation failed.");
    return;
  }

  animation=0;

  // fake glut initialization
  int args=1;
  // make sure the "qavimator" string is not const char*
  char arg0[]="qavimator";
  char* arg[]={arg0};
  glutInit(&args,arg);

  // init
  skeleton=false;
  selecting=false;
  partHighlighted=0;
  propDragging=0;
  partSelected=0;
  mirrorSelected=0;
  dragX=0;
  dragY=0;
  changeX=0;
  changeY=0;
  changeZ=0;
  xSelect=false;
  ySelect=false;
  zSelect=false;
  nextPropId=OBJECT_START;

#ifdef __APPLE__
  QString dataPath=QApplication::applicationDirPath() + "/../Resources";
#else
  QString dataPath=QAVIMATOR_DATAPATH;
#endif

  QString limFile=dataPath+"/"+LIMITS_FILE;
  qDebug("AnimationView::AnimationView(): using limits file '%s'",limFile.toLatin1().constData());

  // read SL reference models to restore joint positions, in case another model has joints
  // we do not support (e.g. the SL example bvh files)
  for(int i=0;i<Animation::NUM_FIGURES;i++)
  {
    QString model(dataPath+"/"+figureFiles[i]);
    qDebug("Reading reference model '%s'",model.toLatin1().constData());
    joints[i]=bvh->animRead(model,limFile);
  }

// FIXME:    mode(FL_DOUBLE | FL_MULTISAMPLE | FL_ALPHA | FL_DEPTH);

  leftMouseButton=false;
  frameProtected=false;
  modifier=0;
  if(anim) setAnimation(anim);
  setMouseTracking(true);
  setFocusPolicy(Qt::StrongFocus);

  connect(this,SIGNAL(storeCameraPosition(int)),&camera,SLOT(storeCameraPosition(int)));
  connect(this,SIGNAL(restoreCameraPosition(int)),&camera,SLOT(restoreCameraPosition(int)));
}

AnimationView::~AnimationView()
{
  clear();

  // call delete on all animations in the list
  qDeleteAll(animList);
  // clear all now invalid references from the list
  animList.clear();
}

BVH* AnimationView::getBVH() const
{
  return bvh;
}

void AnimationView::selectAnimation(unsigned int index)
{
  if(index< (unsigned int) animList.count())
  {
    animation=animList.at(index);
    emit animationSelected(getAnimation());
    repaint();
  }
}

void AnimationView::setAnimation(Animation* anim)
{
    clear();

    animation=anim;
    animList.append(anim);
    connect(anim,SIGNAL(frameChanged()),this,SLOT(repaint()));
    repaint();
}

void AnimationView::drawFloor()
{
  float alpha=(100-Settings::floorTranslucency())/100.0;

  glEnable(GL_DEPTH_TEST);
  glBegin(GL_QUADS);
  for(int i=-10;i<10;i++)
  {
    for(int j=-10;j<10;j++)
    {
      if((i+j) % 2)
      {
        if(frameProtected)
          glColor4f(0.3,0.0,0.0,alpha);
        else
          glColor4f(0.1,0.1,0.1,alpha);
      }
      else
      {
        if(frameProtected)
          glColor4f(0.8,0.0,0.0,alpha);
        else
          glColor4f(0.6,0.6,0.6,alpha);
      }

      glVertex3f(i*40,0,j*40);
      glVertex3f(i*40,0,(j+1)*40);
      glVertex3f((i+1)*40,0,(j+1)*40);
      glVertex3f((i+1)*40,0,j*40);
    }
  }

  glEnd();
}

void AnimationView::drawProp(const Prop* prop) const
{
  Prop::State state=Prop::Normal;
  if(propSelected==prop->id)
    state=Prop::Selected;
  else if(partHighlighted==prop->id) state=Prop::Highlighted;
  prop->draw(state);
  if(propSelected==prop->id)
    drawDragHandles(prop);
}

void AnimationView::drawProps()
{
  for(unsigned int index=0;index<(unsigned int) propList.count();index++)
  {
    Prop* prop=propList.at(index);
    if(!prop->isAttached()) drawProp(prop);
  }
}

// Adds a new animation without overriding others, and sets it current
void AnimationView::addAnimation(Animation* anim)
{
  if(!inAnimList(anim))
  {
    animList.append(anim);
    animation=anim; // set it as the current one
    if(animList.count() && anim!=animList.first())
      anim->setFrame(animList.first()->getFrame());

    connect(anim,SIGNAL(frameChanged()),this,SLOT(repaint()));
    repaint();
  }
}

void AnimationView::clear()
{
  animList.clear();
  animation=NULL;
}

void AnimationView::setFrame(int frame)
{
//  qDebug(QString("animationview->frame now %1").arg(frame));
  for(unsigned int i=0;i< (unsigned int) animList.count();i++)
  {
    animList.at(i)->setFrame(frame);
  } // for
}

void AnimationView::stepForward()
{
  for(unsigned int i=0;i< (unsigned int) animList.count();i++)
  {
    animList.at(i)->stepForward();
  } // for
}

void AnimationView::setFPS(int fps)
{
  for(unsigned int i=0;i< (unsigned int) animList.count();i++)
  {
    animList.at(i)->setFPS(fps);
  } // for
}

const Prop* AnimationView::addProp(Prop::PropType type,double x,double y,double z,double xs,double ys,double zs,double xr,double yr,double zr,int attach)
{
  QString name;
  QString baseName;

  if(type==Prop::Box) baseName="Box";
  else if(type==Prop::Sphere) baseName="Sphere";
  else if(type==Prop::Cone) baseName="Cone";
  else if(type==Prop::Torus) baseName="Torus";

  int objectNum=0;
  do
  {
    name=baseName+" "+QString::number(objectNum++);
  } while(getPropByName(name));

  Prop* newProp=new Prop(nextPropId,type,name);

  nextPropId++;

  newProp->attach(attach);

  newProp->setPosition(x,y,z);
  newProp->setRotation(xr,yr,zr);
  newProp->setScale(xs,ys,zs);

  propList.append(newProp);
  repaint();

  return newProp;
}

Prop* AnimationView::getPropByName(const QString& lookName)
{
  for(unsigned int index=0;index< (unsigned int) propList.count();index++)
  {
    Prop* prop=propList.at(index);
    if(prop->name()==lookName) return prop;
  }

  return 0;
}

Prop* AnimationView::getPropById(unsigned int id)
{
  for(unsigned int index=0;index< (unsigned int) propList.count();index++)
  {
    Prop* prop=propList.at(index);
    if(prop->id==id) return prop;
  }

  return 0;
}

void AnimationView::deleteProp(Prop* prop)
{
  propList.removeAll(prop);
  delete prop;
  repaint();
}

void AnimationView::clearProps()
{
  while(propList.count())
  {
    Prop* prop=propList.at(0);
    propList.removeAll(prop);
    delete prop;
  }
  repaint();
}

bool AnimationView::inAnimList(Animation* anim)
{
  return animList.contains(anim);
}

void AnimationView::setProjection()
{
  gluPerspective(60.0,((float)width())/height(),1,2000);
}

void AnimationView::setBodyMaterial()
{
  GLfloat ambientA[]={0.9, 0.667, 0.561, 1};
  GLfloat diffuseA[]={0.9, 0.667, 0.561, 0};
  GLfloat specularA[]={0.6, 0.6, 0.6, 0.0};
  GLfloat shininessA=100.0;

  glMaterialfv(GL_FRONT,GL_AMBIENT,ambientA);
  glMaterialfv(GL_FRONT,GL_DIFFUSE,diffuseA);
  glMaterialfv(GL_FRONT,GL_SPECULAR,specularA);
  glMaterialf(GL_FRONT,GL_SHININESS,shininessA);
}

void AnimationView::paintGL()
{
  draw();
}

void AnimationView::paintOverlayGL()
{
  draw();
}

void AnimationView::initializeGL()
{
  GLfloat position0[]={0.0,80.0,100.0,1.0};
  GLfloat ambient0[]={0.2,0.2,0.2,1 };
//  GLfloat diffuse0[] = { .5, .5, .5, 0.2 };
//  GLfloat specular0[] = { 0.5, 0.5, 0.2, 0.5 };

  GLfloat position1[]={0.0,80.0,-100.0,1.0};
  GLfloat ambient1[]={0.2,0.2,0.2,1};
  GLfloat diffuse1[]={0.5,0.5,0.5,1};
  GLfloat specular1[]={1,1,1,1};

  glViewport(0,0,width(),height());

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

  glLightfv(GL_LIGHT0,GL_AMBIENT,ambient0);
  //  glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0);
  //  glLightfv(GL_LIGHT0, GL_SPECULAR, specular0);

  glLightfv(GL_LIGHT1,GL_AMBIENT,ambient1);
  glLightfv(GL_LIGHT1,GL_DIFFUSE,diffuse1);
  glLightfv(GL_LIGHT1,GL_SPECULAR,specular1);

  glEnable(GL_NORMALIZE);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  setProjection();

  glLightfv(GL_LIGHT0,GL_POSITION,position0);
  glLightfv(GL_LIGHT1,GL_POSITION,position1);
}

void AnimationView::draw()
{
  if(!isValid()) initializeGL();

  if(Settings::fog())
  {
    glEnable(GL_FOG);
    {
      GLfloat fogColor[4]={0.5,0.5,0.5,0.3};
      int fogMode=GL_EXP; // GL_EXP2, GL_LINEAR
      glFogi(GL_FOG_MODE,fogMode);
      glFogfv(GL_FOG_COLOR,fogColor);
      glFogf(GL_FOG_DENSITY,0.005);
      glHint(GL_FOG_HINT,GL_DONT_CARE);
      glFogf(GL_FOG_START,200.0);
      glFogf(GL_FOG_END,2000.0);
    }
  }
  else
    glDisable(GL_FOG);

  glClearColor(0.5,0.5,0.5,0.3); /* fog color */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_LIGHTING);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_COLOR_MATERIAL);
  glShadeModel(GL_FLAT);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  camera.setModelView();
  if(!animList.isEmpty()) drawAnimations();
  drawFloor();
  drawProps();
  glFlush();
}

void AnimationView::drawAnimations()
{
  for(unsigned int index=0;index< (unsigned int) animList.count();index++)
  {
    drawFigure(animList.at(index),index);
  }
}

int AnimationView::pickPart(int x, int y)
{
  int bufSize=((Animation::MAX_PARTS+10)*animList.count()+propList.count())*4;

  GLuint* selectBuf=(GLuint*) malloc(bufSize);

//  GLuint selectBuf[bufSize];

  GLuint *p, num, name = 0;
  GLint hits;
  GLint viewport[4];
  GLuint depth=~0;

  glGetIntegerv(GL_VIEWPORT,viewport);
  glSelectBuffer(bufSize, selectBuf);
  (void) glRenderMode(GL_SELECT);
  glInitNames();
  glPushName(0);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluPickMatrix(x,(viewport[3]-y),5.0,5.0,viewport);
  setProjection();
  camera.setModelView();
  drawAnimations();
  drawProps();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glFlush();
  hits=glRenderMode(GL_RENDER);
  p=selectBuf;
  for(int i=0;i<hits;i++)
  {
    num = *(p++);
    if(*p < depth)
    {
      depth = *p;
      name = *(p+2);
    }
    p+=2;
    for (unsigned int j=0; j < num; j++) { *(p++); }
  }
  free(selectBuf);

//  qDebug("AnimationView::pickPart(): %d",name);

  return name;
}

void AnimationView::mouseMoveEvent(QMouseEvent* event)
{
  if(leftMouseButton)
  {
    QPoint dragPos=QCursor::pos();

    // since MacOSX doesn't like the old "drag and snap back" solution, we're going for a
    // more elaborate solution

    // calculate drag distance from last dragging point
    dragX=dragPos.x()-oldDragX;
    dragY=dragPos.y()-oldDragY;

    // remember the current position as last dragging point
    oldDragX=dragPos.x();
    oldDragY=dragPos.y();

    // if mouse has moved sufficiently far enough from first clicking position ...
    if(abs(clickPos.x()-dragPos.x())>100 || abs(clickPos.y()-dragPos.y())>100)
    {
      // set remembered drag position back to first click position
      oldDragX=clickPos.x();
      oldDragY=clickPos.y();
      // set mouse cursor back to first click position
      QCursor::setPos(clickPos);
    }

    if(partSelected)
    {
      if(partSelected<OBJECT_START)
      {
        changeX=0;
        changeY=0;
        changeZ=0;

        if(modifier & SHIFT) changeX=dragY;
        if(modifier & ALT)   changeY=dragX;
        else if(modifier & CTRL) changeZ=-dragX;

        emit partDragged(getSelectedPart(),changeX,changeY,changeZ);
        repaint();
      }
    }
    else if(propDragging)
    {
      float rot=camera.yRotation();
      Prop* prop=getPropById(propSelected);

      if(propDragging==DRAG_HANDLE_X)
      {
        if(rot>90 && rot<270) dragX=-dragX;
        emit propDragged(prop,(double) dragX/10.0,0,0);
      }
      else if(propDragging==DRAG_HANDLE_Y)
      {
        emit propDragged(prop,0,(double) -dragY/10.0,0);
      }
      else if(propDragging==DRAG_HANDLE_Z)
      {
        if(rot>90 && rot<270) dragY=-dragY;
        emit propDragged(prop,0,0,(double) dragY/10.0);
      }
      else if(propDragging==SCALE_HANDLE_X)
      {
        if(rot>90 && rot<270) dragX=-dragX;
        emit propScaled(prop,(double) dragX/10.0,0,0);
      }
      else if(propDragging==SCALE_HANDLE_Y)
      {
        if(rot>90 && rot<270) dragY=-dragY;
        emit propScaled(prop,0,(double) -dragY/10.0,0);
      }
      else if(propDragging==SCALE_HANDLE_Z)
      {
        emit propScaled(prop,0,0,(double) dragY/10.0);
      }
      else if(propDragging==ROTATE_HANDLE_X)
      {
        emit propRotated(prop,(double) dragX/5.0,0,0);
      }
      else if(propDragging==ROTATE_HANDLE_Y)
      {
        emit propRotated(prop,0,(double) -dragY/5.0,0);
      }
      else if(propDragging==ROTATE_HANDLE_Z)
      {
        emit propRotated(prop,0,0,(double) dragY/5.0);
      }
      repaint();
    }
    else
    {
      if(modifier & SHIFT)
        camera.pan(dragX/2,dragY/2,0);
      else if(modifier & ALT)
      {
        camera.pan(0,0,dragY);
        camera.rotate(0,dragX);
      }
      else
        camera.rotate(dragY,dragX);

      repaint();
    }
  }
  else
  {
    unsigned int oldPart=partHighlighted;
    partHighlighted=pickPart(event->x(),event->y());
    if(oldPart!=partHighlighted) repaint();
  }
}

void AnimationView::mousePressEvent(QMouseEvent* event)
{
  if(event->button()==Qt::LeftButton)
  {
    leftMouseButton=true;
    // hide mouse cursor to avoid confusion
    setCursor(QCursor(Qt::BlankCursor));

    // MacOSX didn't like our old "move and snap back" solution, so here goes another

    // remember mouse position to return to
    returnPos=QCursor::pos();
    // place mouse in the center of our view to avoid slamming against the screen border
    //// Causes problems on leopard
    ////QCursor::setPos(mapToGlobal(QPoint(width()/2,height()/2)));
    // remember new mouse position for dragging
    clickPos=QCursor::pos();
    // put in position for distance calculation
    oldDragX=clickPos.x();
    oldDragY=clickPos.y();
    // check out which part or prop has been clicked
    unsigned int selected=pickPart(event->x(),event->y());

    // if another part than the current one has been clicked, switch off mirror mode
    if(selected!=partSelected) getAnimation()->setMirrored(false);

    // background clicked, reset all
    if(!selected)
    {
      partSelected=0;
      mirrorSelected=0;
      propSelected=0;
      propDragging=0;
      emit backgroundClicked();
    }
    // body part clicked
    else if(selected<OBJECT_START)
    {
      partSelected=selected;
      selectAnimation(selected/ANIMATION_INCREMENT);
      propSelected=0;
      propDragging=0;

      BVHNode* part=getSelectedPart();
      changeX = changeY = changeZ = 0;
      dragX = dragY = 0;

      emit partClicked(part,
                       Rotation(getAnimation()->getRotation(part)),
                       getAnimation()->getRotationLimits(part),
                       Position(getAnimation()->getPosition())
                      );
      emit partClicked(partSelected % ANIMATION_INCREMENT);
    }
    // drag handle clicked
    else if(selected==DRAG_HANDLE_X ||
            selected==DRAG_HANDLE_Y ||
            selected==DRAG_HANDLE_Z ||
            selected==SCALE_HANDLE_X ||
            selected==SCALE_HANDLE_Y ||
            selected==SCALE_HANDLE_Z ||
            selected==ROTATE_HANDLE_X ||
            selected==ROTATE_HANDLE_Y ||
            selected==ROTATE_HANDLE_Z)
    {
      propDragging=selected;
      changeX = changeY = changeZ = 0;
      dragX = dragY = 0;
    }
    else
    {
      emit propClicked(getPropById(selected));
      propSelected=selected;
    }
    repaint();
  }
}

void AnimationView::mouseReleaseEvent(QMouseEvent* event)
{
  if(event->button()==Qt::LeftButton)
  {
    // move mouse cursor back to the beginning of the dragging process
    //// causes problems on leopard
    ////QCursor::setPos(returnPos);
    // show mouse cursor again
    setCursor(Qt::ArrowCursor);
    leftMouseButton=false;
    propDragging=0;
  }
}

void AnimationView::mouseDoubleClickEvent(QMouseEvent* event)
{
  int selected=pickPart(event->x(),event->y());

  // no double clicks for props or drag handles
  if(selected>=OBJECT_START) return;

  if(modifier & SHIFT)
  {
    mirrorSelected=getSelectedPart()->getMirrorIndex()+(selected/ANIMATION_INCREMENT)*ANIMATION_INCREMENT;
    if(mirrorSelected)
      getAnimation()->setMirrored(true);
  }
  else if(selected && selected < OBJECT_START)
    getAnimation()->setIK(getAnimation()->getNode(selected),
                     !getAnimation()->getIK(getAnimation()->getNode(selected)));
  repaint();
}

void AnimationView::wheelEvent(QWheelEvent* event)
{
  camera.pan(0,0,-event->delta()/12);
  repaint();
}

void AnimationView::keyPressEvent(QKeyEvent* event)
{
  int num=-1;

  switch(event->key())
  {
    case Qt::Key_PageUp:
      camera.pan(0,0,-5);
      repaint();
      break;
    case Qt::Key_PageDown:
        camera.pan(0,0,5);
        repaint();
        break;
    case Qt::Key_Shift:
      modifier|=SHIFT;
      xSelect = true;
      repaint();
      break;
    case Qt::Key_Alt:
      modifier|=ALT;
      ySelect = true;
      repaint();
      break;
    case Qt::Key_Control:
      modifier|=CTRL;
      zSelect = true;
      repaint();
      break;
    case Qt::Key_F9:
      num=0;
      break;
    case Qt::Key_F10:
      num=1;
      break;
    case Qt::Key_F11:
      num=2;
      break;
    case Qt::Key_F12:
      num=3;
      break;
    case Qt::Key_Escape:
      resetCamera();
      break;
    default:
      event->ignore();
      return;
  }

  if(num>=0)
  {
    Qt::KeyboardModifiers modifier=event->modifiers();
    if(modifier==Qt::ShiftModifier)
      emit storeCameraPosition(num);
    else if(modifier==Qt::NoModifier)
    {
      emit restoreCameraPosition(num);
      repaint();
    }
  }
  event->ignore();
}

void AnimationView::keyReleaseEvent(QKeyEvent* event)
{
  switch(event->key())
  {
    case Qt::Key_Shift:
      modifier&=!SHIFT;
      xSelect = false;
      repaint();
      break;
    case Qt::Key_Alt:
      modifier&=!ALT;
      ySelect = false;
      repaint();
      break;
    case Qt::Key_Control:
      modifier&=!CTRL;
      zSelect = false;
      repaint();
      break;
  }
  event->ignore();
}

void AnimationView::drawFigure(Animation* anim,unsigned int index)
{
    Animation::FigureType figType=anim->getFigureType();

    glShadeModel(GL_SMOOTH);
    setBodyMaterial();
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // save current drawing matrix
    glPushMatrix();

    // scale drawing matrix to avatar scale specified
    float scale=anim->getAvatarScale();
    glScalef(scale,scale,scale);

    Position pos=anim->getPosition();
    glTranslatef(pos.x,pos.y,pos.z);

    // visual compensation
    glTranslatef(0, 2, 0);

    selectName = index*ANIMATION_INCREMENT;
    glEnable(GL_DEPTH_TEST);
    drawPart(anim,index,anim->getFrame(),anim->getMotion(),joints[figType],MODE_PARTS);
    selectName = index*ANIMATION_INCREMENT;
    glEnable(GL_COLOR_MATERIAL);
    drawPart(anim,index,anim->getFrame(),anim->getMotion(),joints[figType],MODE_ROT_AXES);
    selectName = index*ANIMATION_INCREMENT;
    glDisable(GL_DEPTH_TEST);
    drawPart(anim,index,anim->getFrame(),anim->getMotion(),joints[figType],MODE_SKELETON);

    // restore drawing matrix
    glPopMatrix();
}

// NOTE: joints == motion for now
void AnimationView::drawPart(Animation* anim,unsigned int currentAnimationIndex,int frame,BVHNode* motion,BVHNode* joints,int mode)
{
  float color[4];

  GLint renderMode;
  bool selecting;

  glGetIntegerv(GL_RENDER_MODE, &renderMode);
  selecting=(renderMode==GL_SELECT);
  if(motion && joints)
  {
    selectName++;
    glPushMatrix();
    glTranslatef(joints->offset[0],joints->offset[1],joints->offset[2]);
    if(motion->type==BVH_NO_SL)
    {
      selectName++;
      motion=motion->child(0);
    }
    if(mode==MODE_SKELETON && skeleton && !selecting)
    {
      glColor4f(0,1,1,1);
      glLineWidth(1);
      glBegin(GL_LINES);
      glVertex3f(-joints->offset[0],-joints->offset[1],-joints->offset[2]);
      glVertex3f(0,0,0);
      glEnd();

      if(joints->type!=BVH_ROOT)
      {
        // draw joint spheres in skeleton mode, red for selected parts,
        // blue for hightlighted and green for all others
        if(partSelected==selectName)
          glColor4f(1,0,0,1);
        else if(partHighlighted==selectName)
          glColor4f(0,0,1,1);
        else
          glColor4f(0,1,0,1);

        glutSolidSphere(1,16,16);
      }
    }

    Rotation rot=motion->frameData(frame).rotation();
    for(int i=0;i<motion->numChannels;i++)
    {
/*
      float value;
      if(motion->ikOn)
        value = motion->frame[frame][i] + motion->ikRot[i];
      else
        value = motion->frame[frame][i];

      switch(motion->channelType[i]) {
        case BVH_XROT: glRotatef(value, 1, 0, 0); break;
        case BVH_YROT: glRotatef(value, 0, 1, 0); break;
        case BVH_ZROT: glRotatef(value, 0, 0, 1); break;
        default: break;
      } */

      Rotation ikRot;
      if(motion->ikOn) ikRot=motion->ikRot;

      // need to do rotations in the right order
      switch(motion->channelType[i])
      {
        case BVH_XROT: glRotatef(rot.x+ikRot.x,1,0,0); break;
        case BVH_YROT: glRotatef(rot.y+ikRot.y,0,1,0); break;
        case BVH_ZROT: glRotatef(rot.z+ikRot.z,0,0,1); break;
        default: break;
      }

      if(mode==MODE_ROT_AXES && !selecting && partSelected==selectName)
      {
        switch(motion->channelType[i])
        {
          case BVH_XROT: drawCircle(0,10,xSelect ? 4 : 1); break;
          case BVH_YROT: drawCircle(1,10,ySelect ? 4 : 1); break;
          case BVH_ZROT: drawCircle(2,10,zSelect ? 4 : 1); break;
          default: break;
        }
      }
    }
    if(mode==MODE_PARTS)
    {
      if(selecting) glLoadName(selectName);
      if(anim->getMirrored() &&
          (mirrorSelected==selectName || partSelected==selectName)
	)
      {
        glColor4f(1.0, 0.635, 0.059, 1);
      }
      else if(partSelected==selectName)
        glColor4f(0.6, 0.3, 0.3, 1);
      else if(partHighlighted==selectName)
        glColor4f(0.4, 0.5, 0.3, 1);
      else
          glColor4f(0.6, 0.5, 0.5, 1);           //edu: when drawing aux figures, use (0.55, 0.5, 0.5, 0.9)
      if(anim->getIK(motion))
      {
        glGetFloatv(GL_CURRENT_COLOR,color);
        glColor4f(color[0],color[1],color[2]+0.3,color[3]);
      }
      anim->getFigureType()==Animation::FIGURE_MALE ? drawSLMalePart(motion->name()):drawSLFemalePart(motion->name());

      for(unsigned int index=0;index< (unsigned int) propList.count();index++)
      {
        Prop* prop=propList.at(index);
        if(prop->isAttached()==selectName) drawProp(prop);
      } // for
    }
    for(int i=0;i<motion->numChildren();i++)
    {
      drawPart(anim,currentAnimationIndex,frame,motion->child(i),joints->child(i),mode);
    }
    glPopMatrix();
  }
}

void AnimationView::drawDragHandles(const Prop* prop) const
{
  // get prop's position
  double x=prop->x;
  double y=prop->y;
  double z=prop->z;
 // get prop's scale
  double xs=prop->xs/2.0;
  double ys=prop->ys/2.0;
  double zs=prop->zs/2.0;

  QColor xRGB("#ff0000");
  QColor yRGB("#00ff00");
  QColor zRGB("#0000ff");

  int xLineWidth=1;
  int yLineWidth=1;
  int zLineWidth=1;

  switch(partHighlighted)
  {
    case SCALE_HANDLE_X:
    case ROTATE_HANDLE_X:
    case DRAG_HANDLE_X:
      xRGB=xRGB.lighter(120);
      xLineWidth=3;
      break;
    case SCALE_HANDLE_Y:
    case ROTATE_HANDLE_Y:
    case DRAG_HANDLE_Y:
      yRGB=yRGB.lighter(120);
      yLineWidth=3;
      break;
    case SCALE_HANDLE_Z:
    case ROTATE_HANDLE_Z:
    case DRAG_HANDLE_Z:
      zRGB=zRGB.lighter(120);
      zLineWidth=3;
      break;
    default:
      break;
  }

  // remember drawing matrix on stack
  glPushMatrix();
  // set matrix origin to our object's center
  glTranslatef(x,y,z);

  if(modifier & SHIFT)
  {
    // now draw the scale cubes with proper depth sorting
    glEnable(GL_DEPTH_TEST);

    glRotatef(prop->xr,1,0,0);
    glRotatef(prop->yr,0,1,0);
    glRotatef(prop->zr,0,0,1);

    glLoadName(SCALE_HANDLE_X);
    glColor4f(xRGB.redF(),xRGB.greenF(),xRGB.blueF(),1);
    glTranslatef(-xs,0,0);
    glutSolidCube(2);
    glTranslatef(xs*2,0,0);
    glutSolidCube(2);

    glLoadName(SCALE_HANDLE_Y);
    glColor4f(yRGB.redF(),yRGB.greenF(),yRGB.blueF(),1);
    glTranslatef(-xs,-ys,0);
    glutSolidCube(2);
    glTranslatef(0,ys*2,0);
    glutSolidCube(2);

    glLoadName(SCALE_HANDLE_Z);
    glColor4f(zRGB.redF(),zRGB.greenF(),zRGB.blueF(),1);
    glTranslatef(0,-ys,-zs);
    glutSolidCube(2);
    glTranslatef(0,0,zs*2);
    glutSolidCube(2);
  }
  else if(modifier & CTRL)
  {
    // now draw the rotate spheres with proper depth sorting
    glEnable(GL_DEPTH_TEST);

    glLoadName(ROTATE_HANDLE_X);
    glColor4f(xRGB.redF(),xRGB.greenF(),xRGB.blueF(),1);
    glTranslatef(-xs-5,0,0);
    glutSolidSphere(1,16,16);
    glTranslatef(2*(xs+5),0,0);
    glutSolidSphere(1,16,16);

    glLoadName(ROTATE_HANDLE_Y);
    glColor4f(yRGB.redF(),yRGB.greenF(),yRGB.blueF(),1);
    glTranslatef(-xs-5,-ys-5,0);
    glutSolidSphere(1,16,16);
    glTranslatef(0,2*(ys+5),0);
    glutSolidSphere(1,16,16);

    glLoadName(ROTATE_HANDLE_Z);
    glColor4f(zRGB.redF(),zRGB.greenF(),zRGB.blueF(),1);
    glTranslatef(0,-ys-5,-zs-5);
    glutSolidSphere(1,16,16);
    glTranslatef(0,0,2*(zs+5));
    glutSolidSphere(1,16,16);
  }
  else
  {
    // first draw the crosshair lines without depth testing, so they are always visible
    glDisable(GL_DEPTH_TEST);

    glLineWidth(xLineWidth);
    glBegin(GL_LINES);
    glColor4f(xRGB.redF(),xRGB.greenF(),xRGB.blueF(),1);
    glVertex3f(-xs-5,0,0);
    glVertex3f( xs+5,0,0);
    glEnd();

    glLineWidth(yLineWidth);
    glBegin(GL_LINES);
    glColor4f(yRGB.redF(),yRGB.greenF(),yRGB.blueF(),1);
    glVertex3f(0,-ys-5,0);
    glVertex3f(0, ys+5,0);
    glEnd();

    glLineWidth(zLineWidth);
    glBegin(GL_LINES);
    glColor4f(zRGB.redF(),zRGB.greenF(),zRGB.blueF(),1);
    glVertex3f(0,0,-zs-5);
    glVertex3f(0,0, zs+5);
    glEnd();

    // now draw the drag handle arrows with proper depth sorting
    glEnable(GL_DEPTH_TEST);

    glLoadName(DRAG_HANDLE_X);
    glColor4f(xRGB.redF(),xRGB.greenF(),xRGB.blueF(),1);
    glTranslatef(-xs-5,0,0);
    glRotatef(270,0,1,0);
    glutSolidCone(1,3,16,16);
    glRotatef(90,0,1,0);
    glTranslatef(2*(xs+5),0,0);
    glRotatef(90,0,1,0);
    glutSolidCone(1,3,16,16);
    glRotatef(270,0,1,0);

    glLoadName(DRAG_HANDLE_Y);
    glColor4f(yRGB.redF(),yRGB.greenF(),yRGB.blueF(),1);
    glTranslatef(-xs-5,-ys-5,0);
    glRotatef(90,1,0,0);
    glutSolidCone(1,3,16,16);
    glRotatef(270,1,0,0);
    glTranslatef(0,2*(ys+5),0);
    glRotatef(270,1,0,0);
    glutSolidCone(1,3,16,16);
    glRotatef(90,1,0,0);

    glLoadName(DRAG_HANDLE_Z);
    glColor4f(zRGB.redF(),zRGB.greenF(),zRGB.blueF(),1);
    glTranslatef(0,-ys-5,-zs-5);
    glRotatef(180,1,0,0);
    glutSolidCone(1,3,16,16);
    glRotatef(180,1,0,0);
    glTranslatef(0,0,2*(zs+5));
    glutSolidCone(1,3,16,16);
  }
  // restore drawing matrix
  glPopMatrix();
}

void AnimationView::drawCircle(int axis,float radius,int width)
{
  GLint circle_points=100;
  float angle;

  glLineWidth(width);
  switch(axis)
  {
    case 0: glColor4f(1,0,0,1); break;
    case 1: glColor4f(0,1,0,1); break;
    case 2: glColor4f(0,0,1,1); break;
  }
  glBegin(GL_LINE_LOOP);
  for(int i=0;i<circle_points;i++)
  {
    angle=2*M_PI*i/circle_points;
    switch(axis)
    {
      case 0: glVertex3f(0,radius*cos(angle),radius*sin(angle)); break;
      case 1: glVertex3f(radius*cos(angle),0,radius*sin(angle)); break;
      case 2: glVertex3f(radius*cos(angle),radius*sin(angle),0); break;
    }
  }
  glEnd();
  glBegin(GL_LINES);
  switch(axis)
  {
    case 0: glVertex3f(-10,0,0); glVertex3f(10,0,0); break;
    case 1: glVertex3f(0,-10,0); glVertex3f(0,10,0); break;
    case 2: glVertex3f(0,0,-10); glVertex3f(0,0,10); break;
  }
  glEnd();

}

BVHNode* AnimationView::getSelectedPart()
{
  return getAnimation()->getNode(partSelected % ANIMATION_INCREMENT);
}

unsigned int AnimationView::getSelectedPartIndex()
{
  return partSelected % ANIMATION_INCREMENT;
}
/*
const QString AnimationView::getPartName(int index)
{
  // get part name from animation, with respect to multiple animations in view
  return getAnimation()->getPartName(index % ANIMATION_INCREMENT);
}
*/
// returns the selected prop name or an empty string if none selected
const QString AnimationView::getSelectedPropName()
{
  for(unsigned int index=0;index< (unsigned int) propList.count();index++)
    if(propList.at(index)->id==propSelected) return propList.at(index)->name();
  return QString();
}

void AnimationView::selectPart(int partNum)
{
  BVHNode* node=getAnimation()->getNode(partNum);
  qDebug("AnimationView::selectPart(%d)",partNum);

  if(!node)
  {
    qDebug("AnimationView::selectPart(%d): node==0!",partNum);
    return;
  }

  if(node->type==BVH_END)
  {
    partSelected=0;
    mirrorSelected=0;
    propSelected=0;
    propDragging=0;
    emit backgroundClicked();
    repaint();
  }
  else selectPart(node);
}

void AnimationView::selectPart(BVHNode* node)
{
  if(!node)
  {
    qDebug("AnimationView::selectPart(node): node==0!");
    return;
  }

  qDebug("AnimationView::selectPart(node): %s",node->name().toLatin1().constData());
  // make sure no prop is selected anymore
  propSelected=0;
  propDragging=0;

  // find out the index count of the animation we're working with currently
  int animationIndex=animList.indexOf(getAnimation());

  // get the part index to be selected, including the proper animation increment
  // FIXME: when we are adding support for removing animations we need to remember
  //        the increment for each animation so they don't get confused
  partSelected=getAnimation()->getPartIndex(node)+ANIMATION_INCREMENT*animationIndex;
  emit partClicked(node,
                   Rotation(getAnimation()->getRotation(node)),
                   getAnimation()->getRotationLimits(node),
                   Position(getAnimation()->getPosition())
                  );
  repaint();
}

void AnimationView::selectProp(const QString& propName)
{
  // make sure no part is selected anymore
  partSelected=0;
  mirrorSelected=0;
  Prop* prop=getPropByName(propName);
  if(prop) propSelected=prop->id;
  repaint();
}

void AnimationView::resetCamera()
{
  camera.reset();
  repaint();
}

// handle widget resizes
void AnimationView::resizeEvent(QResizeEvent* newSize)
{
  int w=newSize->size().width();
  int h=newSize->size().height();

  // reset coordinates
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  // resize GL viewport
  glViewport(0,0,w,h);

  // set up aspect ratio
  setProjection();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

// set the current frame's visual protection status
void AnimationView::protectFrame(bool on)
{
  // only redraw if we need to
  if(frameProtected!=on)
  {
    frameProtected=on;
    repaint();
  }
}
