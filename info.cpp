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

    QListWidgetItem* ageItem = new QListWidgetItem(QString::number(u.get_age()));
    ageItem->setTextAlignment(Qt::AlignCenter);
    ui->listWidget_4->addItem(ageItem);

    QListWidgetItem* genderItem;
    if (u.get_male()) genderItem = new QListWidgetItem("male");
    else genderItem = new QListWidgetItem("female");
    genderItem->setTextAlignment(Qt::AlignCenter);
    ui->listWidget_5->addItem(genderItem);
}

void Info::on_pushButton_clicked()
{
    close();
}

