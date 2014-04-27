// growl.h
// 2/1/2013 jichi

#include "growl.h"
#include "driver/rpccli.h"

void growl::msg(const QString &msg) { if (auto p = RpcClient::instance()) p->showMessage(msg); }
void growl::warn(const QString &msg) { if (auto p = RpcClient::instance()) p->showWarning(msg); }
void growl::error(const QString &msg) { if (auto p = RpcClient::instance()) p->showError(msg); }
void growl::notify(const QString &msg) { if (auto p = RpcClient::instance()) p->showNotification(msg); }

// EOF
