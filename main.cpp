#include <iostream>
#include <boost/asio.hpp>
#include "dnsclient.h"
#include <ctime>
using namespace std;

int main()
{
    boost::asio::io_service io_service;
    DnsClient* dns=new DnsClient(io_service);
    char hostname[100];
    int numberOfIteration=0;
    printf("Enter hostname\n");
    scanf("\n%s",hostname);
    printf("\nEnter iteration number\n");
    std::cin>>numberOfIteration;
    dns->do_send(hostname, numberOfIteration);
    io_service.run();
    return 0;
}

