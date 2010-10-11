#include <QStringListModel>
#include <QFileDialog>

#include <QMessageBox>        //DEBUG

#include "AnimationList.h"
#include "ui_AnimationList.h"
#include "settings.h"

#define ANIM_FILTER "Animation Files (*.avm *.bvh)"         //TODO: this is repeated on few places. Solve it.


AnimationList::AnimationList(QWidget *parent) :
    QWidget(parent), ui(new Ui::AnimationList)
{
    ui->setupUi(this);

    QStringListModel* model = new QStringListModel(this);
    model->setStringList(availableAnimations);
    ui->availableAnimsListView->setModel(model);

    connect(ui->availableAnimsListView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
            this, SLOT(onSelectionChanged()));
}

AnimationList::~AnimationList()
{
  delete ui;
}


void AnimationList::addNewFile()
{
  //// For some unknown reason passing "this" locks up the OSX qavimator window. Possibly a QT4 bug, needs investigation
#ifdef __APPLE__
  QString file=QFileDialog::getOpenFileName(NULL, "Choose an animation file", Settings::Instance()->lastPath(), ANIM_FILTER);
#else
  QString file=QFileDialog::getOpenFileName(this, "Choose an animation file", Settings::Instance()->lastPath(), ANIM_FILTER);
#endif

  if(file.isEmpty() || availableAnimations.contains(file))
    return;
  if(file.endsWith(".avm", Qt::CaseInsensitive) || file.endsWith(".bvh", Qt::CaseInsensitive))
  {
    availableAnimations.append(file);
    QStringListModel* model =  dynamic_cast<QStringListModel*>(ui->availableAnimsListView->model());
    model->setStringList(availableAnimations);                //TODO: this is sooo lame. Find a dignified way
  }
}

//--------------------- UI slots ---------------------//

void AnimationList::onSelectionChanged()
{
  bool selected = (ui->availableAnimsListView->selectionModel()->selectedRows(0).count() > 0);
  setButtonsEnabled(selected);
}

void AnimationList::setButtonsEnabled(bool enabled)
{
  ui->addButton->setEnabled(enabled);
  ui->removeButton->setEnabled(enabled);
}


void AnimationList::on_availableAnimsListView_doubleClicked(QModelIndex)
{
  on_addButton_clicked();
}

void AnimationList::on_addButton_clicked()
{
  QItemSelectionModel* selModel = ui->availableAnimsListView->selectionModel();
  int index = selModel->selectedIndexes().at(0).row();
//  QString filename = "" + availableAnimations.at(index);
//  QMessageBox::warning(this, "DEBUG", filename);

//  qDebug("Animation file offered: %s", filename);                 //SIGILL fun begins here

  emit AnimationFileTaken(availableAnimations.at(index));
}

void AnimationList::on_removeButton_clicked()
{
  QStringListModel* model =  dynamic_cast<QStringListModel*>(ui->availableAnimsListView->model());
  QItemSelectionModel* selModel = ui->availableAnimsListView->selectionModel();
  int index = selModel->selectedIndexes().at(0).row();
  availableAnimations.removeAt(index);
  model->setStringList(availableAnimations);                //TODO: bleh!

  onSelectionChanged();         //must be called explicitely
}
