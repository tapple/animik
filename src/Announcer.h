#ifndef ANNOUNCER_H
#define ANNOUNCER_H

#include <QApplication>
#include <QMessageBox>
#include <QString>
#include <QToolTip>
#include <QWidget>


/** Helper static class for application messages (so far only with message box) and cursor handling */
class Announcer
{
public:
  static void Exception(QWidget* parent, QString message);

  /** Use it like: StartAction(this, "What's happening"); lenghty_action(); EndAction(); **/
  static bool StartAction(QWidget* parent, QString message);
  static void EndAction();


private:
  Announcer() { };
  ~Announcer() {};
  static QCursor _cursor;
  static QWidget* _parent;
};

#endif // ANNOUNCER_H
