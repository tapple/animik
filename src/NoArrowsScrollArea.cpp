#include <QScrollBar>
#include <QWheelEvent>
#include "noarrowsscrollarea.h"

NoArrowsScrollArea::NoArrowsScrollArea(QWidget* parent) : QScrollArea(parent)
{
}


void NoArrowsScrollArea::wheelEvent(QWheelEvent* event)
{
  int numSteps = event->delta() / 6;

  horizontalScrollBar()->setValue(horizontalScrollBar()->value() + numSteps);
  event->accept();
}


void NoArrowsScrollArea::keyPressEvent(QKeyEvent *e)
{
  e->ignore();          //no keys handled by this scroll area, let all to a parent
}
