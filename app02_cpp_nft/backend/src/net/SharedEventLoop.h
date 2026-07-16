#pragma once

#include <trantor/net/EventLoop.h>

// A single background event-loop thread shared by every outbound Drogon HttpClient instance
// (Pinata pinning, Ethereum JSON-RPC). Outbound calls in this codebase are made synchronously
// (blocking `sendRequest`) from Drogon's main request-handling loop; Drogon's HttpClient must run
// on a *different* loop than the caller or a synchronous call deadlocks forever, hence this
// dedicated loop instead of passing `nullptr` (which would reuse the app's own main loop).
namespace nftcerts::net {

trantor::EventLoop* sharedClientLoop();

}  // namespace nftcerts::net
