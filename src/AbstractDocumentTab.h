#ifndef ABSTRACTDOCUMENTTAB_H
#define ABSTRACTDOCUMENTTAB_H


/**
  Abstract base class for all document tabs. Ensures all common operation like
  saving a document, copy/paste etc.
  */
class AbstractDocumentTab
{
public:
  virtual void Save() = 0;

  //Update display state of buttons in the toolbar of main window
  virtual void UpdateToolbar();
  //TODO: more

};

#endif // ABSTRACTDOCUMENTTAB_H
