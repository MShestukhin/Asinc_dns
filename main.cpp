#include <iostream>
#include <boost/asio.hpp>
#include "dnsclient.h"
#include <ctime>
using namespace std;
boost::asio::io_service* io_service=new boost::asio::io_service;
int func(int number,char* host){
    DnsClient dns(*io_service);//=new DnsClient(io_service);
    dns.do_send(host);
    if(number!=0) return func(number-1,host);
    else return 0;
}

int main()
{
    int numberOfIteration=0;
    printf("Enter number of iteration\n");
    std::cin>>numberOfIteration;
    double start=clock();
    char hostname[100]="79834077832";
    func(numberOfIteration,hostname);
    io_service->run();
    io_service->reset();
    delete io_service;
    double stop=clock();
    std::cout<<(stop-start)/10000.0;
    return 0;
}

