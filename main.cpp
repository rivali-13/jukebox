#include "menu.h"
#include "home.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Menu w;
    home m;
    m.show();
    //w.show();
    return a.exec();
}
