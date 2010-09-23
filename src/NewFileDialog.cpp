#include <QStringListModel>
#include <QAbstractButton>
#include <QListData>
#include <QFileDialog>
#include <QRegExp>

#include "NewFileDialog.h"
#include "ui_NewFileDialog.h"
#include "settings.h"



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
  setOkButtonEnabled(modelIndex.isValid());

  if(modelIndex.isValid())
  {
    selectedItem = (ProjectType) modelIndex.row();
    //TODO: some hint to the left?

    ui->selectLocationWidget->setEnabled(true);
  }
  else
    ui->selectLocationWidget->setEnabled(false);
}


void NewFileDialog::on_listView_doubleClicked(QModelIndex modelIndex)
{
  if(modelIndex.isValid())
  {
    selectedItem = (ProjectType) modelIndex.row();
    accept();
  }
}

void NewFileDialog::on_pushButton_clicked()
{
  //// For some unknown reason passing "this" locks up the OSX qavimator window. Possibly a QT4 bug, needs investigation
  #ifdef __APPLE__
     QString path = QFileDialog::getExistingDirectory(NULL, "Select location", Settings::Instance()->lastPath());
  #else
     QString path = QFileDialog::getExistingDirectory(this, "Select location", Settings::Instance()->lastPath());
  #endif

  ui->pathLineEdit->setText(path);
}

void NewFileDialog::on_nameLineEdit_textChanged(QString )
{
  QString name = ui->nameLineEdit->text();
  QString path = ui->pathLineEdit->text();
  path += path.endsWith('/') ? "" : "/";

  QString* tooltip = new QString();

  if(!validFileName(path, name, tooltip))
  {
    ui->nameLineEdit->setBackgroundRole(QPalette::LinkVisited);
    ui->nameLineEdit->setToolTip(*tooltip);
    setOkButtonEnabled(false);
  }
}


void NewFileDialog::setOkButtonEnabled(bool enabled)
{
  ui->buttonBox->buttons().at(0)->setEnabled(enabled);
}


bool NewFileDialog::validFileName(QString location, QString name, QString* outMessage)
{
  if(name.isEmpty())
  {
    outMessage = new QString("Specify file name");
    return false;
  }
  if(QFile::exists(location + name))
  {
    outMessage = new QString("File with given name already exists");
    return false;
  }

  char array[9] = {'\\', '/', ':', '*', '?', '"', '<', '>', '|'};
  for(int i = 0; i < 9; i++)
    if(name.contains(array[i]))
    {
      outMessage = "Invalid character in file name: " + QString(array[i]);
      return false;
    }

  //'\', '/', ':', '*', '?', '"', '<', '>', '|'
  QRegExp re("\\\\/:\\*\\?\\\"<>\\|");          //TODO: needs heavy testing
  if(name.contains(re))
  {
    outMessage = new QString("Invalid character in file name");
    return false;
  }

  return true;
}
