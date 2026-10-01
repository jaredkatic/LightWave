#pragma once

#include <iostream>
#include <string>
#include <chrono>

// The 3 states of a Circuit Breaker state machine
enum class CircuitState
{
  CLOSED,   // Normal operation: Service is healthy, requests flow through
  OPEN,     // Tripped: Service is failing, block calls immediately (Fast Fail)
  HALF_OPEN // Recovery Test: Cooldown elapsed, allow 1 probe to check health
};

class CircuitBreaker
{
private:
  std::string host;
  std::string port;
  CircuitState state;

  int failure_threshold;
  int consecutive_failures;

  // Cooldown management
  std::chrono::steady_clock::time_point last_state_change;
  std::chrono::seconds cooldown_period;

public:
  CircuitBreaker(std::string h, std::string p, int max_failures, int cooldown_sec)
      : host(h),
        port(p),
        state(CircuitState::CLOSED),
        failure_threshold(max_failures),
        consecutive_failures(0),
        cooldown_period(cooldown_sec)
  {
    last_state_change = std::chrono::steady_clock::now();
  }

  void executeProbe(int (*probeFunc)(const char *, const char *))
  {
    auto now = std::chrono::steady_clock::now();

    if (state == CircuitState::OPEN)
    {
      auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_state_change);

      if (elapsed >= cooldown_period)
      {
        state = CircuitState::HALF_OPEN;
        std::cout << "[CIRCUIT BREAKER] Cooldown expired. Transitioning [" << host << "] to HALF-OPEN (Testing Mode)." << std::endl;
      }
      else
      {
        std::cout << "[CIRCUIT BREAKER] [" << host << "] Circuit is OPEN. Blocking request (Fast Fail)." << std::endl;
        return;
      }
    }

    int result = probeFunc(host.c_str(), port.c_str());

    if (result < 0)
    {
      consecutive_failures++;

      std::cout << "[CIRCUIT BREAKER] Probe failed for [" << host << "] ("
                << consecutive_failures << "/" << failure_threshold
                << ")" << std::endl;

      if (consecutive_failures >= failure_threshold || state == CircuitState::HALF_OPEN)
      {
        state = CircuitState::OPEN;
        last_state_change = std::chrono::steady_clock::now();
        std::cout << ">>> ALERT: Circuit tripped to OPEN for [" << host
                  << "] <<<" << std::endl;
      }
    }
    else
    {
      if (state == CircuitState::HALF_OPEN)
      {
        std::cout << ">>> RECOVERY: [" << host
                  << "] succeeded! Reseting the circuit to CLOSED. <<<" << std::endl;
      }
      consecutive_failures = 0;
      state = CircuitState::CLOSED;
    }
  }

  CircuitState getState() const { return state; }
  int getConsecutiveFailures() const { return consecutive_failures; }
  std::string getHost() const { return host; }
};