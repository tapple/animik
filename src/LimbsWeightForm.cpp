#include "LimbsWeightForm.h"
#include "ui_LimbsWeightForm.h"



LimbsWeightForm::LimbsWeightForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LimbsWeightForm)
{
    ui->setupUi(this);
}

LimbsWeightForm::~LimbsWeightForm()
{
    delete ui;
}


void LimbsWeightForm::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void LimbsWeightForm::on_hideButton_clicked()
{
  hide();
  emit hidden();
}
