#define FILE_FILTER "Blended animation files (*.avbl)"
#define AVM_FILTER "Animation files (*.avm)"


#include <QMessageBox>
#include <QFileDialog>
#include <QCloseEvent>
#include <QStringListModel>

#include "BlenderTab.h"
#include "ui_BlenderTab.h"
#include "qavimator.h"
#include "animationview.h"
#include "settings.h"


BlenderTab::BlenderTab(qavimator* mainWindow, const QString& fileName, bool createFile)
  : QWidget(0), AbstractDocumentTab(mainWindow)
{
  /*ui->*/setupUi(this);

  setAttribute(Qt::WA_DeleteOnClose);

  connect(this, SIGNAL(resetCamera()), blenderAnimationView, SLOT(resetCamera()));
  connect(blenderTimeline, SIGNAL(resultingAnimationChanged(WeightedAnimation*)), blenderPlayer, SLOT(onAnimationChanged(WeightedAnimation*)));
  connect(blenderTimeline, SIGNAL(resultingAnimationChanged(WeightedAnimation*)), this, SLOT(onTimelineAnimationChanged(WeightedAnimation*)));

  if(createFile)
  {
    CurrentFile = fileName;
    fileNew();
    Save();
  }
  else
    fileOpen(fileName);

  bindMenuActions();
}

BlenderTab::~BlenderTab()
{
//  delete ui;
}


void BlenderTab::bindMenuActions()
{
/*  connect(mainWindow->optionsLoopAction, SIGNAL(toggled(bool)), this, SLOT(optionsLoopAction_toggled(bool)));
  connect(mainWindow->optionsProtectFirstFrameAction, SIGNAL(triggered(bool)), this, SLOT(optionsProtectFirstFrameAction_toggled(bool)));   */
}

bool BlenderTab::IsUnsaved()
{
  //TODO
  return true;
}

void BlenderTab::AddFile()
{
  animsList->addNewFile();
}

void BlenderTab::Save()
{
  sorry();
}

void BlenderTab::SaveAs()
{
  sorry();
}

void BlenderTab::Cut()
{
  sorry();
}

void BlenderTab::Copy()
{
  sorry();
}

void BlenderTab::Paste()
{
  sorry();
}

void BlenderTab::ResetView()
{
  emit resetCamera();
}

void BlenderTab::ExportForSecondLife()
{
  sorry();
}

void BlenderTab::UpdateToolbar()
{
  mainWindow->resetCameraAction->setVisible(true);
  mainWindow->resetCameraAction->setEnabled(true);
  mainWindow->fileAddAction->setEnabled(true);
}

void BlenderTab::UpdateMenu()
{
  //Note: actions shared with Toolbar are handled in UpdateToolbar()
  mainWindow->fileLoadPropsAction->setEnabled(false);
  mainWindow->fileSavePropsAction->setEnabled(false);

  mainWindow->toolsOptimizeBVHAction->setEnabled(false);
  mainWindow->toolsMirrorAction->setEnabled(false);

  mainWindow->optionsJointLimitsAction->setEnabled(false);
  mainWindow->optionsLoopAction->setEnabled(true);
  mainWindow->optionsProtectFirstFrameAction->setEnabled(false);
  mainWindow->optionsShowTimelineAction->setEnabled(false);
  mainWindow->optionsSkeletonAction->setEnabled(false);
}

void BlenderTab::onTabActivated()
{
  // hack to get 3D view back in shape
//  qApp->processEvents();
  blenderAnimationView->makeCurrent();
  QSize oldSize = blenderAnimationView->size();
  blenderAnimationView->resize(oldSize.width(), oldSize.height()-1);
  qApp->processEvents();
  blenderAnimationView->resize(oldSize);

  UpdateToolbar();
  UpdateMenu();
}

// convenience function to set window title in a defined way
void BlenderTab::setCurrentFile(const QString& fileName)
{
  CurrentFile=fileName;
  mainWindow->setWindowTitle("Animik ["+CurrentFile+"]");       //edu: Fuj! TODO: encapsulate.
  QFileInfo fileInfo(fileName);
  setWindowTitle(fileInfo.fileName());
}

// Menu action: File / Open ...
void BlenderTab::fileOpen()
{
  fileOpen(QString::null);
}

void BlenderTab::fileOpen(const QString& name)
{
  fileAdd(name);
}

void BlenderTab::fileAdd()
{
  fileAdd(QString::null);
}

QString BlenderTab::selectFileToOpen(const QString& caption)        //TODO: resolve the ugly code multiplication
{
   //// For some unknown reason passing "this" locks up the OSX qavimator window. Possibly a QT4 bug, needs investigation
#ifdef __APPLE__
  QString file=QFileDialog::getOpenFileName(NULL,caption, Settings::Instance()->lastPath(), FILE_FILTER);
#else
   QString file=QFileDialog::getOpenFileName(this,caption,Settings::Instance()->lastPath(), FILE_FILTER);
#endif
  if(!file.isEmpty())
  {
    QFileInfo fileInfo(file);
    if(!fileInfo.exists())
    {
      QMessageBox::warning(this,QObject::tr("Load Animation File"),
                           QObject::tr("<qt>Animation file not found:<br />%1</qt>").arg(file));
      file=QString::null;
    }
    else
      Settings::Instance()->setLastPath(fileInfo.path());
  }

  return file;
}

void BlenderTab::fileAdd(const QString& name)
{
  QString file=name;

  if(file.isEmpty())
    file=selectFileToOpen(tr("Select Animation File to Add"));

  if(!file.isEmpty())
  {
    // handling of non-existant file names
    if(!QFile::exists(file))
    {
      QMessageBox::warning(this, QObject::tr("Load Animation File"),
                           QObject::tr("<qt>Animation file not found:<br />%1</qt>").arg(file));
      return;
    }

    setCurrentFile(file);

/*TODO    blenderAnimationView->addAnimation(anim);
    blenderTimeline->AddAnimation(anim);
    blenderTimeline->selectAnimation(anim);
*/

/*TODO    if(avblFile->getLoopInPoint()==-1)
    {
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
        blenderTimeline->setFrame(0);
        setCurrentFrame(0);
        setLoopInPoint(1);
      }
    }
    else
    {
//      qDebug("qavimator::fileAdd(): reading saved loop points");
      setLoopInPoint(anim->getLoopInPoint()+1);
      setLoopOutPoint(anim->getLoopOutPoint()+1);
    }     */

//TODO?    connect(anim,SIGNAL(currentFrame(int)),this,SLOT(setCurrentFrame(int)));
  }
}


// prevent closing of main window if there are unsaved changes
void BlenderTab::closeEvent(QCloseEvent* event)
{
  if(!resolveUnsavedChanges())
    event->ignore();
  else
    event->accept();
}

bool BlenderTab::resolveUnsavedChanges()
{
  if(IsUnsaved())
  {
    QMessageBox saveMessage;
    saveMessage.setText(tr("The file %1 has been modified. Do you want to save it?").arg(CurrentFile));
    saveMessage.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    saveMessage.setDefaultButton(QMessageBox::Save);
    int answer = saveMessage.exec();
    if(answer==QMessageBox::Save)
      Save();
    else if(answer==QMessageBox::Cancel)
      return false;
  }

  return true;
}

void BlenderTab::fileNew()
{
  //TODO: open empty scene and timeline
}


// ------- Autoconnection of UI elements -------- //

void BlenderTab::on_animsList_AnimationFileTaken(QString filename)
{
  WeightedAnimation* anim=new WeightedAnimation(blenderAnimationView->getBVH(),filename);
  QFileInfo fInfo(filename);
  if(!blenderTimeline->AddAnimation(anim, fInfo.completeBaseName()))
    QMessageBox::warning(this, "Error loading animation", "Can't add animation file '" +
                         filename + "' to timeline. Not enough space.");
}

// ---------------------------------------------- //

// --------------- other slots ------------------ //
void BlenderTab::onTimelineAnimationChanged(WeightedAnimation* anim)
{
  blenderAnimationView->setAnimation(anim);
}

// ---------------------------------------------- //




////////////////////////////////////////////// DEBUG //////////////////////////////////////////////

void BlenderTab::sorry()
{
  QMessageBox::warning(this, "Under construction", "Not implemented yet. Sorry.");
}

