#include "menu.h"
#include "ui_menu.h"
#include "login.h"
#include "sign_up.h"
#include "forget.h"


Menu::Menu(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Menu)
{
    ui->setupUi(this);

    login = new Login(this);
    sign_up = new Sign_up(this);

    animation = new QPropertyAnimation(ui->coverPage, "pos");
    animation->setDuration(250);
    ui->coverPage->raise();

    startPos = ui->coverPage->pos();
    endPos = QPoint(350, startPos.y());

    setInfoCover();

    // Enter Login Page
    connect(ui->logUsername, &QLineEdit::returnPressed, this, [=]() {
        ui->logPassword->setFocus();
    });
    connect(ui->logPassword, &QLineEdit::returnPressed, this, [=]() {
        ui->login->animateClick();
    });

    // Enter SignUp Page
    connect(ui->upUsername, &QLineEdit::returnPressed, this, [=]() {
        ui->upName->setFocus();
    });
    connect(ui->upName, &QLineEdit::returnPressed, this, [=]() {
        ui->upEmail->setFocus();
    });
    connect(ui->upEmail, &QLineEdit::returnPressed, this, [=]() {
        ui->upPassword->setFocus();
    });
    connect(ui->upPassword, &QLineEdit::returnPressed, this, [=]() {
        ui->signUp->animateClick();
    });


}

Menu::~Menu()
{
    delete ui;
}

void Menu::on_buttonCover_clicked()
{
    if (isLoginMode){
        animation->setStartValue(startPos);
        animation->setEndValue(endPos);
        isLoginMode = false;
        animation->start();
        cleanLabels();
        setInfoCover();
    }
    else{
        animation->setStartValue(endPos);
        animation->setEndValue(startPos);
        isLoginMode = true;
        animation->start();
        cleanLabels();
        setInfoCover();
    }

}

void Menu::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    QPixmap background(":/JukeBox/Icon/background.jpg");
    painter.drawPixmap(this->rect(), background);
}

void Menu::setInfoCover(){
    if (isLoginMode){
        ui->titleCover->setText("سلام، دوست من!");
        ui->infoCover->setText("برای استفاده از تمام ویژگی های نرم افزار ثبت نام کنید.");
        ui->buttonCover->setText("ایجاد حساب");
        ui->logUsername->setFocus();
    }
    else{
        ui->titleCover->setText("خوش آمدید!");
        ui->infoCover->setText("اگر حساب کاربری دارید با استفاده از دکمه زیر وارد حساب کاربری خود شوید.");;
        ui->buttonCover->setText("ورود");
        ui->upUsername->setFocus();
    }
}

void Menu::cleanLabels()
{
    if(!isLoginMode){
        ui->upUsername->setText("");
        ui->upName->setText("");
        ui->upPassword->setText("");
        ui->upEmail->setText("");
    }
    else{
        ui->logUsername->setText("");
        ui->logPassword->setText("");
    }
}


void Menu::on_login_clicked()
{
    login->doLogin();
    ui->main->setFocus();
}


void Menu::on_signUp_clicked()
{
    sign_up->create_account();
    ui->main->setFocus();
}


void Menu::on_buttonForget_clicked()
{
    Forget* f = new Forget (this);
    f->show();
}

