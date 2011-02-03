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

#ifndef SETTINGS_H
#define SETTINGS_H

/**
	@author Zi Ree <Zi Ree @ Second Life>
*/

#include <QString>
#include <QStringList>



class Settings
{
public:
  static Settings* Instance();

  static const int MaxRecentOpenFiles = 15;

  bool windowMaximized() const;
  void setWindowMaximized(bool fullscreen);
  int windowHeight() const;
  void setWindowHeight(int height);
  int windowWidth() const;
  void setWindowWidth(int widht);

  const QStringList RecentFiles() const;
  QString lastPath() const;
  void setLastPath(QString dir);

  void ReadSettings();
  void WriteSettings();

  void setFog(bool on);
  bool fog() const;

  void setFloorTranslucency(int value);
  int floorTranslucency() const;
  void setTPoseWarning(bool value);
  bool tPoseWarning() const;

//    static void setEaseIn(bool on);
  bool easeIn() const;
//    static void setEaseOut(bool on);
  bool easeOut() const;


private:
  Settings();
  ~Settings();

  static Settings* instance;

  bool m_windowMaximized;
  int m_windowHeight;
  int m_windowWidth;

  QStringList recentFiles;
  QString m_lastPath;

  bool m_fog;
  int  m_floorTranslucency;
  bool m_tPoseWarning;

  bool m_easeIn;       //TODO: animation
  bool m_easeOut;      //      specific?
};

#endif
