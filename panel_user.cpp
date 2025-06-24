#include "panel_user.h"
#include "ui_panel_user.h"


Panel_User::Panel_User(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Panel_User)
    ,currentUser(nullptr)
{
    ui->setupUi(this);
}

Panel_User::~Panel_User()
{
    delete ui;
}

void Panel_User::set_curuser(User& _user)
{
    currentUser.set_user_name(_user.get_user_name());
    currentUser.set_name(_user.get_name());
    currentUser.set_password(_user.get_password());
    currentUser.set_email(_user.get_email());
}

void Panel_User::on_pushButton_2_clicked()
{
    close();
}


void Panel_User::on_pushButton_clicked()
{
    Info * i = new Info();
    i->set_info(currentUser);
    i->show();
}

void Panel_User::on_pushButton_3_clicked() //***
{
    home * pagemusic = home::single();
    this->close();
    pagemusic->show();
}
