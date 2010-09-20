#ifndef ABSTRACTDOCUMENTTAB_H
#define ABSTRACTDOCUMENTTAB_H

#include <QWidget>

#include "NewFileDialog.h"        //TODO

class qavimator;


/**
  Abstract base class for all document tabs. Ensures all common actions like
  saving a document, copy/paste etc. Offers implementation of some really common.
  */
class AbstractDocumentTab
{
public:
  AbstractDocumentTab(qavimator* mainWindow);

  //Factory methods to return apropriate tab type based on file extension or just demand
  static AbstractDocumentTab* GetNewTab(qavimator* mainWindow, QString& fileFullPath);
  static AbstractDocumentTab* GetNewTab(qavimator* mainWindow, NewFileDialog::ProjectType tabType);

//public slots:
  /** Load file in this tab. Supposed to be called once,
      right after tab creation (opening existing file)*/
  virtual void Open(const QString& fileName) = 0;
  virtual void Save() = 0;
  virtual void SaveAs() = 0;
  virtual void Cut() = 0;
  virtual void Copy() = 0;
  virtual void Paste() = 0;
  virtual void Undo() = 0;
  virtual void Redo() = 0;
  virtual void ExportForSecondLife() = 0;

  //Update display state of buttons in the toolbar of main window
  virtual void UpdateToolbar() = 0;

  virtual void onTabActivated() = 0;

protected:
  qavimator* mainWindow;
  //current file name
  QString CurrentFile;

  //Constants
  virtual QString UntitledName() const = 0;
};

#endif // ABSTRACTDOCUMENTTAB_H
