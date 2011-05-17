#ifndef FRAMESWEIGHTDIALOG_H
#define FRAMESWEIGHTDIALOG_H

#include <QDialog>

class QPixmap;
class TrailItem;
class WeightedAnimation;


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
  virtual void paintEvent(QPaintEvent *);

private:
  Ui::FramesWeightDialog *ui;
  QPixmap* offscreen;

  WeightedAnimation* _anim;
  int frames_count;
  int* frameWeights;

  void drawWeights();

private slots:
  void on_setWeightPushButton_clicked();
};

#endif // FRAMESWEIGHTDIALOG_H
