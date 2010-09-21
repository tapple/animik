#include <QAbstractItemModel>

#include "SaveChangesDialog.h"
#include "ui_SaveChangesDialog.h"

#include "AbstractDocumentTab.h"

//TODO: It's impossible without model

class UnsavedFilesModel : QAbstractItemModel
{
public:
  UnsavedFilesModel(){};
  ~UnsavedFilesModel(){};
};


SaveChangesDialog::SaveChangesDialog(QWidget *parent, QList<AbstractDocumentTab*> tabs) :
    QDialog(parent), ui(new Ui::SaveChangesDialog)
{
  unsavedTabs = tabs;
  ui->setupUi(this);

//  UnsavedFilesModel* model = new UnsavedFilesModel();
//  ui->listView->setModel(model);

  connect(ui->listView, SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(onSelectionChanged()));
}

SaveChangesDialog::~SaveChangesDialog()
{
  delete ui;
}

void SaveChangesDialog::onSelectionChanged()
{
//  ui->saveButton->setEnabled(ui->listView->selectedIndexes().count());
}

void SaveChangesDialog::on_saveButton_clicked()
{
  /*TDOO: make it at least buildable

  for(int i = 0; i< ui->listView->model()->rowCount(); i++)
  {
    tab = ui->listView->model()->itemData(i);

    static_cast<AbstractDocumentTab*>(tab)->Save();       //TODO: what if it's new file not saved yet?
                                                          //      The full path probably needs to be specified at creation.
                                                          //      Remake the NewFileDialog to 2 pages widget.
    ui->listView->model()->removeRow()
  }

  if(ui->listWidget->count() < 1)
    accept();           */
}

void SaveChangesDialog::on_dontSaveButton_clicked()
{

}
