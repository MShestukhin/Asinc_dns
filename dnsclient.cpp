#include "dnsclient.h"
#include<stdio.h>
#include<stdlib.h>
#include<boost/array.hpp>
#include <iostream>
#include <string.h>
#include <cstdlib>
#include <boost/timer/timer.hpp>
#include <boost/date_time/date.hpp>
#include <boost/date_time/time_duration.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/chrono.hpp>
#include <ctime>
#include <ocilib.h>
#define T_A 1 //Ipv4 address
#define T_NS 2 //Nameserver
#define T_CNAME 5 // canonical name
#define T_SOA 6 /* start of authority zone */
#define T_PTR 12 /* domain name pointer */
#define T_MX 15 //Mail server
#define T_NAPTR 35

struct DNS_HEADER
{
    unsigned short id; // identification number

    char rd :1; // recursion desired
    char tc :1; // truncated message
    char aa :1; // authoritive answer
    char opcode :4; // purpose of message
    char qr :1; // query/response flag

    char rcode :4; // response code
    char ad :1; // authenticated data
    char z :1; // its z! reserved

    short q_count; // number of question entries
    short ans_count; // number of answer entries
    short auth_count; // number of authority entries
    short add_count; // number of resource entries
};
struct QUESTION
{
    short qtype;
    short qclass;
};
struct R_DATA
{
    unsigned short type;
    unsigned short _class;
    unsigned int ttl;
    unsigned short data_len;
    unsigned short other;
    unsigned short pref;
    unsigned short flagLen;
    unsigned char flag;
    unsigned short servLen;
    unsigned char serv;
    unsigned short regLen;
    unsigned char reg;
};
#pragma pack(pop)

//Pointers to resource record contents
struct RES_RECORD
{
    char *name;
    struct R_DATA *resource;
    char *rdata;
    RES_RECORD(){
        name=0;
        resource=0;
        rdata=0;
    }
    ~RES_RECORD(){

    }
};
void DnsClient::ChangetoDnsNameFormat(char *dns, char *host){
    int lock = 0 , i;
        std::strcat(host,".");
        for(i = 0 ; i < strlen((char*)host) ; i++)
        {
            if(host[i]=='.')
            {
                *dns++= i-lock;
                for(;lock<i;lock++)
                {
                    *dns++=host[lock];
                }
                lock++;
            }
        }
        *dns++='\0';

}

char* DnsClient::ReadName(char* reader,char* buffer,int* count){
   char *name;
        int p=0,jumped=0;//,offset;
        *count = 1;
        name = (char*)malloc(256);
        name[0]='\0';
        //read the names in 3www6google3com format
        while(*reader!=0)
        {
            if(*reader>=192)
            {
                jumped = 1; //we have jumped to another location so counting wont go up!
            }
            else
            {
                name[p++]=*reader;
            }

            reader = reader+1;

            if(jumped==0)
            {
                *count = *count + 1; //if we havent jumped to another location then we can count up
            }
        }
        name[p]='\0'; //string complete
        if(jumped==1)
        {
            *count = *count + 1; //number of steps we actually moved forward in the packet
        }
        return name;
}


void DnsClient::makeBuf(char* number){
   char* host;
   int i;
   int sizeNumber=strlen((char*)number);
   char str[2*sizeNumber];
   int index=1;
   str[0]=number[sizeNumber-1];
   i=1;
    while(i!=sizeNumber+1){
            str[index]='.';
            str[index+1]=number[sizeNumber-i-1];
            index=index+2;
            i++;
    }
    host=(char*)&str;
    strcat(host,"e164.arpa\0");
    struct DNS_HEADER *dns = (struct DNS_HEADER *)&sendBuf;
    dns->id = (unsigned short) htons(getpid()+iter);
    dns->qr = 0; //This is a query
    dns->opcode = 0; //This is a standard query
    dns->aa = 0; //Not Authoritative
    dns->tc = 0; //This message is not truncated
    dns->rd = 1; //Recursion Desired
    dns->z = 0;
    dns->ad = 1;
    dns->rcode = 1;
    dns->q_count = htons(1); //we have only 1 question
    dns->ans_count = 0;
    dns->auth_count = 0;
    dns->add_count = 0;
    qname =(char*)&sendBuf[sizeof(struct DNS_HEADER)];
    ChangetoDnsNameFormat(qname , host);
    struct QUESTION* qinfo =(struct QUESTION*)&sendBuf[sizeof(struct DNS_HEADER) + (strlen((const char*)qname) + 1)];
    qinfo->qtype = htons(T_NAPTR);
    qinfo->qclass = htons(1);
    bufSize=sizeof(struct DNS_HEADER)+(strlen((const char*)qname)+1)+sizeof(struct QUESTION);
    i=1;
}

void DnsClient::do_send(int i){
    usleep(250);
            boost::asio::ip::udp::endpoint endpoint(
            boost::asio::ip::address::from_string("10.241.30.170"), 53);
    socket.async_send_to(boost::asio::buffer(sendBuf,bufSize),
                         endpoint,boost::bind(&DnsClient::handle_send ,
                                              this ,
                                              boost::asio::placeholders::error()));
}

void DnsClient::do_recive(){

}

void DnsClient::handle_send(const boost::system::error_code &error){
    if(!error){
        boost::asio::ip::udp::endpoint endpoint(
        boost::asio::ip::address::from_string("10.241.30.170"), 53); //10.241.30.170
        socket.async_receive_from(boost::asio::buffer(recBuf,512),endpoint,
                                  boost::bind(&DnsClient::handle_receive,
                                  this , boost::asio::placeholders::error()));
        iter++;
        //if(iter<1000){
        char str[100];
        //if(OCI_FetchNext(rs)!=0){
        int i=0;
        while(i<100){
        OCI_FetchNext(rs);
        sprintf(str,OCI_GetString(rs,1));
        all_send_numbers.push_back(std::string(str));
        makeBuf(str);
        do_send(1);
        i++;
        }
        io_serviceLocal->run();

    }
    else{
        printf("have some error");
    }
}

void DnsClient::handle_receive(const boost::system::error_code &error){
    if(!error){
        recvPac++;
        printf("%d recv\n",recvPac);
        struct RES_RECORD answers;
        int stop;
        char *reader = &recBuf[sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION)];
        stop=0;
                answers.name=ReadName(reader,recBuf,&stop);
                reader = reader + stop;

                answers.resource = (struct R_DATA*)(reader);
                reader = reader + sizeof(struct R_DATA);

                    answers.rdata = ReadName(reader,recBuf,&stop);
                    reader = reader + stop;
                    //for(int i=0; i<all_send_numbers.size();i++){
                     //   printf("%d", all_send_numbers.size());
                    std::string regStr=answers.rdata;
                    std::string::size_type n;
                    for(int i=0;i<all_send_numbers.size();i++){
                        std::string str=all_send_numbers.at(i);
                        std::cout<<"\nfrome buf "<<str;
                        std::cout<<"\nrdata "<<regStr;
                        n=regStr.find(str);
                        if(n!=std::string::npos){
                            printf("\n sovp");
                            all_send_numbers.erase(all_send_numbers.begin()+i);
                            break;
                        }
                    }

                    //printf("\n Regexp : %s",answers.rdata);
                    char* ident=answers.rdata+(strlen((char*)answers.rdata)-3);
                    char* mtsIdent="01!";
                    if((strcmp((char*)ident,mtsIdent))==0){
                    printf("\n Abonent mts");
                }
                    //io_serviceLocal->stop();
                    memset(recBuf,0,512);

    }
else{
        printf("Err to recieve");
        delete this;
}
}
