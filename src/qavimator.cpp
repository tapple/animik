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
#include <QTabBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QMdiSubWindow>
#include <QRegExp>

#include "qavimator.h"
#include "settings.h"
#include "settingsdialog.h"
#include "SaveChangesDialog.h"
#include "NewFileDialog.h"
#include "KeyFramerTab.h"

#define ANIM_FILTER "Animation Files (*.avm *.avbl *.bvh)"
#define SVN_ID      "$Id$"



qavimator::qavimator() //: QMainWindow(0)
{
  setupUi(this);
  UpdateMenus();
  UpdateToolbar();

  setWindowTitle("Animik");             //sorry Zi
  setAttribute(Qt::WA_DeleteOnClose);

  resize(Settings::Instance()->windowWidth(), Settings::Instance()->windowHeight());

  if(qApp->argc()>1)
  {
    fileOpen(qApp->argv()[1]);
  }
}

qavimator::~qavimator()
{}



void qavimator::OpenNewTab(NewFileDialog::ProjectType fileType, const QString& filename, bool newFile)
{
  QWidget* tab;

  switch(fileType)
  {
    case NewFileDialog::AVM :
      tab = new KeyFramerTab(this, filename, newFile);
    break;
    case NewFileDialog::AVBL :
      throw "Not implemented yet";     //TODO
    break;
    default: throw "Unknown file type";
  }

  mdiArea->addSubWindow(tab);
  addTabsCloseButtons();

  connect(tab, SIGNAL(destroyed()), this, SLOT(UpdateMenus()));
  connect(tab, SIGNAL(destroyed()), this, SLOT(UpdateToolbar()));

  tab->showMaximized();
}

void qavimator::addTabsCloseButtons()
{
  QRegExp expr("^(?!excludeFromClosableTabs)", Qt::CaseSensitive, QRegExp::RegExp);
  foreach (QTabBar* tab, mdiArea->findChildren<QTabBar*>(expr))
  {
    if(!tab->tabsClosable())
    {
      tab->setTabsClosable(true);
      connect(tab, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    }
  }
}

void qavimator::UpdateMenus()
{
  if(activeTab() != 0)
    return;           //Let the tab handle the actions by itself
                      //Document related actions common for all tabs
                      //are handled in UpdateToolbar()
  fileExportForSecondLifeAction->setEnabled(false);
  fileLoadPropsAction->setEnabled(false);
  fileSavePropsAction->setEnabled(false);

  toolsOptimizeBVHAction->setEnabled(false);
  toolsMirrorAction->setEnabled(false);

  optionsJointLimitsAction->setEnabled(false);
  optionsLoopAction->setEnabled(false);
  optionsProtectFirstFrameAction->setEnabled(false);
  optionsShowTimelineAction->setEnabled(false);
  optionsSkeletonAction->setEnabled(false);
}


void qavimator::UpdateToolbar()
{
  bool hasTabs = (activeTab() != 0);

  fileAddAction->setEnabled(false);         //TODO: decide it's future

  fileSaveAction->setEnabled(hasTabs);
  fileSaveAsAction->setEnabled(hasTabs);
  fileCloseAction->setEnabled(hasTabs);

  editCutAction->setEnabled(hasTabs);
  editCopyAction->setEnabled(hasTabs);
  editPasteAction->setEnabled(hasTabs);

  if(!hasTabs)
    resetCameraAction->setVisible(false);
}


/**
  Return QWidget content of active tab.
  */
AbstractDocumentTab* qavimator::activeTab()
{
  if (QMdiSubWindow *activeSubWindow = mdiArea->activeSubWindow())
    return dynamic_cast<AbstractDocumentTab *>(activeSubWindow->widget());
  return 0;
}


QList<AbstractDocumentTab*> qavimator::openTabs()
{
  QList<AbstractDocumentTab*> tabs;
  foreach(QMdiSubWindow* subWindow, mdiArea->subWindowList())
    tabs.append(dynamic_cast<AbstractDocumentTab *>(subWindow->widget()));

  return tabs;
}


bool qavimator::activateTab(AbstractDocumentTab* tab)
{
  foreach(QMdiSubWindow* subWindow, mdiArea->subWindowList())
  {
    if(dynamic_cast<AbstractDocumentTab *>(subWindow->widget()) == tab)
    {
      mdiArea->setActiveSubWindow(subWindow);
      return true;
    }
  }
  return false;       //no such tab found
}


QString qavimator::selectFileToOpen(const QString& caption)
{
   //// For some unknown reason passing "this" locks up the OSX qavimator window. Possibly a QT4 bug, needs investigation
#ifdef __APPLE__
  //  quit();
  QString file=QFileDialog::getOpenFileName(NULL, caption, Settings::Instance()->lastPath(), ANIM_FILTER);
#else
   QString file=QFileDialog::getOpenFileName(this, caption, Settings::Instance()->lastPath(), ANIM_FILTER);
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
      Settings::Instance()->setLastPath(fileInfo.canonicalPath());
  }

  return file;
}

// Menu action: File / Open ...

void qavimator::fileOpen(const QString& name)
{
  if(!name.isEmpty())
  {
    //check if not already open
    foreach(AbstractDocumentTab* tab, openTabs())
    {
      if(tab->CurrentFile == name)
      {
        activateTab(tab);
        return;
      }
    }

    NewFileDialog::ProjectType filetype;

    if(name.endsWith(".avm", Qt::CaseInsensitive) || name.endsWith(".bvh", Qt::CaseInsensitive))
      filetype = NewFileDialog::AVM;
    else if(name.endsWith(".avbl", Qt::CaseInsensitive))
      throw "Not implemented yet";
    else
      throw "Unknown file extension";

    OpenNewTab(filetype, name, false);
  }
}


// Menu Action: File / Quit
void qavimator::quit()
{
//  if(!clearOpenFiles())
//    return;

  if(!resolveUnsavedTabs())
    return;

  Settings::Instance()->WriteSettings();
  
  // remove all widgets and close the main form
  qApp->exit(0);
}


bool qavimator::resolveUnsavedTabs()
{
  QList<AbstractDocumentTab*> unsaved;
  foreach(QMdiSubWindow* subWindow, mdiArea->subWindowList())
  {
    AbstractDocumentTab* tab = dynamic_cast<AbstractDocumentTab*>(subWindow->widget());
    if(tab->IsUnsaved())
      unsaved.append(tab);
  }

  if(unsaved.count())
  {
    SaveChangesDialog* dialog = new SaveChangesDialog(this, unsaved);
    dialog->exec();

    if(dialog->result() == QDialog::Accepted)
    {
      delete dialog;
      return true;
    }

    return false;
  }
  else
    return true;     //no unsaved changes
}

// Menu Action: Edit / Cut
/*rbsh
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
}     */


// Menu Action: Options / Configure QAvimator
void qavimator::configure()
{
  SettingsDialog* dialog=new SettingsDialog(this);
  connect(dialog,SIGNAL(configChanged()),this,SLOT(configChanged()));

  dialog->exec();

  delete dialog;
}

//only re-emits further
void qavimator::configChanged()
{
//rbsh  animationView->repaint();                 //TODO: inside Tabs of course
  emit configurationChanged();
}

// Menu Action: Help / About ...
void qavimator::helpAbout()
{
  QMessageBox::about(this,QObject::tr("About QAvimator"),
                     QObject::tr("QAvimator - Animation editor for Second Life<br />%1").arg(SVN_ID));
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

/*rbsh
// Adds a file to the open files list, and adds the entry in the combo box
void qavimator::addToOpenFiles(const QString& fileName)
{
    openFiles.append(fileName);

    QString fixedName=fileName;         */
//rbsh as well    QRegExp pattern(".*/");
/*rbsh    fixedName.remove(pattern);
    pattern.setPattern("(\\.bvh|\\.avm)");
    fixedName.remove(pattern);

    selectAnimationCombo->addItem(fixedName);
}

void qavimator::removeFromOpenFiles(unsigned int which)
{
  if(which>= (unsigned int) openFiles.count()) return;
  openFiles.removeAt(which);
  selectAnimationCombo->removeItem(which);
}         */


// convenience function to set window title in a defined way
void qavimator::setCurrentFile(const QString& fileName)           //TODO: to setWindowTitle
{
  setWindowTitle("qavimator ["+fileName+"]");
}



// prevent closing of main window if there are unsaved changes
void qavimator::closeEvent(QCloseEvent* event)
{
  if(/*!clearOpenFiles()*/ !resolveUnsavedTabs())
    event->ignore();
  else
    event->accept();
}


//slot called by AbstractDocumentTab's inner QTabBar that wishes to be closed
void qavimator::closeTab(int i)
{
  QMdiSubWindow* sub = mdiArea->subWindowList()[i];
  mdiArea->setActiveSubWindow(sub);
  mdiArea->closeActiveSubWindow();
}


// -------------------------------------------------------------------------
// autoconnection from designer UI

// ------- Menu Action Slots --------

void qavimator::on_fileNewAction_triggered()
{
  NewFileDialog* dialog = new NewFileDialog(this);
  dialog->exec();

  if(dialog->result() == QDialog::Accepted)
  {
    OpenNewTab(dialog->SelectedProjectType(), dialog->FileName(), true);
  }

  delete dialog;
}

void qavimator::on_fileOpenAction_triggered()
{
  QString file = selectFileToOpen(tr("Select Animation File to Load"));

  fileOpen(file);
}

void qavimator::on_fileSaveAction_triggered()
{
  activeTab()->Save();
}

void qavimator::on_fileSaveAsAction_triggered()
{
  activeTab()->SaveAs();
}

void qavimator::on_fileCloseAction_triggered()
{
  mdiArea->activeSubWindow()->close();
}

void qavimator::on_fileExportForSecondLifeAction_triggered()
{
  activeTab()->ExportForSecondLife();
}

void qavimator::on_fileQuitAction_triggered()
{
  quit();
}

/*rbsh
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
}   */

void qavimator::on_optionsConfigureQAvimatorAction_triggered()
{
  configure();
}

void qavimator::on_helpAboutAction_triggered()
{
  helpAbout();
}


// ------- UI Element Slots --------


void qavimator::on_mdiArea_subWindowActivated(QMdiSubWindow*)
{
  //as there is no such meaningful event in a document tab
  //we need to nudge it explicitely
  if(activeTab())
    activeTab()->onTabActivated();

  if(activeTab())
    setCurrentFile(activeTab()->CurrentFile);

  UpdateMenus();
  UpdateToolbar();
}

// end autoconnection from designer UI
// -------------------------------------------------------------------------
