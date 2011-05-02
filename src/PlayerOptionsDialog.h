#ifndef PLAYEROPTIONSDIALOG_H
#define PLAYEROPTIONSDIALOG_H

#include <QDialog>


namespace Ui {
  class PlayerOptionsDialog;
}


class PlayerOptionsDialog : public QDialog
{
  Q_OBJECT

public:
  explicit PlayerOptionsDialog(int fps, bool loop, QWidget *parent = 0);
  ~PlayerOptionsDialog();

  int fps() const     { return _fps; }
  bool loop() const   { return _loop; }

private:
  Ui::PlayerOptionsDialog *ui;
  int _fps;
  bool _loop;

private slots:
  void on_loopCheckBox_toggled(bool checked);
  void on_fpsSpinBox_valueChanged(int );
};

#endif // PLAYEROPTIONSDIALOG_H
