#include <QAbstractListModel>
#include <QStringListModel>

#include "SaveChangesDialog.h"
#include "ui_SaveChangesDialog.h"

#include "AbstractDocumentTab.h"


SaveChangesDialog::SaveChangesDialog(QWidget *parent, QList<AbstractDocumentTab*> tabs) :
    QDialog(parent), ui(new Ui::SaveChangesDialog)
{
  QStringList list;

  foreach(AbstractDocumentTab* tab, tabs)
  {
    map.insert(tab->CurrentFile, tab);
    list.append(tab->CurrentFile);
  }

  ui->setupUi(this);
  setWindowTitle("Save modified documents");
  ui->listView->setModel(new QStringListModel(list));

  connect(ui->listView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(onSelectionChanged()));
}

SaveChangesDialog::~SaveChangesDialog()
{
  delete ui;
}


void SaveChangesDialog::onSelectionChanged()
{
  bool selected = (ui->listView->selectionModel()->selectedRows(0).count() > 0);
  ui->saveButton->setEnabled(selected);
}

void SaveChangesDialog::on_saveButton_clicked()
{
  QModelIndexList sel = ui->listView->selectionModel()->selectedRows(0);
  for(int i = 0; i< sel.count(); i++)
  {
    QString path = sel.at(i).data(Qt::DisplayRole).toString();
    map[path]->Save();      //TODO: what if it's new file not saved yet?
                            //      The full path probably needs to be specified at creation.
                            //      Remake the NewFileDialog to contain LineEdit & OpenFileDialog button
    ui->listView->model()->removeRows(sel.at(i).row(), 1);
  }

  if(ui->listView->model()->rowCount() < 1)
    accept();
}

void SaveChangesDialog::on_dontSaveButton_clicked()
{
  accept();     //lose unsaved changes
}
