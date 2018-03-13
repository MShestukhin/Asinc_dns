#include <iostream>
#include <boost/asio.hpp>
#include "dnsclient.h"
#include <time.h>
using namespace std;

int func(int number,char* host){
    boost::asio::io_service* io_service=new boost::asio::io_service;
    DnsClient dns(*io_service);//=new DnsClient(io_service);
    dns.makeBuf(host);
    dns.do_send(1);
    io_service->poll();
    delete io_service;
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
    double stop=clock();
    printf("%f",(stop-start)/CLOCKS_PER_SEC);
    return 0;
}

