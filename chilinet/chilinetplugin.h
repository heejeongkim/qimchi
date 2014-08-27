#ifndef CHILINETPLUGIN_H
#define CHILINETPLUGIN_H

#include <QQmlExtensionPlugin>

class ChilinetPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri);
};

#endif // CHILINETPLUGIN_H
