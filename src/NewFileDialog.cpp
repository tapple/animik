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

  ui->buttonBox->buttons().at(0)->setEnabled(false);    //OK button disabled

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

  if(modelIndex.isValid())
  {
    selectedItem = (ProjectType) modelIndex.row();
    //TODO: some hint to the left?
  }
}


void NewFileDialog::on_listView_doubleClicked(QModelIndex modelIndex)
{
  if(modelIndex.isValid())
  {
    selectedItem = (ProjectType) modelIndex.row();
    accept();
  }
}
