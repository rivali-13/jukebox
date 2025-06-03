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

QString Info::twoWayDecrypt(const QString& input) {
    return QString(QByteArray::fromBase64(input.toUtf8()));
}

void Info::set_info(User& u)
{
    QListWidgetItem* nameItem = new QListWidgetItem(twoWayDecrypt(u.get_name()));
    nameItem->setTextAlignment(Qt::AlignCenter);
    ui->listWidget->addItem(nameItem);

    QListWidgetItem* userNameItem = new QListWidgetItem(twoWayDecrypt(u.get_user_name()));
    userNameItem->setTextAlignment(Qt::AlignCenter);
    ui->listWidget_2->addItem(userNameItem);

    QListWidgetItem* emailItem = new QListWidgetItem(twoWayDecrypt(u.get_email()));
    emailItem->setTextAlignment(Qt::AlignCenter);
    ui->listWidget_3->addItem(emailItem);
}

void Info::on_pushButton_clicked()
{
    close();
}

