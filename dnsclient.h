#ifndef DNSCLIENT_H
#define DNSCLIENT_H
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <ctime>
class DnsClient
{
private:
    unsigned char sendBuf[512], recBuf[512];
    int bufSize;
    char *qname;
    int iter;
public:
    boost::asio::ip::udp::socket socket;
    u_char* ReadName(unsigned char* reader,unsigned char* buffer,int* count);
    DnsClient(boost::asio::io_service &io_service) : socket(io_service,{boost::asio::ip::udp::v4(),53}){iter=0;}
    void do_send(int i);
    void makeBuf(char *number);
    void do_recive();
    void handle_send(const boost::system::error_code &error);
    void handle_receive(const boost::system::error_code &error);
    void ChangetoDnsNameFormat (char* dns,char* host);
};

#endif // DNSCLIENT_H
