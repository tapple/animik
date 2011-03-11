#ifndef LIMBSWEIGHTDIALOG_H
#define LIMBSWEIGHTDIALOG_H



#include <QDialog>
#include <QHash>
#include <QList>


class BVHNode;


namespace Ui {
  class LimbsWeightDialog;
}


/** A dialog to set weights for given set of BVHNodes. On accepting it effectively performs
    the weighting, not sooner. **/
class LimbsWeightDialog : public QDialog
{
    Q_OBJECT

  public:
    /** @param limbs list of BVHNodes to be weighted
        @param frame index of key-frame for which the limbs are being weighted inside
               their respective animation */
    LimbsWeightDialog(QString animationName, QList<BVHNode*>* limbs, int frame, int totalFrames, QWidget *parent = 0);
    ~LimbsWeightDialog();


  public slots:
    virtual void accept();      //overriden from QDialog

  private:
    Ui::LimbsWeightDialog *ui;
    QList<BVHNode*>* limbs;
    int currentFrame;
    int totalFrames;
    /** Hash table of summary weights (value) in given frames (key) **/
    QHash<int, int> _weights;

    void initComponents();

  private slots:
    void on_prevFrameButton_clicked();
    void on_nextFrameButton_clicked();
    void on_weightSlider_valueChanged(int value);
};

#endif // LIMBSWEIGHTDIALOG_H
