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

/*
 * features added by Darkside Eldrich
 */

#ifndef QAVIMATOR_H
#define QAVIMATOR_H

/*
#define PLAY_IMAGE "data/play.png"
#define PAUSE_IMAGE "data/pause.png"
#define KEY_IMAGE "data/key.png"
#define NOKEY_IMAGE "data/nokey.png"      */

#include <QList>

#include "ui_mainapplicationform.h"
/*#include "rotation.h"
#include "playstate.h"    */

#include "NewFileDialog.h"

class Animation;
class Prop;
class Timeline;
class AbstractDocumentTab;
class QFileInfo;
class QCloseEvent;
class QWidget;



class qavimator : public QMainWindow, public Ui::MainWindow
{
  Q_OBJECT

public:
  qavimator();
  ~qavimator();

signals:
  void enableRotation(bool state);
  void enablePosition(bool state);
  void enableProps(bool state);
  void enableEaseInOut(bool state);
  void configurationChanged();

protected slots:
  void configChanged();
  void closeTab(int);

  // autoconnection from designer UI
  // ------- Menu Action Slots --------
  void on_fileNewAction_triggered();
  void on_fileOpenAction_triggered();
  void on_fileSaveAction_triggered();
  void on_fileSaveAsAction_triggered();
  void on_fileCloseAction_triggered();
  void on_fileQuitAction_triggered();
  void on_fileExportForSecondLifeAction_triggered();

/*rbsh  void on_editCutAction_triggered();
  void on_editCopyAction_triggered();
  void on_editPasteAction_triggered();      */

  void on_optionsConfigureQAvimatorAction_triggered();
  void on_helpAboutAction_triggered();

  // ------- UI Element Slots --------

  void on_mdiArea_subWindowActivated(QMdiSubWindow*);

protected:
  // prevent closing of main window if there are unsaved changes
  virtual void closeEvent(QCloseEvent* event);

  void addTabsCloseButtons();
  /** Save or discard content of tabs with unsaved changes */
  bool resolveUnsavedTabs();

  /** Open new tab and load given file (or create new) */
  void OpenNewTab(NewFileDialog::ProjectType fileType, const QString& filename, bool newFile);
  AbstractDocumentTab* activeTab();
  /** Return all open document tabs */
  QList<AbstractDocumentTab*> openTabs();
  /** Activate a tab with given AbstractDocumentTab content.
      Returns FALSE if no such tab exists, otherwise TRUE. */
  bool activateTab(AbstractDocumentTab* tab);
  void UpdateMenus();
  void UpdateToolbar();

  void fileOpen(const QString& name);
  void quit();

/*rbsh
  void editCut();
  void editCopy();
  void editPaste();
*/
  void configure();
  void helpAbout();

  QString selectFileToOpen(const QString& caption);

  bool checkFileOverwrite(const QFileInfo& fileInfo);
  void setCurrentFile(const QString& fileName);

  bool loop;
  bool jointLimits;
  // if set the first frame of an animation is protected
  bool protectFirstFrame;
  // will be true if a frame is protected
  bool protect;

private slots:
    void on_fileAddAction_triggered();
    void on_resetCameraAction_triggered();
};

#endif
