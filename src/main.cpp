#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

std::string getHostIpAddress(const char *domainName);

int main(int argc, char const *argv[])
{
  std::string output;

  output = getHostIpAddress("google.com");

  std::cout << output << std::endl;
  return 0;
}

std::string getHostIpAddress(const char *domainName)
{
  struct addrinfo hints, *res;
  int errcode;
  char addrstr[100];
  void *ptr;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags |= AI_CANONNAME;

  errcode = getaddrinfo(domainName, nullptr, &hints, &res);
  if (errcode != 0)
  {
    std::cout << "getaddrinfo failed with error: " << errcode << std::endl;
    return "";
  }

  std::cout << "Host: " << domainName << std::endl;

  struct addrinfo *head = res;

  while (res)
  {
    switch (res->ai_family)
    {
    case AF_INET:
      ptr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
      inet_ntop(res->ai_family, ptr, addrstr, 100);
      break;
    case AF_INET6:
      ptr = &((struct sockaddr_in6 *)res->ai_addr)->sin6_addr;
      inet_ntop(res->ai_family, ptr, addrstr, 100);
      break;
    };

    if (res->ai_family == AF_INET)
    {
      std::string final_ip = addrstr;
      freeaddrinfo(head);
      return final_ip;
    }

    res = res->ai_next;
  }

  freeaddrinfo(head);
  return addrstr; // retuns wrong address as it iterates past the domain name to the gateway ip
};