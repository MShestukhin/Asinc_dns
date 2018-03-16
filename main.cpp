#include <iostream>
#include <boost/asio.hpp>
#include "dnsclient.h"
#include <time.h>
using namespace std;

int main()
{
    int numberOfIteration=0;
    char hostname[100]="79834077832";
    boost::asio::io_service* io_service=new boost::asio::io_service;
    DnsClient dns(*io_service);//=new DnsClient(io_service);
    double start=clock();
    dns.makeBuf(hostname);
    dns.do_send(1);
    io_service->run();
    double stop=clock();
    printf("%f",(stop-start)/CLOCKS_PER_SEC);
    int i=0;
    //while(1){};
    delete io_service;
    return 0;
}

