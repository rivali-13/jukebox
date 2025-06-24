#include "video.h"
#include "ui_video.h"

Video::Video(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Video)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->showFullScreen();
}

Video::~Video()
{
    delete ui;
}

void Video::on_exit_clicked()
{
    close();
}


void Video::on_full_clicked()
{
    if (!this->isFullScreen())
    {
        ui->main->setStyleSheet("#main { background-color: #201f1d;	border-radius: 0px;    border: none }");
        ui->bottom->setStyleSheet("#bottom {background-color: #232220;   border-bottom-left-radius: 0px;  border-bottom-right-radius: 0px }}");
        this->showFullScreen();
    }
    else
    {
        this->showNormal();
        ui->main->setStyleSheet("#main {  background-color: #201f1d;	border-radius: 20px;    border: 1px solid #0632a2; }");
        ui->bottom->setStyleSheet("#bottom {background-color: #232220;   border-bottom-left-radius: 20px;  border-bottom-right-radius: 20px }}");
    }
}

