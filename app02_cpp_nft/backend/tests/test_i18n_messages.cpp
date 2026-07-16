#include "i18n/Messages.h"

#include <gtest/gtest.h>

using namespace nftcerts::i18n;

TEST(MessagesTest, EnglishAndPolishBundlesExist) {
    ASSERT_NE(messagesForLanguage("en"), nullptr);
    ASSERT_NE(messagesForLanguage("pl"), nullptr);
    EXPECT_FALSE(messagesForLanguage("en")->empty());
    EXPECT_FALSE(messagesForLanguage("pl")->empty());
}

TEST(MessagesTest, UnsupportedLanguageReturnsNull) {
    EXPECT_EQ(messagesForLanguage("de"), nullptr);
    EXPECT_EQ(messagesForLanguage(""), nullptr);
    EXPECT_EQ(messagesForLanguage("EN"), nullptr);
}

TEST(MessagesTest, BothBundlesHaveIdenticalKeySets) {
    const auto* en = messagesForLanguage("en");
    const auto* pl = messagesForLanguage("pl");
    ASSERT_NE(en, nullptr);
    ASSERT_NE(pl, nullptr);

    ASSERT_EQ(en->size(), pl->size());
    for (const auto& [key, value] : *en) {
        EXPECT_TRUE(pl->count(key)) << "key missing from pl bundle: " << key;
        EXPECT_FALSE(value.empty()) << "empty en message for key: " << key;
    }
    for (const auto& [key, value] : *pl) {
        EXPECT_FALSE(value.empty()) << "empty pl message for key: " << key;
    }
}

TEST(MessagesTest, ContainsCertificateAuthenticityKey) {
    const auto* en = messagesForLanguage("en");
    ASSERT_NE(en, nullptr);
    EXPECT_TRUE(en->count("certificate.authenticity.body"));
}
