#ifndef MIXZONESDIALOG_H
#define MIXZONESDIALOG_H

#include <QDialog>

namespace Ui {
  class MixZonesDialog;
}



class MixZonesDialog : public QDialog
{
  Q_OBJECT

public:
  MixZonesDialog(int frames, int mixIn, int mixOut, QWidget *parent = 0);
  ~MixZonesDialog();

  int mixIn() { return _mixIn; }
  int mixOut() { return _mixOut; }

private:
  Ui::MixZonesDialog *ui;

  int framesCount;
  int _mixIn;
  int _mixOut;

private slots:
  void on_mixOutSpinBox_valueChanged(int value);
  void on_mixInSpinBox_valueChanged(int value);
  void on_mixOutSlider_valueChanged(int value);
  void on_mixInSlider_valueChanged(int value);
};

#endif // MIXZONESDIALOG_H
