#ifndef LIMBSWEIGHTFORM_H
#define LIMBSWEIGHTFORM_H

#include <QWidget>

namespace Ui {
    class LimbsWeightForm;
}

class LimbsWeightForm : public QWidget {
    Q_OBJECT
  public:
    LimbsWeightForm(QWidget *parent = 0);
    ~LimbsWeightForm();

  signals:
    void hidden();

  protected:
    void changeEvent(QEvent *e);

  private:
    Ui::LimbsWeightForm *ui;

  private slots:
    void on_hideButton_clicked();
};

#endif // LIMBSWEIGHTFORM_H
