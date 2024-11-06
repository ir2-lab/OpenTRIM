#include "ionsui.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    IonsUI w;
    w.show();
    return app.exec();
}
