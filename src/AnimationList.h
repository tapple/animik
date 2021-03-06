#ifndef ANIMATIONLIST_H
#define ANIMATIONLIST_H

#include <QWidget>
#include <QModelIndex>

class QKeyEvent;
class QStringList;


namespace Ui {
    class AnimationList;
}

class AnimationList : public QWidget {
Q_OBJECT

public:
  AnimationList(QWidget *parent = 0);
  ~AnimationList();

signals:
  void AnimationFileTaken(QString filename, int orderInBatch, int batchSize);

public slots:
  void addNewFile();

protected:
  QStringList availableAnimations;

  void keyPressEvent(QKeyEvent* event);
  void setButtonsEnabled(bool enabled);

protected slots:
  void onSelectionChanged();

private:
  Ui::AnimationList *ui;

  void removeSelectedItems();

private slots:
  void on_availableAnimsListView_doubleClicked(QModelIndex index);
  void on_removeButton_clicked();
  void on_addButton_clicked();
};

#endif // ANIMATIONLIST_H
