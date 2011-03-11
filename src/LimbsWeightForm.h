#ifndef LIMBSWEIGHTFORM_H
#define LIMBSWEIGHTFORM_H

#include <QWidget>
#include <QHash>

class BVHNode;
class QLabel;
class WeightedAnimation;


namespace Ui {
  class LimbsWeightForm;
}


class LimbsWeightForm : public QWidget
{
  Q_OBJECT

  public:
    LimbsWeightForm(QWidget *parent = 0);
    ~LimbsWeightForm();

  public slots:
    void UpdateContent(WeightedAnimation* animation, int frame);

  private:
    Ui::LimbsWeightForm *ui;
    WeightedAnimation* anim;
    int frame;
    BVHNode* selectedLimb;
    QHash<QString, QLabel*> labels;

    void updateLabelsHelper(BVHNode* limb);

  private slots:
    void on_weightSlider_valueChanged(int value);
    void on_limbComboBox_currentIndexChanged(QString itemText);
    void on_hideButton_clicked();
};

#endif // LIMBSWEIGHTFORM_H
