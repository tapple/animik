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

#ifndef TIMELINE_H
#define TIMELINE_H

#include <QFrame>

#include "keyframelist.h"

#define KEY_WIDTH   10
#define KEY_HEIGHT  10
#define LINE_HEIGHT 11
#define LEFT_STRUT  55

// FIXME: find out at runtime
#define NUM_PARTS 25

/**
  @author Zi Ree
*/

class Animation;

class Timeline : public QFrame
{
  Q_OBJECT

  public:
    Timeline(QWidget* parent=0,Qt::WindowFlags f=0);
    ~Timeline();

    void setAnimation(Animation* anim);
    void selectTrack(int track);

  signals:
    void positionCenter(int pos);
    void trackClicked(int track);
    void resized(const QSize& newSize);
    void animationChanged(Animation* anim);
    void deleteFrame(int track,int frame);
    void insertFrame(int track,int frame);

  public slots:
    void setCurrentFrame(int frame);

  protected slots:
    void redrawTrack(int track);
    void redrawTrackImmediately(int track); // does an immediate repaint()
    void setNumberOfFrames(int frames);

  protected:
    virtual void paintEvent(QPaintEvent* event);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);
    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void wheelEvent(QWheelEvent* e);
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void keyReleaseEvent(QKeyEvent* event);

    void drawKeyframe(int track,int frame);
    void drawTrack(int track);
    bool isDirty(int frame);      // returns true if frame is inside "dirty" redraw region

    Animation* animation;

    int numOfFrames;
    int currentFrame;

    // keyframe drawing clip area
    int firstVisibleKeyX;
    int visibleKeysX;

    bool drawn;

    bool leftMouseButton;
    bool shift;

    bool fullRepaint;

    QPixmap* offscreen;

    int trackSelected;
    int frameSelected;
    int dragging;         // contains the current animation frame where dragging occurs
};

#endif
