#ifndef BLENDERTAB_H
#define BLENDERTAB_H

#include <QWidget>
#include <ui_BlenderTab.h>

#include "AbstractDocumentTab.h"

class qavimator;
class QAction;
class QCloseEvent;
class QFileInfo;
class QStringList;


/*namespace Ui {
    class BlenderTab;
}*/

class BlenderTab : public QWidget, public Ui::BlenderTab, public AbstractDocumentTab {
  Q_OBJECT

  public:
    BlenderTab(/*QWidget *parent = 0*/qavimator* mainWindow, const QString& fileName, bool createFile);
    ~BlenderTab();

    virtual void AddFile();
    virtual void Save();
    virtual void SaveAs();
    virtual void Cut();
    virtual void Copy();
    virtual void Paste();
    virtual void Undo() {/*TODO*/};
    virtual void Redo() {/*TODO*/};
    virtual void ResetView();
    virtual void ExportForSecondLife();
    virtual void UpdateToolbar();
    virtual void UpdateMenu();
    virtual bool IsUnsaved();

  signals:
    void resetCamera();

  public slots:
    virtual void onTabActivated();
    virtual void onTabDeactivated();
    void onTimelineAnimationChanged(WeightedAnimation* anim);

  protected:
    // prevent closing of main window if there are unsaved changes
    virtual void closeEvent(QCloseEvent* event);
    /** Handling key presses mainly for time-line */
    virtual void keyPressEvent(QKeyEvent *);

    //DEBUG so far
    virtual void contextMenuEvent(QContextMenuEvent *event);

    void setCurrentFile(const QString& fileName);
    QString selectFileToOpen(const QString& caption);
    void fileNew();
    void fileOpen();
    void fileOpen(const QString& fileName);
    void fileSaveAs();

    void fileAdd();
    void fileAdd(const QString& fileName);
    bool checkFileOverwrite(const QFileInfo& fileInfo);

    void fileExportForSecondLife();
    bool resolveUnsavedChanges();

    void editCut();
    void editCopy();
    void editPaste();

  private:
    QAction* setLimbWeightsAction;
    void bindMenuActions();
    bool canShowWarn;
    /** TRUE for unsaved content */
    bool isDirty;


    void setSelectedLimbsWeight(QList<int>* jointNumbers);

    /////////////edu: DEBUG /////////////
    void sorry();
    /////////////////////////////////////

  private slots:
    void on_animsList_AnimationFileTaken(QString filename, int orderInBatch, int batchSize);
    void on_zoomOutButton_clicked();
    void on_zoomInButton_clicked();
    void onConfigChanged();
    void onLimbWeights();
    void onLimbDoubleClicked(int jointNumber);
    void onLimbsDialogPreviousFrame();
    void onLimbsDialogNextFrame();
    void onPlaybackStarted();
    void onPlaybackPaused();
};

#endif // BLENDERTAB_H
