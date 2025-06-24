#include "new_queue.h"
#include "ui_new_queue.h"

New_queue::New_queue(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::New_queue)
{
    ui->setupUi(this);
    ui->lineEdit->setFocus();
}

New_queue::~New_queue()
{
    delete ui;
}

void New_queue::on_buttonBox_accepted()
{
    QString name = ui->lineEdit->text();
    if (name.isEmpty()) return;
    emit new_name(name);
}

