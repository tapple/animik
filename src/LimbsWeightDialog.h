#ifndef LIMBSWEIGHTDIALOG_H
#define LIMBSWEIGHTDIALOG_H



#include <QDialog>
#include <QList>


class BVHNode;


namespace Ui {
  class LimbsWeightDialog;
}


/** A dialog to set weights for given set of BVHNodes. On accepting it effectively performs the weighting,
    so the weight() property is only for informative purposes. **/
class LimbsWeightDialog : public QDialog
{
    Q_OBJECT

  public:
    /** @param limbs list of BVHNodes to be weighted
        @param frame index of key-frame for which the limbs are being weighted inside
               their respective animation */
    LimbsWeightDialog(QString animationName, QList<BVHNode*>* limbs, int frame, QWidget *parent = 0);
    ~LimbsWeightDialog();

    int weight() const { return _weight; }

  public slots:
    virtual void accept();      //overriden from QDialog

  private:
    Ui::LimbsWeightDialog *ui;
    QList<BVHNode*>* limbs;
    int frame;
    int _weight;

  private slots:
    void on_weightSlider_valueChanged(int value);
};

#endif // LIMBSWEIGHTDIALOG_H
