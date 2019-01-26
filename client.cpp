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
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include<csignal>

using namespace std;
//Client side
#define ClientPort 25834	//Client Port
#define ServerIP "127.0.0.1"	//Local IP address
void signal_handler(int sig); //function to catch the signal

class TCPClient		//TCPClient contains all function and variables related to TCP
{
	public:
	int clientSocket, newclientSocket, bindStatus;	
	double result;
	struct hostent* host; 
    	struct sockaddr_in sendSockAddr; 
	void setup();				//Setup TCP Connection
	void detach();				//Close TCP connection
	void sendtoaws(double *a);		//Send data to AWS
	double receive(double *a);		//Receive results from AWS
};

void signal_handler(int sig)
{
    //cout<<"Client Disconnected, SIGPIPE signal caught!! Error Number: "<<sig<<endl;
   	exit(0);
}

void TCPClient::setup()
{
   //setup a socket and connection tools 
    host = gethostbyname(ServerIP);  
    bzero((char*)&sendSockAddr, sizeof(sendSockAddr)); 
    
    sendSockAddr.sin_family = AF_INET; 
    
    sendSockAddr.sin_addr.s_addr = 
        inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
    
    sendSockAddr.sin_port = htons(ClientPort);
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
   //Bind the socket
    bindStatus = connect(clientSocket,(sockaddr*) &sendSockAddr, sizeof(sendSockAddr));
    if(bindStatus < 0)
    {
        cout<<"Error connecting to socket!"<<endl; 
        exit(0);
    }
//    cout << "Connected to the server!" << endl;
}

void TCPClient::sendtoaws(double *a)
{
	send(clientSocket , a , 3*sizeof(double) , 0 );
	cout<<"The Client sent LinkID=<"<<a[0]<<">, FileSize=<"<<a[1]<<">, Power=<"<<a[2] 
	<<"> to AWS"<<endl;
}

double TCPClient::receive(double *a)
{
	result=0;
	read(clientSocket , &result, sizeof(result));
	if(result!=0)
	cout<<"The delay for link <"<<a[0]<<"> is <"<<result<<">ms"<<endl;
	else
	cout<<"Found no matches for link <"<< a[0]<<">"<<endl;
	return result;
}

void TCPClient::detach()
{
	close(clientSocket);
	//cout << "\nConnection closed!!" << endl;
	cout<<"________________________________________________________________"<<endl;
}

int main(int argc, char *argv[])
{
    signal(SIGINT,signal_handler);	//initialize signal SIGINT handles ctrl+C signal
    double linkid,filesize,power,result=0;
    if(argc != 4)
    {
        cerr << "Not Valid Arguments! Please Enter LinkID, FileSize in bit and Signal-Power in dB" << endl; 
        exit(0); 
    } //grab the arguments
    
    TCPClient tcp;
    
//	linkid = atoi(argv[1]);
	linkid = atoi(argv[1]);
	filesize = atof(argv[2]);
	power = atof(argv[3]);   
    double a[3]={linkid,filesize,power};	//Store received values in an array a[]
    
    tcp.setup();		//Setup tcp connection with AWS
    cout<<"The Client is up and running."<<endl;
  
    tcp.sendtoaws(a);		//Send data to AWS
    result=tcp.receive(a);    	//Receive results from AWS
    tcp.detach();		//Close Client TCP Port

    return 0;    
}
