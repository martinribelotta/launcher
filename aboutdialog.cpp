#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include <QProcessEnvironment>

AboutDialog::AboutDialog(const QSize& z, QWidget *parent) :
                                            QDialog(parent),
                                            ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    ui->plainTextEdit->setWordWrapMode(QTextOption::NoWrap);
    ui->plainTextEdit->setFont(QFont{"Monospace, Consolas, Courier"});
    ui->plainTextEdit->setPlainText(QProcessEnvironment::systemEnvironment().toStringList().join("\n"));
    resize(z);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
