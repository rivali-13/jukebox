#include "menu.h"
#include <QApplication>
#include "video.h"
#include "home.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Menu w;
    // home* m = home::single();

    w.show();
    //Video v;
    //v.show();





    // w.show();
    return a.exec();
}
