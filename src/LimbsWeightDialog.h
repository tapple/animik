#ifndef LIMBSWEIGHTDIALOG_H
#define LIMBSWEIGHTDIALOG_H



#include <QDialog>
#include <QList>


class BVHNode;


namespace Ui {
    class LimbsWeightDialog;
}

class LimbsWeightDialog : public QDialog
{
    Q_OBJECT

  public:
    /** @param weightHint a limb weight to be preset. Only values in <0, 100> will be accepted */
    LimbsWeightDialog(QList<BVHNode*>* limbs, int weightHint = -1, QWidget *parent = 0);
    ~LimbsWeightDialog();

    int weight() { return _weight; }

  private:
    Ui::LimbsWeightDialog *ui;
    int _weight;

  private slots:
    void on_weightSlider_valueChanged(int value);
};

#endif // LIMBSWEIGHTDIALOG_H
