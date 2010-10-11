#ifndef ABSTRACTDOCUMENTTAB_H
#define ABSTRACTDOCUMENTTAB_H

#include <QWidget>

#include "NewFileDialog.h"        //TODO

class qavimator;


/**
  Abstract base class for all document tabs. Ensures all common actions like
  saving a document, copy/paste etc. Offers implementation of some really common.
  */
class AbstractDocumentTab : public QVariant
{
public:
  AbstractDocumentTab(qavimator* mainWindow);

  //current file name
  QString CurrentFile;
  virtual bool IsUnsaved() = 0;       //TODO: think of the negative

//public slots:
  virtual void AddFile() = 0;
  virtual void Save() = 0;
  virtual void SaveAs() = 0;
  virtual void Cut() = 0;
  virtual void Copy() = 0;
  virtual void Paste() = 0;
  virtual void Undo() = 0;
  virtual void Redo() = 0;
  virtual void ResetView() = 0;
  virtual void ExportForSecondLife() = 0;

  //Update display state of buttons in the toolbar of main window
  virtual void UpdateToolbar() = 0;
  //Update menu of main window
  virtual void UpdateMenu() = 0;

  virtual void onTabActivated() = 0;

protected:
  qavimator* mainWindow;
};

#endif // ABSTRACTDOCUMENTTAB_H
