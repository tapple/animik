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
#include <QStringList>
#include <QCoreApplication>
#include <QDir>

#include "settings.h"



Settings* Settings::instance = 0;

Settings::Settings()
{
  m_windowWidth = 850;
  m_windowHeight = 600;

  m_lastPath = QDir::currentPath();

  m_fog = true;
  m_floorTranslucency = 33;

  m_easeIn = m_easeOut = false;
}

Settings::~Settings()
{
  while(recentFiles.count() > MaxRecentOpenFiles)
    recentFiles.removeFirst();
}


Settings* Settings::Instance()
{
  if(!instance)
    instance = new Settings;
  return instance;
}


void Settings::ReadSettings()
{
  QCoreApplication::setOrganizationName("DeZiRee");
  QCoreApplication::setOrganizationDomain("qavimator.org");
  QCoreApplication::setApplicationName("QAvinmator");

  QSettings settings;
  settings.beginGroup("/qavimator");

  // if no settings found, start up with defaults
/*rbsh  int figureType=0;
  bool skeleton=false;
  bool showTimelinePanel=true;

  jointLimits=true;
  loop=true;
  protectFirstFrame=true;     */

  // OpenGL presets
/*  Settings::setFog(true);
  Settings::setFloorTranslucency(33);   */


  // defaults for ease in/ease out
/*  Settings::setEaseIn(false);
  Settings::setEaseOut(false);      */


  bool settingsFound=settings.value("/settings").toBool();
  if(settingsFound)
  {
/*TODO: these will become animation specific
    loop=settings.value("/loop").toBool();
    skeleton=settings.value("/skeleton").toBool();
    jointLimits=settings.value("/joint_limits").toBool();
    protectFirstFrame=settings.value("/protect_first_frame").toBool();
    showTimelinePanel=settings.value("/show_timeline").toBool();        */

    m_windowMaximized = settings.value("/mainwindow_maximized").toBool();
    int width = settings.value("/mainwindow_width").toInt();
    int height = settings.value("/mainwindow_height").toInt();

    m_lastPath=settings.value("/last_path").toString();
    recentFiles = settings.value("/recent_file").toStringList();

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

void Settings::WriteSettings()
{
  QSettings settings;
  settings.beginGroup("/qavimator");

  // make sure we know next time, that there actually was a settings file
  settings.setValue("/settings",true);

/*  settings.setValue("/loop",loop);
  settings.setValue("/skeleton",optionsSkeletonAction->isChecked());
  settings.setValue("/joint_limits",optionsJointLimitsAction->isChecked());
  settings.setValue("/protect_first_frame",optionsProtectFirstFrameAction->isChecked());
  settings.setValue("/show_timeline",optionsShowTimelineAction->isChecked());   */

//rbsh  settings.setValue("/figure",figureCombo->currentIndex());
  settings.setValue("/mainwindow_maximized", m_windowMaximized);
  settings.setValue("/mainwindow_width", m_windowWidth);
  settings.setValue("/mainwindow_height", m_windowHeight);

  settings.setValue("/recent_files", recentFiles);
  settings.setValue("/last_path", m_lastPath);

  // OpenGL settings
  settings.setValue("/fog", m_fog);
  settings.setValue("/floor_translucency", m_floorTranslucency);

  // settings for ease in/ease outFrame
  settings.setValue("/ease_in", m_easeIn);
  settings.setValue("/ease_out", m_easeOut);

  settings.endGroup();
}


//void Settings::setWindowHeight(int height)     { m_windowHeight=height; }
bool Settings::windowMaximized() const           { return m_windowMaximized; }
void Settings::setWindowMaximized(bool full)     { m_windowMaximized = full; }
int Settings::windowHeight() const                { return m_windowHeight; }
void Settings::setWindowHeight(int height)        { m_windowHeight = height; }
//void Settings::setWindowWidth(int width)       { m_windowHeight=width; }
int Settings::windowWidth() const                 { return m_windowWidth; }
void Settings::setWindowWidth(int width)          { m_windowWidth = width; }

const QStringList Settings::RecentFiles() const   { return recentFiles; }
QString Settings::lastPath() const                { return m_lastPath; }
void Settings::setLastPath(QString dir)           { m_lastPath = dir; }

void Settings::setFog(bool on)                    { m_fog=on; }
bool Settings::fog() const                        { return m_fog; }

void Settings::setFloorTranslucency(int value)    { m_floorTranslucency=value; }
int  Settings::floorTranslucency() const          { return m_floorTranslucency; }

//void Settings::setEaseIn(bool on)              { m_easeIn=on; }
bool Settings::easeIn() const                     { return m_easeIn; }
//void Settings::setEaseOut(bool on)             { m_easeOut=on; }
bool Settings::easeOut() const                    { return m_easeOut; }
