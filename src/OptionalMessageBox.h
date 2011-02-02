#ifndef OPTIONALMESSAGEBOX_H
#define OPTIONALMESSAGEBOX_H

#include <QDialog>


namespace Ui {
    class OptionalMessageBox;
}




class OptionalMessageBox : public QDialog
{
    Q_OBJECT
public:
    OptionalMessageBox(QString title, QString text, QWidget* parent = 0);
    ~OptionalMessageBox();

    bool showAgain() { return _showAgain; }

private:
    Ui::OptionalMessageBox *ui;
    bool _showAgain;

private slots:
    void on_showAgainCheckBox_toggled(bool checked);
};

#endif // OPTIONALMESSAGEBOX_H
