#include "menu.h"
#include "home.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Menu w;
    home m;
    QObject::connect(m.GetUi()->exit, &QPushButton::clicked, &m, &QWidget::close);

    m.setWindowFlags(Qt::FramelessWindowHint);
    m.setAttribute(Qt::WA_TranslucentBackground);

    m.show();
    // w.show();
    return a.exec();
}
