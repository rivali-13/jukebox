#include "menu.h"
#include "./ui_menu.h"
#include "sign_in.h"
#include "login.h"

Menu::Menu(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Menu)
{
    ui->setupUi(this);
}

Menu::~Menu()
{
    delete ui;
}

void Menu::on_pushButton_2_clicked()
{
    Sign_in *t = new Sign_in(this);
    t->show();
}


void Menu::on_pushButton_clicked()
{
    Login * l = new Login(this);
    l->show();
}

