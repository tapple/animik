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

#include <QPainter>
#include <QPaintEvent>

#include "timeline.h"
#include "animation.h"
#include "keyframelist.h"

Timeline::Timeline(QWidget* parent,Qt::WindowFlags) : QFrame(parent)
{
  qDebug("Timeline(0x%0lx)",(unsigned long) this);

  // init offscreen pixmap as 0 so the first change of number of frames will not
  // try to copy old pixmap data
  offscreen=0;

  resize(50,50);

  fullRepaint=false;

  animation=0;
  setCurrentFrame(0);
  setWindowTitle(tr("Timeline"));

  leftMouseButton=false;
  shift=false;

  dragging=0;
  trackSelected=-1;

  setFocusPolicy(Qt::StrongFocus);
}

Timeline::~Timeline()
{
}

void Timeline::paintEvent(QPaintEvent* /* e */)
{
  if(!animation) return;
  if(isHidden()) return;

  qDebug("Timeline::paintEvent(%d)",(int) fullRepaint);

  QSize newSize=QSize(numOfFrames*KEY_WIDTH,(NUM_PARTS-1)*LINE_HEIGHT);

  if(newSize!=size())
  {
    qDebug("Timeline::paintEvent(): resizing timeline widget to %dx%d",newSize.width(),newSize.height());
    resize(newSize);
    emit resized(newSize);
  }
/*
  // clip keyframe drawing to "dirty" area of redraw
  firstVisibleKeyX=e->rect().x()/KEY_WIDTH;
  visibleKeysX=e->rect().width()/KEY_WIDTH;
*/
  // clip keyframe drawing to "dirty" area of redraw
  firstVisibleKeyX=0;
  visibleKeysX=numOfFrames;

  // full repaint only rarely needed
  if(fullRepaint)
  {
    // clip keyframe drawing to "dirty" area of redraw
    firstVisibleKeyX=0;
    visibleKeysX=numOfFrames;

    for(int part=0;part<NUM_PARTS;part++)
    {
      drawTrack(part);
    } // for
    fullRepaint=false;
  }

  // reset keyframe clipping to have new or updated transition lines painted properly
  firstVisibleKeyX=0;
  visibleKeysX=numOfFrames;

  int xpos=frameSelected*KEY_WIDTH;

  QPainter p(this);
  p.drawPixmap(0,0,*offscreen);

  // draw current frame marker
  p.fillRect(xpos+KEY_WIDTH/2-1,0,2,height(),QColor("#ff0000"));

  qDebug("Timeline::paintEvent(): done");
}

void Timeline::setNumberOfFrames(int frames)
{
  qDebug("Timeline::setNumberOfFrames(%d==%d): %0lx",numOfFrames,frames,(unsigned long) offscreen);

  numOfFrames=frames;

  QPixmap* newOffscreen=new QPixmap(numOfFrames*KEY_WIDTH,LINE_HEIGHT*(NUM_PARTS+1));

  // copy old offscreen pixmap to new pixmap, if there is one
  if(offscreen)
  {
    QPainter p(newOffscreen);
    p.drawPixmap(0,0,*offscreen);
    delete offscreen;
  }

  offscreen=newOffscreen;

  fullRepaint=true;
  repaint();
}

void Timeline::setCurrentFrame(int frame)
{
  frameSelected=frame;
  emit positionCenter(frameSelected*KEY_WIDTH);
}

void Timeline::setAnimation(Animation* anim)
{
  // don't do anything if this is the same animation as before
  if(anim==animation) return;

  if(animation)
  {
    disconnect(animation,SIGNAL(numberOfFrames(int)),this,SLOT(setNumberOfFrames(int)));
    disconnect(animation,SIGNAL(redrawTrack(int)),this,SLOT(redrawTrackImmediately(int)));
    disconnect(this,SIGNAL(deleteFrame(int,int)),animation,SLOT(deleteFrame(int,int)));
    disconnect(this,SIGNAL(insertFrame(int,int)),animation,SLOT(insertFrame(int,int)));
  }
  animation=anim;

  if(animation)
  {
    connect(animation,SIGNAL(numberOfFrames(int)),this,SLOT(setNumberOfFrames(int)));
    connect(animation,SIGNAL(redrawTrack(int)),this,SLOT(redrawTrackImmediately(int)));
    connect(this,SIGNAL(deleteFrame(int,int)),animation,SLOT(deleteFrame(int,int)));
    connect(this,SIGNAL(insertFrame(int,int)),animation,SLOT(insertFrame(int,int)));

    qDebug("Timeline::setAnimation(): signal setup done");
  }

  emit animationChanged(anim);

  fullRepaint=true;
  repaint();
}

void Timeline::redrawTrackImmediately(int track)
{
  drawTrack(track);
  repaint();
}

void Timeline::redrawTrack(int track)
{
  drawTrack(track);
}

void Timeline::drawKeyframe(int track,int frame)
{
//  qDebug(QString("drawKeyframe(%1,%2)").arg(track).arg(frame));

  if(track==-1) return;

  int ypos=track*LINE_HEIGHT;

  QPainter p(offscreen);
  // calculate x position
  int xpos=frame*KEY_WIDTH;

  QPalette::ColorRole keyColor=QPalette::Foreground;

#ifdef Q_OS_WIN32
  // on windows systems use text highlight color to contrast with track highlight
  if(trackSelected==track)
    keyColor=QPalette::HighlightedText;
#endif

  QColor color(palette().color(QPalette::Active,keyColor));
  // draw the key frame
  if(frame==0)
  {
    p.fillRect(xpos,ypos,KEY_WIDTH/2,KEY_HEIGHT,QBrush(color));
    p.fillRect(xpos,ypos+LINE_HEIGHT/2-2,KEY_WIDTH,4,QBrush(color));
  }
  else if(frame==(numOfFrames-1))
  {
    p.fillRect(xpos+KEY_WIDTH/2,ypos,KEY_WIDTH/2,KEY_HEIGHT,QBrush(color));
    p.fillRect(xpos,ypos+LINE_HEIGHT/2-2,KEY_WIDTH,4,QBrush(color));
  }
  else
  {
    // find out if we should paint in highlight or normal color
    if(frame && dragging==frame && trackSelected==track) color=Qt::red;

    // get the list of key frames
    const BVHNode* joint=animation->getNode(track);

    bool ps=animation->compareFrames(joint,joint->getKeyframeBefore(frame).frameNumber(),frame);
    bool ns=animation->compareFrames(joint,joint->getNextKeyframe(frame).frameNumber(),frame);

    p.setPen(color);
    p.setBrush(QBrush(color));

    if(ps && ns)
      p.drawEllipse(xpos,ypos,KEY_WIDTH-1,LINE_HEIGHT-1);

    else if(ns)
    {
      QPolygon triangle(3);
      triangle.putPoints(0,3, xpos,ypos+(LINE_HEIGHT-1)/2, xpos+KEY_WIDTH-1,ypos, xpos+KEY_WIDTH-1,ypos+LINE_HEIGHT-1);

      p.drawPolygon(triangle);
    }
    else if(ps)
    {
      QPolygon triangle(3);
      triangle.putPoints(0,3, xpos+KEY_WIDTH-1,ypos+(LINE_HEIGHT-1)/2, xpos,ypos, xpos,ypos+LINE_HEIGHT-1);

      p.drawPolygon(triangle);
    }
    else
      p.fillRect(xpos,ypos,KEY_WIDTH-1,KEY_HEIGHT,QBrush(color));
  }
}

void Timeline::selectTrack(int track)
{
  int oldTrack=trackSelected;
  trackSelected=track;
  // if the user clicked on a different track
  if(oldTrack!=trackSelected)
  {
    // draw old track with old color
    drawTrack(oldTrack);
    // draw new track
    drawTrack(trackSelected);
  }
  repaint();
}

void Timeline::drawTrack(int track)
{
  if(track==-1) return;
  if(!offscreen) return;
//  qDebug("first %d vis %d",firstVisibleKeyX,visibleKeysX);
  qDebug("Timeline::drawTrack(%d, %d)",track,trackSelected);

  // normal tracks get default background color
  QPalette::ColorRole baseColor=QPalette::Background;
  // normal tracks get default foreground colors for keyframe transition lines
  QPalette::ColorRole lineColor=QPalette::Foreground;
  // selected tracks get other color, unless it's a "Site" track (end of limbs group)
  QString trackName=animation->getPartName(track);
  if(track==trackSelected && trackName!="Site")
  {
    baseColor=QPalette::Highlight;
#ifdef Q_OS_WIN32
    // additionally, set inverse highlight color for transition lines on windows for contrast
    lineColor=QPalette::HighlightedText;
#endif
  }

  int light=0;
  int y=track*LINE_HEIGHT;

  // temporary painter, must be destroyed before entering main keyframe loop,
  // or we clash with drawKeyframe() painter
  QPainter* p=new QPainter(offscreen);

  // find widget coordinates of clipped area and decide if we paint light or dark background
  int firstGreyShade=(firstVisibleKeyX/10);
  light=firstGreyShade % 2;
  firstGreyShade*=KEY_WIDTH*10;

  // draw alternatingly light / dark background
  for(int x=firstGreyShade;x<(firstGreyShade+visibleKeysX+10)*KEY_WIDTH;x+=10*KEY_WIDTH)
  {
    // draw a filled rectangle in background color, alternatingly darkened by 5 %
    p->fillRect(x,y,x+10*KEY_WIDTH,LINE_HEIGHT,palette().color(QPalette::Active,baseColor).dark(100+5*light));
    light=1-light;
  }

  // if this is a "Site end" marker (end of group of limbs) do nothing more
  if(trackName=="Site")
  {
    delete p;
    return;
  }

  // draw straight line as track marker within "dirty" redraw region
  p->fillRect(firstVisibleKeyX*KEY_WIDTH,y+LINE_HEIGHT/2,(visibleKeysX+1)*KEY_WIDTH,1,palette().color(QPalette::Active,baseColor).dark(115));
  p->fillRect(firstVisibleKeyX*KEY_WIDTH,y+LINE_HEIGHT/2+1,(visibleKeysX+1)*KEY_WIDTH,1,palette().color(QPalette::Active,baseColor).light(115));

  delete p;

  // get the list of key frames
  const BVHNode* joint=animation->getNode(track);
  // start drawing rest of key frames
  if(joint->numKeyframes())
  {
    // reset previous frame marker
    int oldFrame=0;
    // iterate through list of key frames
    for(int key=0;key<joint->numKeyframes();key++)
    {
      // get frame number of key frame
      const FrameData& frameData=joint->keyframeDataByIndex(key);
      int frameNum=frameData.frameNumber();

      // if key frame is not out of animation length
      if(frameNum<numOfFrames)
      {
        // if this key frame is not at the first animation frame
        if(frameNum>0)
        {
          // create new painter here, will be out of scope before drawKeyframe() gets called
          QPainter p(offscreen);
          // check if it differs from the previous frame, if it does, draw a line there
          // but only if this line is inside the "dirty" redraw region
          if(
              (frameNum>firstVisibleKeyX && oldFrame<(firstVisibleKeyX+visibleKeysX)) &&
              !animation->compareFrames(joint,frameNum,oldFrame)
            )
          {
            const FrameData& oldFrameData=joint->keyframeDataByIndex(key-1);
            int frameDiff=(frameNum-oldFrame+1)*KEY_WIDTH/2;

            QColor pen1(palette().color(QPalette::Active,lineColor));
            QColor pen2(palette().color(QPalette::Active,baseColor).light(115));

            if(oldFrameData.easeOut())
            {
              QPoint oldPos(oldFrame*KEY_WIDTH+KEY_WIDTH-1,y+LINE_HEIGHT/2);
              for(int x=0;x<frameDiff-KEY_WIDTH+1;x++)
              {
                QPoint newPos(x+oldFrame*KEY_WIDTH+KEY_WIDTH-1,
                              (int)(y+sin(x*6.283/(frameDiff-KEY_WIDTH+1))*(LINE_HEIGHT/2-1)+LINE_HEIGHT/2)
                             );
                p.setPen(pen1);
                p.drawLine(oldPos,newPos);
                p.setPen(pen2);
                p.drawLine(oldPos+QPoint(0,1),newPos+QPoint(0,1));
                oldPos=newPos;
              }
            }
            else
            {
              // we use fillRect instead of drawLine because for some reason the windows
              // version did not draw properly
              p.fillRect(oldFrame*KEY_WIDTH+KEY_WIDTH-1,
                         y+KEY_HEIGHT/2,
                         frameDiff-KEY_WIDTH,
                         1,QBrush(pen1));
              p.fillRect(oldFrame*KEY_WIDTH+KEY_WIDTH-1,
                         y+KEY_HEIGHT/2+1,
                         frameDiff,
                         1,QBrush(pen2));
            }

            if(frameData.easeIn())
            {
              QPoint oldPos(oldFrame*KEY_WIDTH+frameDiff,y+LINE_HEIGHT/2);
              for(int x=0;x<frameDiff-KEY_WIDTH+1;x++)
              {
                QPoint newPos(x+oldFrame*KEY_WIDTH+frameDiff-1,
                              (int)(y+sin(x*6.283/(frameDiff-KEY_WIDTH+1))*(LINE_HEIGHT/2-1)+LINE_HEIGHT/2)
                             );
                p.setPen(pen1);
                p.drawLine(oldPos,newPos);
                p.setPen(pen2);
                p.drawLine(oldPos+QPoint(0,1),newPos+QPoint(0,1));
                oldPos=newPos;
              }
            }
            else
            {
              p.fillRect(oldFrame*KEY_WIDTH+frameDiff-1,
                         y+KEY_HEIGHT/2,
                         frameDiff,
                         1,QBrush(pen1));
              p.fillRect(oldFrame*KEY_WIDTH+frameDiff-1,
                         y+KEY_HEIGHT/2+1,
                         frameDiff,
                         1,QBrush(pen2));
            }
          }
        }
        // only draw keyframes if they are in "dirty" clipping region
        if(isDirty(frameNum))
          // draw the key frame
          drawKeyframe(track,frameNum);
      }
      // remember this frame number as previous key frame
      oldFrame=frameNum;
    } // for

    // last frame is always a key frame, so draw it if it's inside the clipping region
    if(isDirty(numOfFrames-1)) drawKeyframe(track,numOfFrames-1);
  }
}

// returns true if frame is inside "dirty" redraw region
bool Timeline::isDirty(int frame)
{
  return (frame>=firstVisibleKeyX && frame<=(firstVisibleKeyX+visibleKeysX));
}

void Timeline::mousePressEvent(QMouseEvent* e)
{
  // get track and frame based on mouse coordinates
  selectTrack(e->y()/LINE_HEIGHT);
  frameSelected=e->x()/KEY_WIDTH;

  // set animation frame to where we clicked
  animation->setFrame(frameSelected);
  // check if we clicked on a key frame which is not the first or last one in animation
  if(frameSelected>0 && frameSelected<(numOfFrames-1) && animation->isKeyFrame(trackSelected,frameSelected))
  {
    // first switch on dragging state, so drawKeyframe() will work as expected
    dragging=frameSelected;
    // highlight keyframe
    drawKeyframe(trackSelected,frameSelected);
  }
  emit trackClicked(trackSelected);
  repaint();
  // remember mouse button state
  leftMouseButton=true;
}

void Timeline::mouseReleaseEvent(QMouseEvent*)
{
  leftMouseButton=false;
  if(dragging)
  {
    // first switch off dragging state, so drawKeyframe() will work as expected
    dragging=0;
    // remove highlight from keyframe
    drawKeyframe(trackSelected,frameSelected);
    repaint();
  }
}

void Timeline::mouseMoveEvent(QMouseEvent* e)
{
  // calculate new position (in frames)
  int frame=e->x()/KEY_WIDTH;

  // if frame position has not changed, do nothing
  if(frame==frameSelected) return;

  // special treatment if user is dragging a keyframe
  if(dragging)
  {
    // if user is dragging a keyframe, move it within more restrictive bounds
    if(frame>0 && frame<(numOfFrames-1))
    {
      // update dragging position
      dragging=frame;

      // make sure we don't overwrite already existing frames
      if(!animation->isKeyFrame(trackSelected,frame))
      {
        // check if the user holds the shift key
        if(shift)
        {
          // reset shift key status so we don't constantly create copies
          shift=false;
          // copy selected frame to new place
          animation->copyKeyFrame(trackSelected,frameSelected,frame);
        }
        else
          // move selected frame to new place
          animation->moveKeyFrame(trackSelected,frameSelected,frame);

        redrawTrack(trackSelected);
        // remember new position
        frameSelected=frame;
      }
    }
  }
  // no dragging so check if new position would be out of bounds
  else if(frame>=0 && frame<numOfFrames)
  {
    // remember new position
    frameSelected=frame;
    // set the new position
    animation->setFrame(frame);
  }
}

void Timeline::wheelEvent(QWheelEvent* e)
{
  // get curernt frame position
  int newFrame=frameSelected;

  // check if we should move forward or backwards
  if(e->delta()>0) newFrame-=5;
  else newFrame+=5;

  // check if new position would be out of bounds
  if(newFrame>=numOfFrames) newFrame=numOfFrames-1;
  else if(newFrame<0) newFrame=0;

  // remember new position
  frameSelected=newFrame;
  // set the new position
  animation->setFrame(newFrame);
}

void Timeline::keyPressEvent(QKeyEvent* e)
{
  switch(e->key())
  {
    case Qt::Key_Shift:
      shift=true;
      break;

    case Qt::Key_Delete:
      emit deleteFrame(trackSelected,frameSelected);

      if(trackSelected!=-1)
        drawTrack(trackSelected);
      else
        fullRepaint=true;

      repaint();
      break;

    case Qt::Key_Insert:
      emit insertFrame(trackSelected,frameSelected);

      if(trackSelected!=-1)
        drawTrack(trackSelected);
      else
        fullRepaint=true;

      repaint();
      break;

    default:
      e->ignore();
      return;
  }
  e->accept();
}

void Timeline::keyReleaseEvent(QKeyEvent* e)
{
  switch(e->key())
  {
    case Qt::Key_Shift:
      shift=false;
      break;
    default:
      e->ignore();
      return;
  }
  e->accept();
}
