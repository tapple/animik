#ifndef FRAMESWEIGHTDIALOG_H
#define FRAMESWEIGHTDIALOG_H

#include <QDialog>

class FramesWeightFrame;
class NoArrowsScrollArea;
class QKeyEvent;
class TrailItem;

namespace Ui {
  class FramesWeightDialog;
}

class FramesWeightDialog : public QDialog
{
  Q_OBJECT

public:
  explicit FramesWeightDialog(TrailItem* animItem, QWidget *parent = 0);
  ~FramesWeightDialog();

protected:
  void keyPressEvent(QKeyEvent* keyEvent);
  void paintEvent(QPaintEvent*);
  void resizeEvent(QResizeEvent*);

private:
  Ui::FramesWeightDialog *ui;
  FramesWeightFrame* fwf;
  QWidget* stackWidget;
  NoArrowsScrollArea* scrollArea;
  bool needsReshape;

private slots:
  void on_buttonBox_accepted();
  void on_setWeightPushButton_clicked();
};

#endif // FRAMESWEIGHTDIALOG_H
