#ifndef NEWFILEDIALOG_H
#define NEWFILEDIALOG_H

#include <QDialog>
#include <QModelIndex>

#include "ui_newfiledialog.h"

class QPalette;



/*namespace Ui {
    class NewFileDialog;
}*/

class NewFileDialog : public QDialog {
    Q_OBJECT
public:
  NewFileDialog(QWidget *parent = 0);
  ~NewFileDialog();

  //Constants
  static const QString UntitledName;

  enum ProjectType {AVM = 0, AVBL};     //TODO: this definitely must go somewhere else
  NewFileDialog::ProjectType SelectedProjectType() const {return selectedItem;}
  QString FileName() const {return fileName;}

protected slots:
  void on_listView_clicked(QModelIndex modelIndex);
  void on_listView_doubleClicked(QModelIndex modelIndex);

private:
  Ui::NewFileDialog *ui;
  NewFileDialog::ProjectType selectedItem;

  QString fileName;
  QPalette originalPalette;
  void setOkButtonEnabled(bool enabled);
  void suggestFilename(ProjectType type);
  static bool validFileName(QString location, QString name, QString* outMessage);
  static bool validDirectory(QString location, QString* outMessage);

private slots:
  virtual void accept();
  void on_pathLineEdit_textChanged(QString );
  void on_nameLineEdit_textChanged(QString );
  void on_pushButton_clicked();
};

#endif // NEWFILEDIALOG_H
