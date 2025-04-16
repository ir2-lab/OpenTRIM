#include "mainui.h"

#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setStyle(QStyleFactory::create("Fusion"));

    QFont f = app.font();
    f.setPointSize(11);
    app.setFont(f);

    MainUI w;
    w.show();
    return app.exec();
}
