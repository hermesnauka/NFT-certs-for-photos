#pragma once

#include <map>
#include <string>

// Flattened English/Polish message bundles for GET /api/i18n/{lang}, ported verbatim from app01's
// messages_en.properties / messages_pl.properties.
namespace nftcerts::i18n {

// Returns the message map for "en" or "pl", or nullptr if unsupported.
const std::map<std::string, std::string>* messagesForLanguage(const std::string& lang);

}  // namespace nftcerts::i18n
