#ifndef DNSCLIENT_H
#define DNSCLIENT_H
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <ctime>
class DnsClient
{
private:
    unsigned char buf[512];
    char *qname;
public:
    boost::asio::ip::udp::socket socket;
    u_char* ReadName(unsigned char* reader,unsigned char* buffer,int* count);
    DnsClient(boost::asio::io_service &io_service) : socket(io_service,{boost::asio::ip::udp::v4(),53}){}
    void do_send(char *number);
    void handle_send(const boost::system::error_code &error);
    void handle_receive(const boost::system::error_code &error);
    void ChangetoDnsNameFormat (char* dns,char* host);
};

#endif // DNSCLIENT_H
