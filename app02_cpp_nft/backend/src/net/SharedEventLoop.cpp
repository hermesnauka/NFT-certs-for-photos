#include "net/SharedEventLoop.h"

#include <trantor/net/EventLoopThread.h>

namespace nftcerts::net {

trantor::EventLoop* sharedClientLoop() {
    static trantor::EventLoopThread thread;
    static bool started = [] {
        thread.run();
        return true;
    }();
    (void)started;
    return thread.getLoop();
}

}  // namespace nftcerts::net
