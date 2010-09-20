#ifndef KEYFRAMERTAB_H
#define KEYFRAMERTAB_H


#define PLAY_IMAGE "data/play.png"
#define PAUSE_IMAGE "data/pause.png"
#define KEY_IMAGE "data/key.png"
#define NOKEY_IMAGE "data/nokey.png"

#include <QWidget>

#include "AbstractDocumentTab.h"
#include "ui_KeyFramerTab.h"
#include "bvhnode.h"
#include "animation.h"
#include "prop.h"
#include "rotation.h"
#include "playstate.h"

class qavimator;
class Animation;
class Prop;
class Timeline;
class QSlider;
class QLineEdit;
class QCloseEvent;



/*namespace Ui {
    class KeyFramerTab;
}*/

class KeyFramerTab : public QWidget, public Ui::KeyFramerTab, public AbstractDocumentTab {
    Q_OBJECT
public:
    /**
      Main window is passed to access some menu and toolbar actions
    */
  KeyFramerTab(/*QWidget *parent = 0, */qavimator* mainWindow, const QString& fileName);
  ~KeyFramerTab();

  virtual void Save();
  virtual void SaveAs();
  virtual void Cut();
  virtual void Copy();
  virtual void Paste();
  virtual void Undo() {/*TODO*/};
  virtual void Redo() {/*TODO*/};
  virtual void ExportForSecondLife();
  virtual void UpdateToolbar();

signals:
    void enableRotation(bool state);
    void enablePosition(bool state);
    void enableProps(bool state);
    void enableEaseInOut(bool state);
    void resetCamera();
    void protectFrame(bool state);

public slots:
    virtual void Open(const QString& fileName);

protected slots:
    void readSettings();

    void partClicked(BVHNode* node,Rotation rot,RotationLimits rotLimits,Position pos);
    void partDragged(BVHNode* node,double changeX,double changeY,double changeZ);
    void propClicked(Prop* prop);
    void propDragged(Prop* prop,double x,double y,double z);
    void propScaled(Prop* prop,double x,double y,double z);
    void propRotated(Prop* prop,double x,double y,double z);
    void backgroundClicked();

    void frameTimeout();

    void setCurrentFrame(int frame);

    void selectAnimation(Animation* animation);
    void clearProps();

    // autoconnection from designer UI

    // ------- Menu Action Slots --------
//    void fileNewAction_triggered();
//edu: it's up    void fileOpenAction_triggered();
    void fileAddAction_triggered();
//    void fileSaveAction_triggered();
    void fileSaveAsAction_triggered();
    void fileExportForSecondLifeAction_triggered();
    void fileLoadPropsAction_triggered();
    void fileSavePropsAction_triggered();
    void fileExitAction_triggered();

    void toolsOptimizeBVHAction_triggered();
    void toolsMirrorAction_triggered();

    void optionsSkeletonAction_toggled(bool on);
    void optionsJointLimitsAction_toggled(bool on);
    void optionsLoopAction_toggled(bool on);
    void optionsProtectFirstFrameAction_toggled(bool on);
    void optionsShowTimelineAction_toggled(bool on);

    // ------- Additional Toolbar Element Slots -------

    void resetCameraAction_triggered();

    // ------- UI Element Slots --------
    void on_selectAnimationCombo_activated(int);
    void on_figureCombo_activated(int);
    void on_scaleSpin_valueChanged(int newValue);
    void on_editPartCombo_activated(int);
    void on_xRotationEdit_returnPressed();
    void on_xRotationEdit_lostFocus();
    void on_xRotationSlider_valueChanged(int);
    void on_yRotationEdit_returnPressed();
    void on_yRotationEdit_lostFocus();
    void on_yRotationSlider_valueChanged(int);
    void on_zRotationEdit_returnPressed();
    void on_zRotationEdit_lostFocus();
    void on_zRotationSlider_valueChanged(int);
    void on_xPositionEdit_returnPressed();
    void on_xPositionEdit_lostFocus();
    void on_xPositionSlider_valueChanged(int);
    void on_yPositionEdit_returnPressed();
    void on_yPositionEdit_lostFocus();
    void on_yPositionSlider_valueChanged(int);
    void on_zPositionEdit_returnPressed();
    void on_zPositionEdit_lostFocus();
    void on_zPositionSlider_valueChanged(int);
    void on_easeInCheck_stateChanged(int newState);
    void on_easeOutCheck_stateChanged(int newState);

    void on_newBoxPropButton_clicked();
    void on_newSpherePropButton_clicked();
    void on_newConePropButton_clicked();
    void on_newTorusPropButton_clicked();
    void on_propNameCombo_activated(const QString& name);
    void on_deletePropButton_clicked();
    void on_attachToComboBox_activated(int attachmentPoint);
    void on_propXPosSpin_valueChanged(int);
    void on_propYPosSpin_valueChanged(int);
    void on_propZPosSpin_valueChanged(int);
    void on_propXScaleSpin_valueChanged(int);
    void on_propYScaleSpin_valueChanged(int);
    void on_propZScaleSpin_valueChanged(int);
    void on_propXRotSpin_valueChanged(int);
    void on_propYRotSpin_valueChanged(int);
    void on_propZRotSpin_valueChanged(int);

    void on_currentFrameSlider_valueChanged(int newValue);
    void on_playButton_clicked();
    void on_keyframeButton_toggled(bool on);
    void on_loopInSpinBox_valueChanged(int newValue);
    void on_loopOutSpinBox_valueChanged(int newValue);
    void on_framesSpin_valueChanged(int num);
    void on_fpsSpin_valueChanged(int num);
    // end autoconnection from designer UI

protected:
    // prevent closing of main window if there are unsaved changes
    virtual void closeEvent(QCloseEvent* event);
    //
    virtual void resizeEvent(QResizeEvent *event);

    void fileNew();
    void fileOpen();
    void fileOpen(const QString& fileName);
    // "add" a new file without clearing the old one(s)
    void fileAdd();
    void fileAdd(const QString& fileName);
//edu    void fileSave();
    void fileSaveAs();
    void fileExportForSecondLife();
    void fileLoadProps();
    void fileSaveProps();
    void fileExit();

    void editCut();
    void editCopy();
    void editPaste();

    void toolsOptimizeBVH();
    void toolsMirror();

    void showSkeleton(bool on);
    void setJointLimits(bool on);
    void setLoop(bool on);
    void setProtectFirstFrame(bool on);
    void showTimeline(bool state);

    void animationChanged(int which);
    void setAvatarShape(int shape);
    void setAvatarScale(int percent);
    void partChoice();
    void rotationValue();
    void rotationSlider(const QObject* slider);
    void positionValue();
    void positionSlider(const QObject* slider);

    void easeInChanged(int change);
    void easeOutChanged(int change);

    void newProp(Prop::PropType);
    void selectProp(const QString& name);
    void deleteProp();
    void attachProp(int attachmentPoint);
    void propPositionChanged();
    void propScaleChanged();
    void propRotationChanged();

    void frameSlider(int position);
    void nextPlaystate();
    void setLoopInPoint(int inFrame);
    void setLoopOutPoint(int outFrame);
    void numFramesChanged(int num);
    void setFPS(int fps);

    void setSliderValue(QSlider* slider,QLineEdit* edit,float value);

    QString selectFileToOpen(const QString& caption);
    void addToOpenFiles(const QString& fileName);
    void removeFromOpenFiles(unsigned int which);
    bool clearOpenFiles();

    void setPlaystate(PlayState state);

    bool checkFileOverwrite(const QFileInfo& fileInfo);
    void setCurrentFile(const QString& fileName);
    void enableInputs(bool state);

    void updateFps();
    void updateKeyBtn();
    void updateInputs();
    void updatePropSpins(const Prop* prop);

    // calculates the longest running time of all loaded animations, returns it
    // and stores it in longestRunningTime member variable
    double calculateLongestRunningTime();

    void setX(float x);
    void setY(float y);
    void setZ(float z);

    float getX();
    float getY();
    float getZ();

    void setXPos(float x);
    void setYPos(float y);
    void setZPos(float z);

    float getXPos();
    float getYPos();
    float getZPos();

//edu: went to ADT    QString CurrentFile;
    //edu so far
    virtual QString UntitledName() const {return "Untitled.avm";}

    QStringList openFiles;
    // last path used for open or save
    QString lastPath;
    QTimer timer;
    // list of animation ids mapped to combo box indexes
    QList<Animation*> animationIds;

    // mapping of combo box indexes to node ids
    QList<int> nodeMapping;

    BVHNode* currentPart;

    Timeline* timeline;
    // icons for play button
    QIcon playIcon;
    QIcon loopIcon;
    QIcon stopIcon;

    // holds the current playing status
    PlayState playstate;

    bool loop;
    bool jointLimits;
    bool frameDataValid;
    // if set the first frame of an animation is protected
    bool protectFirstFrame;
    // will be true if a frame is protected
    bool protect;

    // holds the longest running time of all currently opened animations
    double longestRunningTime;


//    void changeEvent(QEvent *e);

private:
    void bindMenuActions();
    void bindToolbarActions();
//    Ui::KeyFramerTab *ui;
};

#endif // KEYFRAMERTAB_H
