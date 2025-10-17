#include <gtest/gtest.h>
#include "jwtService.h"

class JwtServiceTest : public ::testing::Test {
protected:
    JwtService::Config config_;
    
    JwtServiceTest() {
        config_.secret = "unit-test-secret";
        config_.issuer = "TestIssuer";
        config_.audience = "TestAudience";
        config_.accessTokenLifetime = std::chrono::seconds(600);
    }

    JwtService service_{config_};
};

TEST_F(JwtServiceTest, GenerateAndValidateToken) {
    User user("test@example.com", "StrongPass1");
    user.setId("user-123");
    user.setActive(true);

    const auto token = service_.generateToken(user);
    ASSERT_FALSE(token.empty());

    auto claims = service_.validateToken(token);
    ASSERT_TRUE(claims.has_value());
    EXPECT_EQ(claims->subject, "user-123");
    EXPECT_EQ(claims->email, "test@example.com");
    EXPECT_EQ(claims->issuer, config_.issuer);
    EXPECT_EQ(claims->audience, config_.audience);
}

TEST_F(JwtServiceTest, InvalidTokenFailsValidation) {
    auto claims = service_.validateToken("invalid.token.value");
    EXPECT_FALSE(claims.has_value());
}

TEST_F(JwtServiceTest, TamperedTokenFailsValidation) {
    User user("tamper@example.com", "StrongPass1");
    user.setId("user-456");
    user.setActive(true);

    auto token = service_.generateToken(user);
    ASSERT_FALSE(token.empty());

    token.push_back('a'); // tamper token

    auto claims = service_.validateToken(token);
    EXPECT_FALSE(claims.has_value());
}
