#ifndef DNSCLIENT_H
#define DNSCLIENT_H
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <ctime>
#include <ocilib.h>
#include <string.h>
#include <set>
class DnsClient
{
private:
    char sendBuf[512], recBuf[512];
    int bufSize;
    char *qname;
    int iter;
    int recvPac;
    OCI_Connection* cn;
    OCI_Statement* st;
    OCI_Resultset*rs;
    boost::asio::io_service* io_serviceLocal;
public:
    std::vector<std::string> all_send_numbers;
    boost::asio::ip::udp::socket socket;
    char *ReadName(char *reader, char *buffer, int* count);
    DnsClient(boost::asio::io_service &io_service) : socket(io_service,{boost::asio::ip::udp::v4(),53})
    {
        io_serviceLocal=(boost::asio::io_service*)&io_service;
        iter=0;
        recvPac=0;
        OCI_Initialize(NULL, NULL, OCI_ENV_DEFAULT);
        //192.168.97.41:1521/sk
        cn = OCI_ConnectionCreate("192.168.97.41:1521/sk", "svcbic", "m83hose55tcp", OCI_SESSION_DEFAULT);
        st = OCI_StatementCreate(cn);
        OCI_Prepare(st,"select MSISDN from ES_MSISDNS");
        OCI_Execute(st);
        rs=OCI_GetResultset(st);

    }
    void do_send(int i);
    void makeBuf(char *number);
    void do_recive();
    void handle_send(const boost::system::error_code &error);
    void handle_receive(const boost::system::error_code &error);
    void ChangetoDnsNameFormat (char* dns,char* host);
};

#endif // DNSCLIENT_H
