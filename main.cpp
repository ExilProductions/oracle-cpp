#include <QApplication>
#include "AURManager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Oracle");
    app.setApplicationDisplayName("Oracle – AUR Helper Wrapper");

    AURManager window;
    window.show();
    return app.exec();
}