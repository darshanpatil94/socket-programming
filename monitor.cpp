#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <csignal>
using namespace std;
//Monitor side
#define MonitorPort 26834		//Monitor Port
#define ServerIP "127.0.0.1"

int flag=0;
int socket_OK=0;

void signal_handler(int sig);

class TCPMonitor		//TCPMonitor class contains all TCP functions and variables
{
	public:
	int monitorSd, newSd, status;
	double a[3];
	double r[3];
	struct hostent* host; 
    struct sockaddr_in sendSockAddr; 
	void setup();			//Function to setup TCP
	void detach();			//Function to close TCP
	double *receive();		//Function to receive Inputs of Client from AWS
	double *receivedresult();	//Function to receive results from AWS
};

void signal_handler(int sig)
{
    //cout<<"AWS Disconnected, SIGNAL caught! Error Number: "<<sig<<endl;
    socket_OK=0;
    exit(1);
}

void TCPMonitor::setup()
{
	//setup a socket and connection tools 
    host = gethostbyname(ServerIP);  
    bzero((char*)&sendSockAddr, sizeof(sendSockAddr)); 
    
    sendSockAddr.sin_family = AF_INET; 
    
    sendSockAddr.sin_addr.s_addr = 
        inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
    
    sendSockAddr.sin_port = htons(MonitorPort);
    monitorSd = socket(AF_INET, SOCK_STREAM, 0);
    //try to connect...
    status = connect(monitorSd,
                         (sockaddr*) &sendSockAddr, sizeof(sendSockAddr));
    if(status < 0)
    {
        cout<<"Error connecting to socket!"<<endl; 
        exit(0);
    }
    socket_OK=1;
}

double *TCPMonitor::receive()
{
	for(int i=0;i<3;i++)
		a[i]=0;
	recv(monitorSd , a, 3*sizeof(double),0);
	return a;
}

double *TCPMonitor::receivedresult()
{
	read(monitorSd , r, 3*sizeof(double));
	//flag=1;
	return r;
}

void TCPMonitor::detach()
{
	close(monitorSd);
	//cout << "\nConnection closed!!" << endl;
}

int main()
{
    TCPMonitor tcp;		//tcp object
    double *res;
    double ip[3],result[3];
    double num=0;
    
    signal(SIGINT,signal_handler);
    tcp.setup();		//setup tcp connection
	cout<<"The Monitor is up and running."<<endl;
	socket_OK=1;
    while(1)
    {
    if(!socket_OK)
    	cout<<"CONNECTION BROKE!"<<endl;	//if tcp connection broke
   
     res=tcp.receivedresult();		//recieve inputs
     flag=1;
     for(int i=0;i<3;i++)
	 {
	 	ip[i]=res[i];
	 	res[i]=0;
	 }
	 
     if(flag)
     {
	 if(ip[0]==0 &&ip[1]==0&&ip[2]==0)
	 {
	 	cout<<"AWS Disconnected!\n";	//if TCP Connection broke
	 	tcp.detach();
		 exit(0);
	 } 
	 cout<<"The monitor received link = <"<<ip[0]<<">,size =<"<<ip[1]<<">, and power =<"<<ip[2]<<"> from the AWS"<<endl;
	 res=tcp.receive();
	 for(int i=0;i<3;i++)
	 {
	 	result[i]=res[i];
	 	res[i]=0;
	 }
	 	if(result[2]!=0)
	 	{
			cout<<"The result for link <"<<ip[0]<<">:\nTt =<"<<result[0]<<">ms"<<",\nTp=<"<<result[1]<<">ms"<<
			",\nDelay=<"<<result[2]<<">ms\n"<<endl;
			cout<<"---------------------------------------------------------------"<<endl;
  		 }
	 	else
	 	{
			cout<<"Found no matches for link <"<<num<<">"<<endl;
			cout<<"---------------------------------------------------------------"<<endl;
	 	}
	 flag=0;
	 }
	}
    tcp.detach(); 	//close TCP connections

    return 0;    
}
