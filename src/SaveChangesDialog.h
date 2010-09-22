#ifndef SAVECHANGESDIALOG_H
#define SAVECHANGESDIALOG_H

#include <QDialog>
#include <QList>
#include <QMap>

class QModelIndex;
class AbstractDocumentTab;



namespace Ui {
    class SaveChangesDialog;
}


class SaveChangesDialog : public QDialog {
    Q_OBJECT
public:
  SaveChangesDialog(QWidget *parent, QList<AbstractDocumentTab*> tabs);       //TODO: is the 'tabs' param necessary?
  ~SaveChangesDialog();

private:
  Ui::SaveChangesDialog *ui;

  //Key = full path of an animation file. Value = respective tab
  QMap<QString, AbstractDocumentTab*> map;

private slots:
  void onSelectionChanged();
  void on_dontSaveButton_clicked();
  void on_saveButton_clicked();
};

#endif // SAVECHANGESDIALOG_H
