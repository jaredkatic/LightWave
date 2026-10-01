#include <gtest/gtest.h>
#include "CircuitBreaker.hpp"

int mockSuccessProbe(const char *host, const char *port) { return 0; };
int mockFailureProbe(const char *host, const char *port) { return -1; };

TEST(CircuitBreakerTest, StartsInClosedState)
{
  CircuitBreaker breaker("test.com", "80", 3, 5);
  EXPECT_EQ(breaker.getState(), CircuitState::CLOSED);
}

TEST(CircuitBreakerTest, TripsToOpenAfterMaxFailures)
{
  CircuitBreaker breaker("test.com", "80", 3, 5);

  // Fail 1 & 2
  breaker.executeProbe(mockFailureProbe);
  breaker.executeProbe(mockFailureProbe);
  EXPECT_EQ(breaker.getState(), CircuitState::CLOSED);

  // Fail 3: Should trip to OPEN
  breaker.executeProbe(mockFailureProbe);
  EXPECT_EQ(breaker.getState(), CircuitState::OPEN);
}

TEST(CircuitBreakerTest, ResetsOnSuccess)
{
  CircuitBreaker breaker("test.com", "80", 3, 5);

  // Fail twice then succeed
  breaker.executeProbe(mockFailureProbe);
  breaker.executeProbe(mockFailureProbe);
  breaker.executeProbe(mockSuccessProbe);

  EXPECT_EQ(breaker.getState(), CircuitState::CLOSED);
}