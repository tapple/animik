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

#include <QHBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QPainter>

#include "timelineview.h"
#include "timeline.h"
#include "animation.h"

TimelineView::TimelineView(QWidget* parent,Qt::WindowFlags f) : QFrame(parent,f)
{
  QHBoxLayout* layout=new QHBoxLayout(this);

  timelineTracks=new TimelineTracks(this);
  timeline=new Timeline(0);
  view=new QScrollArea(0);

  view->setBackgroundRole(QPalette::Dark);

  view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  view->setWidget(timeline);

  timeline->setBackgroundRole(QPalette::Base);

  layout->addWidget(timelineTracks,0);
  layout->addWidget(view);

  connect(timeline,SIGNAL(animationChanged(Animation*)),this,SLOT(setAnimation(Animation*)));
  connect(timeline,SIGNAL(trackClicked(int)),this,SLOT(selectTrack(int)));
}

TimelineView::~TimelineView()
{
}

// absolute x pixel position
void TimelineView::scrollTo(int x)
{
  QSize size=view->size();
  view->ensureVisible(x,0,size.width()/4,size.height());
}

void TimelineView::setAnimation(Animation* anim)
{
  timelineTracks->setAnimation(anim);
}

Timeline* TimelineView::getTimeline() const
{
  return timeline;
}

void TimelineView::selectTrack(int track)
{
  timeline->selectTrack(track);
  timelineTracks->selectTrack(track);
}

void TimelineView::backgroundClicked()
{
  timeline->selectTrack(-1);
  timelineTracks->selectTrack(-1);
}

// --------------

TimelineTracks::TimelineTracks(QWidget* parent,Qt::WindowFlags f) : QWidget(parent,f)
{
  resize(sizeHint());
}

TimelineTracks::~TimelineTracks()
{
  selectedTrack=-1;
}

QSize TimelineTracks::sizeHint() const
{
  return QSize(LEFT_STRUT,(NUM_PARTS-1)*LINE_HEIGHT+2);
}

void TimelineTracks::paintEvent(QPaintEvent*)
{
  resize(sizeHint());
  if(!animation) return;

  for(int part=0;part<NUM_PARTS;part++) drawTrack(part);
}

void TimelineTracks::drawTrack(int track)
{
  if(track==-1) return;

  QString trackName=(animation) ? animation->getPartName(track) : "";
  if(trackName!="Site")
  {
    QPainter p(this);
    QPalette::ColorRole textColor=QPalette::Foreground;

    int y=track*LINE_HEIGHT+2;

    if(track==selectedTrack)
    {
      p.fillRect(0,y,width(),LINE_HEIGHT,palette().color(QPalette::Active,QPalette::Highlight));
#ifdef Q_OS_WIN32
      // on windows systems use contrast color to track highlight color
      textColor=QPalette::HighlightedText;
#endif
    }
    else
      p.eraseRect(0,y,width(),LINE_HEIGHT);

    // draw track name
    p.setPen(palette().color(QPalette::Active,textColor));
    p.drawText(0,y+KEY_HEIGHT,trackName);
  }
}

void TimelineTracks::setAnimation(Animation* anim)
{
  animation=anim;
}

void TimelineTracks::selectTrack(int track)
{
  selectedTrack=track;
  repaint();
}
