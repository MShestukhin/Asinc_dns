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

    unsigned char rd :1; // recursion desired
    unsigned char tc :1; // truncated message
    unsigned char aa :1; // authoritive answer
    unsigned char opcode :4; // purpose of message
    unsigned char qr :1; // query/response flag

    unsigned char rcode :4; // response code
    unsigned char ad :1; // authenticated data
    unsigned char z :1; // its z! reserved

    unsigned short q_count; // number of question entries
    unsigned short ans_count; // number of answer entries
    unsigned short auth_count; // number of authority entries
    unsigned short add_count; // number of resource entries
};
struct QUESTION
{
    unsigned short qtype;
    unsigned short qclass;
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
    unsigned char *name;
    struct R_DATA *resource;
    unsigned char *rdata;
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

u_char* DnsClient::ReadName(unsigned char* reader,unsigned char* buffer,int* count){
    unsigned char *name;
        unsigned int p=0,jumped=0;//,offset;
        *count = 1;
        name = (unsigned char*)malloc(256);
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
    str[index-1]='\0';
    char* host;
    host=(char*)&str;
    strcat(host,"e164.arpa");
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
    //std::cout<<<<"\n";
            boost::asio::ip::udp::endpoint endpoint(
            boost::asio::ip::address::from_string("10.241.30.170"), 53);
    socket.async_send_to(boost::asio::buffer(sendBuf,bufSize),
                         endpoint,boost::bind(&DnsClient::handle_send ,
                                              this ,
                                              boost::asio::placeholders::error()));
    //boost::chrono::system_clock::time_point time=boost::chrono::system_clock::now();
    //std::cout<<time;
}

void DnsClient::do_recive(){

}

void DnsClient::handle_send(const boost::system::error_code &error){
    if(!error){
        boost::asio::ip::udp::endpoint endpoint(
        boost::asio::ip::address::from_string("10.241.30.170"), 53);
        socket.async_receive_from(boost::asio::buffer(recBuf,512),endpoint,
                                  boost::bind(&DnsClient::handle_receive,
                                  this , boost::asio::placeholders::error()));
        iter++;
        char mts_number_begin[100]="7952407";
        int number=6000+iter;
        char str_number[100]="";
        std::sprintf(str_number,"%d",number);
        std::strcat(mts_number_begin,str_number);
        makeBuf(mts_number_begin);
        if(iter<1000)
        do_send(1);
    }else{
        printf("have some error");
    }
}

void DnsClient::handle_receive(const boost::system::error_code &error){
    if(!error){
        printf("%d\n", recvPac);
        recvPac++;
        struct RES_RECORD answers;
        int stop,j;
        struct DNS_HEADER *dns = (struct DNS_HEADER*) recBuf;
        unsigned char *reader = &recBuf[sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION)];
        stop=0;
                answers.name=ReadName(reader,recBuf,&stop);
                reader = reader + stop;

                answers.resource = (struct R_DATA*)(reader);
                reader = reader + sizeof(struct R_DATA);

                    answers.rdata = ReadName(reader,recBuf,&stop);
                    reader = reader + stop;

                //if(ntohs(answers.resource->type)==T_NAPTR)
                //{
                    //printf("\n Regexp : %s",answers.rdata);
                    unsigned char* ident=answers.rdata+(strlen((char*)answers.rdata)-3);
                    char* mtsIdent="01!";
                    if((strcmp((char*)ident,mtsIdent))==0){
                    //printf("\n Abonent mts");
                }
                    //memset(recBuf,0,512);
            //}

    }
else{
        printf("Err to recieve");
        delete this;
    }
}
