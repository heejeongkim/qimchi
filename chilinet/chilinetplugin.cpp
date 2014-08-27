#include "chilinetplugin.h"
#include "chiliclient.h"
#include "state.h"

#include <qqml.h>

void ChilinetPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<ChiliClient> (uri, 1, 0, "ChiliClient");
    qmlRegisterType<State> (uri, 1, 0, "State");
}
