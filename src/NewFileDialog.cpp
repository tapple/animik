#include <QStringListModel>
#include <QAbstractButton>
#include <QListData>
#include <QFileDialog>

#include "newfiledialog.h"
#include "ui_newfiledialog.h"
#include "settings.h"



const QString NewFileDialog::UntitledName = "Untitled";


NewFileDialog::NewFileDialog(QWidget *parent)
  : QDialog(parent), ui(new Ui::NewFileDialog)
{
  ui->setupUi(this);
  originalPalette = ui->nameLineEdit->palette();
  ui->pathLineEdit->setText(Settings::Instance()->lastPath());

  QStringList list;
  list << "AVM" << "AVBL";
  ui->listView->setModel(new QStringListModel(list));

  ui->buttonBox->buttons().at(0)->setEnabled(false);    //OK button disabled

  connect(ui->buttonBox->buttons().at(0), SIGNAL(clicked()), this, SLOT(accept()));
  connect(ui->buttonBox->buttons().at(1), SIGNAL(clicked()), this, SLOT(reject()));
}

NewFileDialog::~NewFileDialog()
{
  delete ui;
}

void NewFileDialog::accept()
{
  Settings::Instance()->setLastPath(ui->pathLineEdit->text());
  fileName = ui->pathLineEdit->text();
  fileName += fileName.endsWith('/') ? "" : "/";
  fileName += ui->nameLineEdit->text();
  QDialog::accept();
}

void NewFileDialog::on_listView_clicked(QModelIndex modelIndex)
{
  setOkButtonEnabled(modelIndex.isValid());

  if(modelIndex.isValid())
  {
    selectedItem = (ProjectType) modelIndex.row();
    //TODO: some hint to the left?
    suggestFilename(selectedItem);

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
    QPalette pal = originalPalette;
    pal.setColor(QPalette::Base, QColor(245, 169, 169));    //light red background
    ui->nameLineEdit->setPalette(pal);
    ui->nameLineEdit->setToolTip(*tooltip);
    setOkButtonEnabled(false);
  }
  else
  {
    ui->nameLineEdit->setPalette(originalPalette);
    ui->nameLineEdit->setToolTip(QString::null);
    setOkButtonEnabled(true);
  }
}

void NewFileDialog::on_pathLineEdit_textChanged(QString )
{
  QString path = ui->pathLineEdit->text();
  QString* tooltip = new QString();

  if(!validDirectory(path, tooltip))
  {
    QPalette pal = originalPalette;
    pal.setColor(QPalette::Base, QColor(245, 169, 169));    //light red background
    ui->pathLineEdit->setPalette(pal);
    ui->pathLineEdit->setToolTip(*tooltip);
    setOkButtonEnabled(false);
  }
  else
  {
    ui->pathLineEdit->setPalette(originalPalette);
    ui->pathLineEdit->setToolTip(QString::null);
    setOkButtonEnabled(true);
  }
}

void NewFileDialog::setOkButtonEnabled(bool enabled)
{
  ui->buttonBox->buttons().at(0)->setEnabled(enabled);
}

void NewFileDialog::suggestFilename(ProjectType type)
{
  int num = 1;
  QString name;
  QString extension = "";
  QString dir = ui->pathLineEdit->text();
  dir += dir.endsWith('/') ? "" : "/";            //TODO: platform dependent?

  switch(type)
  {
    case NewFileDialog::AVM : extension = ".avm";
      break;
    case NewFileDialog::AVBL : extension = ".avbl";
      break;
  }

  name = UntitledName + extension;
  while(QFile::exists(dir + name))
    name = UntitledName + QString("%1").arg(++num) + extension;

  ui->nameLineEdit->setText(name);
}


/* =============================== input validations =============================== */

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

#ifdef Q_OS_WIN32
  const int size = 9;
  char array[size] = {'\\', '/', ':', '*', '?', '"', '<', '>', '|'};
#else
  const int size = 2;
  char array[] = {'\\', '/'};
#endif
  for(int i = 0; i < size; i++)
    if(name.contains(array[i]))
    {
      QString temp = "Invalid character in file name: " + QString(array[i]);
      *outMessage = temp;
      return false;
    }

  return true;
}

bool NewFileDialog::validDirectory(QString location, QString* outMessage)
{
  if(location.isEmpty())
  {
    outMessage = new QString("Specify existing directory");
    return false;
  }
  if(!(new QDir(location))->exists())
  {
    outMessage = new QString("Invalid location");
    return false;
  }
  return true;
}
