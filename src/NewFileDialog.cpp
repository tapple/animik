#include "NewFileDialog.h"
#include "ui_NewFileDialog.h"

#include <QStringListModel>
#include <QAbstractButton>
#include <QListData>



NewFileDialog::NewFileDialog(QWidget *parent)
  : QDialog(parent), ui(new Ui::NewFileDialog)
{
  ui->setupUi(this);

  QStringList list;
  list << "AVM" << "AVBL (TODO)";
  ui->listView->setModel(new QStringListModel(list));
/*  QList<NewFileDialog::ProjectType> list;
  list << NewFileDialog::AVM << NewFileDialog::AVBL;
  ui->listView->setModel(new QList<NewFileDialog::ProjectType>);  */

  ui->buttonBox->buttons().at(0)->setEnabled(false);    //OK button

  connect(ui->buttonBox->buttons().at(0), SIGNAL(clicked()), this, SLOT(accept()));
  connect(ui->buttonBox->buttons().at(1), SIGNAL(clicked()), this, SLOT(reject()));

}

NewFileDialog::~NewFileDialog()
{
    delete ui;
}


void NewFileDialog::on_listView_clicked(QModelIndex modelIndex)
{
  ui->buttonBox->buttons().at(0)->setEnabled(modelIndex.isValid());
  //TODO... Some hint to the left?
}


void NewFileDialog::on_listView_doubleClicked(QModelIndex modelIndex)
{
  if(modelIndex.isValid())
  {
    selectedItem = ???;     //TODO
    accept();
  }
}
