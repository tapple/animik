#ifndef SAVECHANGESDIALOG_H
#define SAVECHANGESDIALOG_H

#include <QDialog>
#include <QList>

class AbstractDocumentTab;



namespace Ui {
    class SaveChangesDialog;
}


class SaveChangesDialog : public QDialog {
    Q_OBJECT
public:
    SaveChangesDialog(QWidget *parent, QList<AbstractDocumentTab*> tabs);
    ~SaveChangesDialog();

private:
    Ui::SaveChangesDialog *ui;
    QList<AbstractDocumentTab*> unsavedTabs;

private slots:
    void onSelectionChanged();
    void on_dontSaveButton_clicked();
    void on_saveButton_clicked();
};

#endif // SAVECHANGESDIALOG_H
