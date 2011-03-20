#include "Announcer.h"


QCursor Announcer::_cursor = QCursor();
QWidget* Announcer::_parent = NULL;


void Announcer::Exception(QWidget* parent, QString message)
{
  QMessageBox::warning(parent, "Error", message);
}

bool Announcer::StartAction(QWidget* parent, QString message)
{
  if(_parent != NULL)     //some other widget already started an action
    return false;

  _parent = parent;
  _cursor = _parent->cursor();
  _parent->setCursor(Qt::WaitCursor);
  QRect geom = _parent->geometry();
  QToolTip::showText(QPoint(geom.width()/2, geom.height()/2), message, _parent, geom);
}

void Announcer::EndAction()
{
  _parent->setCursor(_cursor);
  _parent = NULL;
  QToolTip::hideText();
}
