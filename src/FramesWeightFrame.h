#ifndef FRAMESWEIGHTFRAME_H
#define FRAMESWEIGHTFRAME_H

#include <QFrame>

class QPixmap;
class TrailItem;
class WeightedAnimation;


namespace Ui {
  class FramesWeightFrame;
}


class FramesWeightFrame : public QFrame
{
  Q_OBJECT

public:
  explicit FramesWeightFrame(TrailItem* animItem, QWidget *parent = 0);
  ~FramesWeightFrame();

  static const int MinTrailFrames = 2;
  static const int FrameBarWidth = 10;
  static const int TrackHeight = 100;
  static const int ScrollBarOffset = 22;
  static const int TextSize = 16;

  int frameCount() const;
  void saveWeightsToAnimation();
  bool isDirty() const    { return dirty; }
  const int* getFrameWeights() const;
  void setCommonWeight(int value);

protected:
  virtual void mouseMoveEvent(QMouseEvent* mouseEvent);
  virtual void mousePressEvent(QMouseEvent* mouseEvent);
  virtual void mouseReleaseEvent(QMouseEvent *);
  virtual void paintEvent(QPaintEvent *);
  virtual void resizeEvent(QResizeEvent *);

private:
  Ui::FramesWeightFrame *ui;
  QPixmap* offscreen;

  WeightedAnimation* _anim;
  int frames_count;
  int* frameWeights;
  int weightedFrame;
  bool dirty;
  bool mouseDown;

  void drawWeights();
  void adjustWeight(int y);
};

#endif // FRAMESWEIGHTFRAME_H
