#include "mainui.h"

#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setStyle(QStyleFactory::create("Fusion"));

    MainUI w;
    w.show();
    return app.exec();
}
