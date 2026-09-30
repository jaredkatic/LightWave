#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <chrono>
#include <unistd.h>

std::string getHostIpAddress(const char *domainName);
int connectionHealthProbe(const char *host, const char *port);

int main(int argc, char const *argv[])
{
  // std::string output;

  // output = getHostIpAddress("google.com");

  // if (!output.empty())
  // {
  //   std::cout << output << std::endl;
  // }
  // else
  // {
  //   std::cout << "Could not resolve IP address." << std::endl;
  // }

  connectionHealthProbe("google.com", "80");

  return 0;
}

int connectionHealthProbe(const char *host, const char *port)
{
  struct addrinfo hints{}, *res = nullptr;

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if (getaddrinfo(host, port, &hints, &res) != 0)
  {
    std::cerr << "Failed to resolve host" << std::endl;
    return -1;
  }

  int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (sockfd < 0)
  {
    freeaddrinfo(res);
    return -1;
  }

  auto start = std::chrono::high_resolution_clock::now();

  if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0)
  {
    std::cerr << "Connection failed to " << host << std::endl;
    close(sockfd);
    freeaddrinfo(res);
    return -1;
  }

  auto elapsed = std::chrono::high_resolution_clock::now() - start;
  long long latency_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

  std::string request = "GET / HTTP/1.1\r\nHost: " + std::string(host) + "\r\nConnection: close\r\n\r\n";
  send(sockfd, request.c_str(), request.length(), 0);

  char buffer[1024] = {0};
  int bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);

  if (bytes_received > 0)
  {
    std::cout << "SUCCESS [" << host << "] Latency: " << latency_ms << "ms" << std::endl;

    std::cout << "--- Raw Response Header ---" << std::endl;
    std::cout << buffer << std::endl;
    std::cout << "---------------------------" << std::endl;

    if (std::string(buffer).find("HTTP/1.1 200 OK") != std::string::npos ||
        std::string(buffer).find("HTTP/2 200") != std::string::npos)
    {
      std::cout << "Status: HEALTHY (200 OK)" << std::endl;
    }
  }

  close(sockfd);
  freeaddrinfo(res);
  return 0;
};

std::string getHostIpAddress(const char *domainName)
{
  struct addrinfo hints{}, *res = nullptr;
  char addrstr[INET6_ADDRSTRLEN];

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  hints.ai_flags |= AI_CANONNAME;

  int errcode = getaddrinfo(domainName, nullptr, &hints, &res);
  if (errcode != 0)
  {
    std::cerr << "getaddrinfo failed: " << gai_strerror(errcode) << std::endl;
    return "";
  }

  std::string final_ip = "";

  std::cout << "Host: " << domainName << std::endl;

  for (struct addrinfo *p = res; p != nullptr; p = p->ai_next)
  {
    void *addr = nullptr;

    if (p->ai_family == AF_INET)
    {
      struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
      addr = &(ipv4->sin_addr);
    }
    else if (p->ai_family == AF_INET6)
    {
      struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
      addr = &(ipv6->sin6_addr);
    }

    if (addr != nullptr)
    {
      inet_ntop(p->ai_family, addr, addrstr, sizeof(addrstr));
      final_ip = addrstr;

      // Prefer IPv4 if available, otherwise keep IPv6
      if (p->ai_family == AF_INET)
      {
        break;
      }
    }
  }

  freeaddrinfo(res);
  return final_ip;
};