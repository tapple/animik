#ifndef NOARROWSSCROLLAREA_H
#define NOARROWSSCROLLAREA_H

#include <QScrollArea>


/** Reimplementation of scroll area that doesn't scroll on arrows key press.
    It's default scroll behaviour is to scroll horizontally */
class NoArrowsScrollArea : public QScrollArea
{
public:
  NoArrowsScrollArea(QWidget* parent);


protected:
  virtual void wheelEvent(QWheelEvent *event);
  virtual void keyPressEvent(QKeyEvent *e);
};

#endif // NOARROWSSCROLLAREA_H
