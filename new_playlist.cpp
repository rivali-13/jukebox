#include "new_playlist.h"
#include "ui_new_playlist.h"

New_playlist::New_playlist(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::New_playlist)
{
    ui->setupUi(this);
}

New_playlist::~New_playlist()
{
    delete ui;
}

QString New_playlist::get_name()
{
    return name;
}

void New_playlist::on_buttonBox_accepted()
{
    QString name = ui->lineEdit->text();
    if(name.isEmpty()) return;
    emit new_name(name);
}


