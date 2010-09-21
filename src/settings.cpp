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

#include <iostream>
#include <QSettings>

#include "settings.h"


Settings::Settings()
{
  // should never be accessed
}

Settings::~Settings()
{
  // should never be accessed
}


// FIXME:: implement a static Settings:: class                              //TODO: Yes, move it there
void Settings::readSettings()
{
  QSettings settings;
  settings.beginGroup("/qavimator");

  // if no settings found, start up with defaults
  int width=850;
  int height=600;
/*rbsh  int figureType=0;
  bool skeleton=false;
  bool showTimelinePanel=true;

  jointLimits=true;
  loop=true;
  protectFirstFrame=true;     */
  m_lastPath = QString::null;

  // OpenGL presets
/*  Settings::setFog(true);
  Settings::setFloorTranslucency(33);   */

  m_fog = true;
  m_floorTranslucency = 33;

  // defaults for ease in/ease out
/*  Settings::setEaseIn(false);
  Settings::setEaseOut(false);      */
  m_easeIn = m_easeOut = false;

  bool settingsFound=settings.value("/settings").toBool();
  if(settingsFound)
  {
/*TODO: these will become animation specific
    loop=settings.value("/loop").toBool();
    skeleton=settings.value("/skeleton").toBool();
    jointLimits=settings.value("/joint_limits").toBool();
    protectFirstFrame=settings.value("/protect_first_frame").toBool();
    showTimelinePanel=settings.value("/show_timeline").toBool();        */

    width = settings.value("/mainwindow_width").toInt();
    height = settings.value("/mainwindow_height").toInt();

    m_lastPath=settings.value("/last_path").toString();

    // OpenGL settings
/*rbsh    Settings::setFog(settings.value("/fog").toBool());
    Settings::setFloorTranslucency(settings.value("/floor_translucency").toInt());

    // settings for ease in/ease outFrame
    Settings::setEaseIn(settings.value("/ease_in").toBool());
    Settings::setEaseOut(settings.value("/ease_out").toBool());     */

    m_fog = settings.value("/fog").toBool();
    m_floorTranslucency = settings.value("/floor_translucency").toInt();

    m_easeIn = settings.value("/ease_in").toBool();
    m_easeOut = settings.value("/ease_out").toBool();

    // sanity
    if(width<50) width=50;
    if(height<50) height=50;

    m_windowWidth = width;
    m_windowHeight = height;

//    figureType=settings.value("/figure").toInt();

    //TODO: if(figureType > Animation::NUM_FIGURES) throw new ConfigurationException();

    settings.endGroup();
  }


/*rbsh  optionsLoopAction->setChecked(loop);
  optionsSkeletonAction->setChecked(skeleton);
  optionsJointLimitsAction->setChecked(jointLimits);
  optionsShowTimelineAction->setChecked(showTimelinePanel);

  if(!showTimelinePanel) timelineView->hide();
  // prevent a signal to be sent to yet uninitialized animation view
  optionsProtectFirstFrameAction->blockSignals(true);
  optionsProtectFirstFrameAction->setChecked(protectFirstFrame);
  optionsProtectFirstFrameAction->blockSignals(false);

  figureCombo->setCurrentIndex(figureType);
  setAvatarShape( figureType );   */
}


//void Settings::setWindowHeight(int height)     { m_windowHeight=height; }
int Settings::windowHeight()                   { return m_windowHeight; }
//void Settings::setWindowWidth(int width)       { m_windowHeight=width; }
int Settings::windowWidth()                    { return m_windowWidth; }

QString Settings::lastPath()                   { return m_lastPath; }

//void Settings::setFog(bool on)                 { m_fog=on; }
bool Settings::fog()                           { return m_fog; }

//void Settings::setFloorTranslucency(int value) { m_floorTranslucency=value; }
int  Settings::floorTranslucency()             { return m_floorTranslucency; }

//void Settings::setEaseIn(bool on)              { m_easeIn=on; }
bool Settings::easeIn()                        { return m_easeIn; }
//void Settings::setEaseOut(bool on)             { m_easeOut=on; }
bool Settings::easeOut()                       { return m_easeOut; }
