#include "AbstractDocumentTab.h"
#include "qavimator.h"
#include "KeyFramerTab.h"



AbstractDocumentTab::AbstractDocumentTab(qavimator* mainWindow)
{
  this->mainWindow = mainWindow;
}


AbstractDocumentTab* AbstractDocumentTab::GetNewTab(qavimator* mainWindow, QString& fileFullPath)
{
  if(fileFullPath.endsWith(".avm", Qt::CaseInsensitive) ||
     fileFullPath.endsWith(".bvh", Qt::CaseInsensitive))
    return new KeyFramerTab(mainWindow, fileFullPath);
  if(fileFullPath.endsWith(".avbl", Qt::CaseInsensitive))
    throw "Not implemented yet";
  else
    return 0;
}


AbstractDocumentTab* AbstractDocumentTab::GetNewTab(qavimator* mainWindow, NewFileDialog::ProjectType tabType)
{
  switch(tabType)
  {
  case NewFileDialog::AVM : return new KeyFramerTab(mainWindow, "");
  case NewFileDialog::AVBL : ;  //TODO
  default : return 0;
  }
}
