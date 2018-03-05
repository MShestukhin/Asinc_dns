#ifndef DNSCLIENT_H
#define DNSCLIENT_H
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <ctime>
class DnsClient
{
private:
    unsigned char buf[1024],*reader;
    struct DNS_HEADER *dns;
    struct QUESTION *qinfo;
    char *qname;
public:
    boost::asio::ip::udp::socket socket;
    u_char* ReadName(unsigned char* reader, int* count);
    DnsClient(boost::asio::io_service &io_service) : socket(io_service,{boost::asio::ip::udp::v4(),53})
    {/*buf[1024]; dns=NULL;qinfo=NULL;*/}
    clock_t start_time;
    void do_send(char *number, int numberOfIteration);
    void do_receive();
    void handle_send(const boost::system::error_code &error);
    void handle_receive(const boost::system::error_code &error);
    void ChangetoDnsNameFormat (char* dns,char* host);
};

#endif // DNSCLIENT_H
