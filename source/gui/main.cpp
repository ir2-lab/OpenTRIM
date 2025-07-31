#include "mainui.h"

#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // select fusion style if available
    // for consistent look among different platforms
    if (QStyleFactory::keys().contains("Fusion"))
        QApplication::setStyle(QStyleFactory::create("Fusion"));

    MainUI w;
    w.show();
    return app.exec();
}
