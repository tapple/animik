#ifndef NEWFILEDIALOG_H
#define NEWFILEDIALOG_H

#include <QDialog>
#include <QModelIndex>



namespace Ui {
    class NewFileDialog;
}

class NewFileDialog : public QDialog {
    Q_OBJECT
public:
    NewFileDialog(QWidget *parent = 0);
    ~NewFileDialog();

    enum ProjectType {AVM = 0, AVBL};     //TODO: this definitely must go somewhere else
    NewFileDialog::ProjectType SelectedProjectType() const {return selectedItem;}

protected slots:
  void on_listView_clicked(QModelIndex modelIndex);
  void on_listView_doubleClicked(QModelIndex modelIndex);

private:
    Ui::NewFileDialog *ui;
    NewFileDialog::ProjectType selectedItem;
};

#endif // NEWFILEDIALOG_H
