#include "info.h"
#include "ui_info.h"

Info::Info(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Info)
{
    ui->setupUi(this);
}

Info::~Info()
{
    delete ui;
}

void Info::set_info(User& u)
{
    QListWidgetItem* nameItem = new QListWidgetItem(u.get_name());
    nameItem->setTextAlignment(Qt::AlignCenter);
    ui->listWidget->addItem(nameItem);

    QListWidgetItem* userNameItem = new QListWidgetItem(u.get_user_name());
    userNameItem->setTextAlignment(Qt::AlignCenter);
    ui->listWidget_2->addItem(userNameItem);

    QListWidgetItem* emailItem = new QListWidgetItem(u.get_email());
    emailItem->setTextAlignment(Qt::AlignCenter);
    ui->listWidget_3->addItem(emailItem);
}

void Info::on_pushButton_clicked()
{
    close();
}

