#include <QApplication>
#include <QCommandLineParser>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Demo05GaussianSplatViewer");
    app.setApplicationVersion("0.5.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("LowAir-GS Demo-05: Gaussian Splat Viewer");
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption gaussianOpt({"g", "gaussian"}, "Path to Gaussian PLY file", "file");
    parser.addOption(gaussianOpt);
    parser.process(app);

    MainWindow w;
    w.show();

    if (parser.isSet(gaussianOpt)) {
        w.loadFromCommandLine(parser.value(gaussianOpt));
    }

    return app.exec();
}
