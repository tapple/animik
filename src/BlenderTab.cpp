#define FILE_FILTER "Blended animation files (*.avbl)"
#define AVM_FILTER "Animation files (*.avm)"


#include <QMessageBox>
#include <QFileDialog>
#include <QCloseEvent>
#include <QStringListModel>

#include "animationview.h"
#include "Avbl.h"
#include "BlenderTab.h"
#include "LimbsWeightDialog.h"
#include "OptionalMessageBox.h"
#include "qavimator.h"
#include "settings.h"
#include "TrailItem.cpp"
#include "ui_BlenderTab.h"



BlenderTab::BlenderTab(qavimator* mainWindow, const QString& fileName, bool createFile)
  : QWidget(0), AbstractDocumentTab(mainWindow)
{
  setupUi(this);
  blenderAnimationView->setUseRotationHelpers(false);
  blenderAnimationView->setUseIK(false);
  blenderAnimationView->setMultiPartPicking(true);
  blenderAnimationView->setShowingPartInfo(true);
  blenderAnimationView->setUseMirroring(false);
  canShowWarn = false;
  isDirty = false;

  setAttribute(Qt::WA_DeleteOnClose);


  setLimbWeightsAction = new QAction(tr("Set limbs' weights"), this);
  connect(setLimbWeightsAction, SIGNAL(triggered()), this, SLOT(onLimbWeights()));
  connect(mainWindow, SIGNAL(configurationChanged()), this, SLOT(onConfigChanged()));
  connect(blenderAnimationView, SIGNAL(partDoubleClicked(int)), this, SLOT(onLimbDoubleClicked(int)));
  connect(this, SIGNAL(resetCamera()), blenderAnimationView, SLOT(resetCamera()));
  connect(blenderTimeline, SIGNAL(resultingAnimationChanged(WeightedAnimation*)),
          blenderPlayer, SLOT(onAnimationChanged(WeightedAnimation*)));
  connect(blenderTimeline, SIGNAL(resultingAnimationChanged(WeightedAnimation*)),
          this, SLOT(onTimelineAnimationChanged(WeightedAnimation*)));
  connect(blenderPlayer, SIGNAL(playbackStarted()), this, SLOT(onPlaybackStarted()));
  connect(blenderPlayer, SIGNAL(playbackPaused()), this, SLOT(onPlaybackPaused()));

  if(createFile)
  {
    isDirty = true;
    setCurrentFile(fileName);
//    fileNew();
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
  return isDirty;
}

void BlenderTab::AddFile()
{
  animsList->addNewFile();
}

void BlenderTab::Save()
{
  Avbl* saver = new Avbl();
  bool saved = saver->SaveToFile(blenderTimeline->Trails(), CurrentFile);
  isDirty = !saved;
  setCurrentFile(CurrentFile);      //asterisk out
}

void BlenderTab::SaveAs()
{
  fileSaveAs();
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
  fileExportForSecondLife();
}

void BlenderTab::UpdateToolbar()
{
  mainWindow->resetCameraAction->setVisible(true);
  mainWindow->resetCameraAction->setEnabled(true);
  mainWindow->fileAddAction->setEnabled(true);
  mainWindow->editCutAction->setVisible(false);
}

void BlenderTab::UpdateMenu()
{
  //Note: actions shared with Toolbar are handled in UpdateToolbar()
  mainWindow->fileLoadPropsAction->setEnabled(false);
  mainWindow->fileSavePropsAction->setEnabled(false);
  mainWindow->fileExportForSecondLifeAction->setEnabled(!blenderTimeline->isClear());

  mainWindow->toolsOptimizeBVHAction->setEnabled(false);
  mainWindow->toolsMirrorAction->setEnabled(false);

  mainWindow->optionsJointLimitsAction->setEnabled(false);
  mainWindow->optionsLoopAction->setEnabled(!blenderTimeline->isClear());
  mainWindow->optionsProtectFirstFrameAction->setEnabled(false);
  mainWindow->optionsShowTimelineAction->setEnabled(false);
  mainWindow->optionsSkeletonAction->setEnabled(true);
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
  blenderPlayer->suspend(false);

  UpdateToolbar();
  UpdateMenu();
}

void BlenderTab::onTabDeactivated()
{
  blenderPlayer->suspend(true);
}

// convenience function to set window title in a defined way
void BlenderTab::setCurrentFile(const QString& fileName)
{
  CurrentFile = fileName;
  QString a = isDirty ? "*" : "";
  mainWindow->setWindowTitle("Animik [" + CurrentFile + a +"]");
  QFileInfo fileInfo(CurrentFile);
  setWindowTitle(fileInfo.fileName() + a);
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


/** Open saved AVBL file from hard drive. */
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
                           QObject::tr("<qt>Blended animation file not found:<br />%1</qt>").arg(file));
      return;
    }

    Avbl loader;
    bool oldDebug = Settings::Instance()->Debug();        //temporarily turn off DEBUG mode to save
    Settings::Instance()->setDebug(false);                //the hassle connected with building a time-line
    QList<TrailItem*>* trails = loader.LoadFromFile(file);
    blenderTimeline->ConstructTimeLine(trails);
    if(oldDebug)
    {
      Settings::Instance()->setDebug(oldDebug);
      blenderTimeline->RebuildResultingAnimation();       //add shadow (helper) TrailItems
    }
    isDirty = false;
    setCurrentFile(file);
  }
}


// Menu Action: File / Save As...
void BlenderTab::fileSaveAs()         //Ugly code repetition. TODO: think of it (I mean act)
{
   //// For some unknown reason passing "this" locks up the OSX qavimator window. Possibly a QT4 bug, needs investigation
#ifdef __APPLE__
   QString file=QFileDialog::getSaveFileName(NULL, tr("Save Composition File"), CurrentFile,
                                             "Composition Files (*.avbl)", 0,
                                             QFileDialog::DontConfirmOverwrite);
#else
   QString file=QFileDialog::getSaveFileName(this, tr("Save Composition File"), CurrentFile,
                                             "Composition Files (*.avbl)", 0,
                                             QFileDialog::DontConfirmOverwrite);
#endif

  if(!file.isEmpty())
  {
    QFileInfo fileInfo(file);

    // make sure file has proper extension (either .bvh or .avm)
    QString extension=fileInfo.suffix().toLower();
    if(extension!="avbl")
      file+=".avbl";

    // if the file didn't exist yet or the user accepted to overwrite it, save it.
    if(checkFileOverwrite(fileInfo))
    {
      Settings::Instance()->setLastPath(fileInfo.path());

      Avbl* saver = new Avbl();
      bool saved = saver->SaveToFile(blenderTimeline->Trails(), file);
      isDirty = !saved;
      setCurrentFile(file);

      mainWindow->fileExportForSecondLifeAction->setEnabled(true);      //TODO: why?
    }
  }
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


// checks if a file already exists at the given path and displays a warning message
// returns true if it's ok to save/overwrite, else returns false
bool BlenderTab::checkFileOverwrite(const QFileInfo& fileInfo)      //VERY UGLY CODE CLONE (FROM KeyFramerTab). TODO: parent class? Or some FileHelper?
{
  // get file info
  if(fileInfo.exists())
  {
    int answer=QMessageBox::question(this,tr("File Exists"),tr("A file with the name \"%1\" does already exist. Do you want to overwrite it?").arg(fileInfo.fileName()),QMessageBox::Yes,QMessageBox::No,QMessageBox::NoButton);
    if(answer==QMessageBox::No) return false;
  }
  return true;
}


// prevent closing of main window if there are unsaved changes
void BlenderTab::closeEvent(QCloseEvent* event)
{
  if(!resolveUnsavedChanges())
    event->ignore();
  else
    event->accept();
}


void BlenderTab::keyPressEvent(QKeyEvent* e)
{
  if(e->key()==Qt::Key_Space)
    blenderPlayer->PlayOrPause();
  else if(e->key()==Qt::Key_Left && blenderPlayer->state()==PLAYSTATE_STOPPED)
    blenderPlayer->StepBackward();
  else if(e->key()==Qt::Key_Right && blenderPlayer->state()==PLAYSTATE_STOPPED)
    blenderPlayer->StepForward();
  else if(e->key()==Qt::Key_Home && blenderPlayer->state()==PLAYSTATE_STOPPED)
    blenderPlayer->SkipToFirst();
  else if(e->key()==Qt::Key_End && blenderPlayer->state()==PLAYSTATE_STOPPED)
    blenderPlayer->SkipToLast();
  else
    e->ignore();            //send it to parent
}


void BlenderTab::contextMenuEvent(QContextMenuEvent *event)
{
  if(blenderTimeline->getSelectedItem() != NULL && !blenderAnimationView->getSelectedPartIndices()->isEmpty())
  {
    QMenu menu(this);
    menu.addAction(setLimbWeightsAction);
    menu.exec(event->globalPos());
  }
}


// Menu Action: File / Export For Second Life
void BlenderTab::fileExportForSecondLife()
{
  QString exportName = QFileDialog::getSaveFileName(this, "Save exported file", Settings::Instance()->lastPath(), BVH_FILTER);

  if(exportName != "")
  {
    qDebug("BlenderTab::fileExportForSecondLife(): exporting animation as '%s'.", exportName.toLatin1().constData());
    if(!exportName.endsWith(".bvh", Qt::CaseInsensitive))
      exportName += ".bvh";
    blenderAnimationView->getAnimation()->saveBVH(exportName);
  }
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


void BlenderTab::setSelectedLimbsWeight(QList<int>* jointNumbers)
{
  TrailItem* selectedItem = blenderTimeline->getSelectedItem();
  if(selectedItem != NULL && !selectedItem->isShadow() && !blenderPlayer->isPlaying())
  {
    QList<BVHNode*> limbList;
    foreach(int jointNumber, *jointNumbers)
      limbList << selectedItem->getAnimation()->getNode(jointNumber);

    blenderTimeline->HideLimsForm();
    LimbsWeightDialog* lwd = new LimbsWeightDialog(selectedItem->Name, &limbList, selectedItem->selectedFrame(),
                                                   selectedItem->frames(), this);
    connect(lwd, SIGNAL(nextFrame()), this, SLOT(onLimbsDialogNextFrame()));
    connect(lwd, SIGNAL(previousFrame()), this, SLOT(onLimbsDialogPreviousFrame()));
    if(lwd->exec() == QDialog::Accepted)
    {
      isDirty = true;
      setCurrentFile(CurrentFile);      //asterisk
      blenderTimeline->RebuildResultingAnimation();
    }
    lwd->disconnect();
    delete lwd;
  }
}

// ------- Autoconnection slots of UI elements -------- //

void BlenderTab::on_animsList_AnimationFileTaken(QString filename, int orderInBatch, int batchSize)
{
  WeightedAnimation* anim = new WeightedAnimation(blenderAnimationView->getBVH(), filename);
  if(anim->isFirstFrameTPose())
    canShowWarn = true;

  QFileInfo fInfo(filename);
  if(!blenderTimeline->AddAnimation(anim, fInfo.completeBaseName()))
    QMessageBox::warning(this, "Error loading animation", "Can't add animation file '" +
                         filename + "' to timeline. Not enough space.");

  if(canShowWarn && orderInBatch==batchSize && Settings::Instance()->tPoseWarning())
  {
    QString* message = new QString("Animation you are adding to the time-line has initial posture of T-pose.\r\n");
    message->append("This is useful for Second Life animations but may be undesirable when joining multiple animations. ");
    message->append("To adjust such animation, select 'Cut initial T-pose frame' from it's context menu.");
    OptionalMessageBox* omb = new OptionalMessageBox("Initial T-pose", *message, this);
    omb->exec();
    Settings::Instance()->setTPoseWarning(omb->showAgain());
    canShowWarn = false;
  }
}

void BlenderTab::on_zoomInButton_clicked()
{
  //TODO
}

void BlenderTab::on_zoomOutButton_clicked()
{
  //TODO
}

// ---------------------------------------------- //

// --------------- other slots ------------------ //
void BlenderTab::onTimelineAnimationChanged(WeightedAnimation* anim)
{
  isDirty = true;
  setCurrentFile(CurrentFile);    //update asterisk
  blenderAnimationView->setAnimation(anim);
  UpdateMenu();                   //ex. export action
}

/** There might be settings that affect the way the resulting animation is built.
    So it must be reevaluated after changes in settings. Only known so far is switching DEBUG mode on/off. */
void BlenderTab::onConfigChanged()
{
  blenderTimeline->RebuildResultingAnimation(false);
}


void BlenderTab::onLimbWeights()
{
  QList<int>* parts = blenderAnimationView->getSelectedPartIndices();
  if(!parts->isEmpty())
    setSelectedLimbsWeight(parts);
}

void BlenderTab::onLimbDoubleClicked(int jointNumber)
{
  QList<int>* parts = new QList<int>();
  parts->append(jointNumber);
  setSelectedLimbsWeight(parts);
}


void BlenderTab::onLimbsDialogNextFrame()
{
  blenderPlayer->StepForward();
}

void BlenderTab::onLimbsDialogPreviousFrame()
{
  blenderPlayer->StepBackward();
}


void BlenderTab::onPlaybackStarted()
{
  blenderTimeline->limitUserActions(true);
}

void BlenderTab::onPlaybackPaused()
{
  blenderTimeline->limitUserActions(false);
}

// ---------------------------------------------- //




////////////////////////////////////////////// DEBUG //////////////////////////////////////////////

void BlenderTab::sorry()
{
  QMessageBox::warning(this, "Under construction", "Not implemented yet. Sorry.");
}
