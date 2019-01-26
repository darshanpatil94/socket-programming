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
#include <csignal>
#include <cerrno>

using namespace std;
//Server side
#define ClientPort 25834	//TCP Client Port
#define MonitorPort 26834	//TCP Monitor Port

#define SERVER "127.0.0.1"		//local IP

#define UDPAWS 24834	  //AWS Port
#define UDPPortA 21834   //UDP Port A
#define UDPPortB 22834	 //UDP Port B
#define UDPPortC 23834	 //UDP Port C

void signal_handler(int sig); //function to catch the signal
int socket_OK=0;
int flag=0;

class TCPServer		//TCPServer Class where all function and variables related to TCP are defined
{
	public:
	int pt,awsSocket, newtcpSocket, socBindStatus;
	float delay;
	double a[3];
//	float r[3];
	struct sockaddr_in servAddr, newSockAddr;
        socklen_t newSockAddrSize;

	void setup(int port);			//TCP setup function
	void detach();				//TCP connection close function
	void sendresult(double rt);		//send results function
	double *receive();			//Receive data from Client function
	void sendtomonitor(double *);		//Send data to monitor function
	void socaccept();			//to keep accepting TCP connection even after client terminates
};

////////UDPClient
class UDPClient		//UDPClient class contains all functions and variable related to UDP connection to back-end servers
{
	public:
	struct sockaddr_in socaws,socserverA,socserverB,socserverC;
        int s, i , st,rf;
        unsigned int slen,sl;
        double mes[5],res[3];
    
	void setup(int);				//UDP setup function
	void detach();					//Close connection
	void sendtoserver(double,int);			//Send data to back-end server A,B
	void sendtoComputeServer(double *msg);		//Send data to computational server C
	double *receive();				//Receive data from server A and B
	double *receiveResult();			//Receive result from server C
};

void signal_handler(int sig)		//Can be used to handle various interupt signals, but Not needed for this project
{
    //cout<<"Disconnected, signal caught!! Error Number: "<<sig<<endl;
    socket_OK=0;
   	exit(0);
}

void UDPClient::setup(int PORT)
{
    s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if ( s == -1)
    {
        cerr << "Error establishing the AWS UDP socket" << endl;
        exit(0);
    }

	//initialize AWS Socket
    memset((char *) &socaws, 0, sizeof(socaws));     
    socaws.sin_family = AF_INET;
    socaws.sin_port = htons(UDPAWS);
    socaws.sin_addr.s_addr = htonl(INADDR_ANY);
    
    //initialize ServerA Socket
    memset((char *) &socserverA, 0, sizeof(socserverA));    
    socserverA.sin_family = AF_INET;
    socserverA.sin_port = htons(UDPPortA);

	//initialize ServerB Socket
    memset((char *) &socserverB, 0, sizeof(socserverB));     
    socserverB.sin_family = AF_INET;
    socserverB.sin_port = htons(UDPPortB);

	//initialize ServerC Socket
    memset((char *) &socserverC, 0, sizeof(socserverC));     
    socserverC.sin_family = AF_INET;
    socserverC.sin_port = htons(UDPPortC);  
    
    //bind AWS socket
    if( bind(s , (struct sockaddr *)&socaws, sizeof(socaws) ) == -1)
    {
        cerr << "Error binding the AWS socket" << endl;
        exit(0);
    }  
}

//send to UDP Back-Server
void UDPClient::sendtoserver(double msg, int ch)
{
	//send the message if ch=1 send to serverA, ch=2 send to serverB
	if(ch==1)
	{
		slen=sizeof(socserverA);
    	st = sendto(s, &msg, sizeof(msg) , 0 , (struct sockaddr *) &socserverA, slen);
    }
    else if(ch==2)
    {
    	slen=sizeof(socserverB);
    	st = sendto(s, &msg, sizeof(msg) , 0 , (struct sockaddr *) &socserverB, slen);
    }
    
    if(st==-1)
    {
       cerr << "Error while sending data!!" << endl;
       exit(0);
    }
}

//Send to compute Server
void UDPClient::sendtoComputeServer(double *msg)
{
	//send the message
	slen=sizeof(socserverC);
    st = sendto(s, msg, 7*sizeof(double) , 0 , (struct sockaddr *) &socserverC, slen);
    if (st==-1)
    {
       cerr << "Error while sending data to ServerC!!!" << endl;
       exit(0);
    }
}

//UDP Receive to receive from ServerA and B 
double *UDPClient::receive()
{
    slen=sizeof(socaws);
    rf = recvfrom(s, mes, 5*sizeof(double), 0, (struct sockaddr *) &socaws, &slen);
    if (rf == -1)
    {
      cerr << "Error while receiving data!!!" << endl;
      exit(0);
    }
    return mes;
}

//UDP Receive result from ServerC
double *UDPClient::receiveResult()
{
	//memset(buf,'\0', BUFLEN);
    //try to receive some data, this is a blocking call
    slen=sizeof(socaws);
    rf = recvfrom(s, res, 3*sizeof(double), 0, (struct sockaddr *) &socaws, &slen);
    if (rf == -1)
    {
      cerr << "Error while receiving data!!!" << endl;
      exit(0);
    }
    return res;
}

void UDPClient::detach()
{
	close(s);
}
////////end of UDPClient

//////Start of TCP function definitions
void TCPServer::setup(int port)
{
 	//setup a socket and connection tools
 	pt=port;
    bzero((char*)&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(port); 
    //open stream oriented socket with internet address
    //also keep track of the socket descriptor
    awsSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(awsSocket < 0)
    {
        cerr << "Error establishing the server socket" << endl;
        exit(0);
    }
    //bind the socket to its local address
    socBindStatus = bind(awsSocket, (struct sockaddr*) &servAddr, sizeof(servAddr));
    if(socBindStatus < 0)
    {
        cerr << "Error binding socket to local address" << endl;
        exit(0);
	}

    if(pt == ClientPort)   
    cout << "Waiting for a client to connect..." << endl;
    else if(pt == MonitorPort)
    cout << "Waiting for a Monitor to connect..." << endl;
    //listen for up to 3 requests at a time
    listen(awsSocket, 20);

	socaccept();   
}

//Defined SocAccept to accept connections and wait for client
void TCPServer::socaccept()
{
    newSockAddrSize = sizeof(newSockAddr);
    //accept, create a new socket descriptor to 
    //handle the new connection with client
    newtcpSocket = accept(awsSocket, (sockaddr *)&newSockAddr, &newSockAddrSize);
    if(newtcpSocket < 0)
    {
        cerr << "Error accepting request from client!" << endl;
        exit(errno);
    }
//    cout << "Connected with client!" << endl;
 	socket_OK=1;
}

void TCPServer::detach()
{
	close(awsSocket);
	close(newtcpSocket);
	cout << "\nAWS Connection closed" << endl;
}

void TCPServer::sendresult(double rt)
{
	  send(newtcpSocket , &rt , sizeof(rt) , 0);
	  
}

void TCPServer::sendtomonitor(double *arr)
{
	send(newtcpSocket , arr , 3*sizeof(double) , 0 );
}
double *TCPServer::receive()
{
	read(newtcpSocket , a, 3*sizeof(double));
    return a;
}


int main()
{	
	double val[7];
	UDPClient udp;
	UDPClient udpA, udpB, udpC;
	double *result;
	double num=0;
	int flag1=0,flag2=0;
	double msg=0;
	double *buf,*arr;
	double buf1[5],buf2[5];
    TCPServer tcp1,tcp2;			//Create tcp1 and tcp2 objects related tcp1-->Client, tcp2-->Monitor
    
    signal(SIGINT,signal_handler);		//Initialize signal to catch interupt handler signals
    
    udp.setup(UDPAWS); 			//Setup UDP Ports
    tcp2.setup(MonitorPort);		//Wait for Monitor to connect and setup connection
    tcp1.setup(ClientPort);		//Wait for client to connect and setup connection
    
	cout<<"The AWS is up and running."<<endl;
	socket_OK=1;
	 while(1)
    {
    	if(!socket_OK)
    	{
    		cout<<"\nWaiting for Client to Connect and send link-id..."<<endl;
    		tcp1.socaccept(); 
    	}
    	else
    	{  
    	arr= tcp1.receive();

// For Debugging
//    	cout<<"//////////////////////socket_OK"<<endl;
//		cout<<"Enter link id, filesize, power: ";
//		for(int i=0;i<3;i++)
//			cin>>arr[i];
		
		flag=arr[1];
    	  if(flag!=0)
    	  {
		cout<<"The AWS received link ID=<"<<val[0]<<">, size=<"<<val[1]<<">, and power=<"<<val[2]
    		<<">  from the client using TCP over port<"<<ClientPort<<">"<<endl;
		tcp2.sendtomonitor(arr);
		cout<<"The AWS sent link ID=<"<<val[0]<<">, size=<"<<val[1]<<">, and power=<"<<val[2]
    		<<">  to the monitor using TCP over port <"<<MonitorPort<<">"<<endl; 
			for(int i=0;i<3;i++)
			{
				val[i]=arr[i];
				arr[i]=0;
				//flag=0;
			}
	     
	num=val[0]; 		//Get link-id   
    	udp.sendtoserver(num,1);			//send link id to server A--- pass 1 for serverA
        cout<<"The AWS sent link ID=<"<<val[0]<<"> to Backend-Server A using UDP over port <"<<UDPAWS<<">"<<endl;
        buf= udp.receive();			//receive data related to link id from serverA  
//        cout<<"buf1[]: "<<buf1[0]<<buf1[1]<<endl;
		for(int i=0;i<5;i++)
			{
				buf1[i]=buf[i];
				buf[i]=0;
			}
        udp.sendtoserver(num,2);		//send link id to server B -- pass 2 for serverB
        cout<<"The AWS sent link ID=<"<<val[0]<<"> to Backend-Server B using UDP over port <"<<UDPAWS<<">"<<endl;
      //  usleep(dl);
         buf=udp.receive();				//receive data related to link id from serverB
         for(int i=0;i<5;i++)
			{
				buf2[i]=buf[i];
				buf[i]=0;
//				cout<<"buf1[], buf2[]: "<<buf1[i]<<",  "<<buf2[i]<<endl;
			}      
               
			if(buf1[1]!=0)
				flag1=1;
			else
				flag1=0;
			if(buf2[1]!=0)
				flag2=1;
			else
				flag2=0;
		
		cout<<"The AWS received <"<<flag1<<"> matches from Backend-Server A using UDP over port <"<<UDPAWS<<">"<<endl;
		cout<<"The AWS received <"<<flag2<<"> matches from Backend-Server B using UDP over port <"<<UDPAWS<<">"<<endl;

        	if(buf1[1]!=0)		//Check if link id received from server A is not zero
        	{
            	for(int i=3;i<7;i++)
	        		val[i]=buf1[i-2];
	        		
	        	buf1[1]=0;
	        
        	udp.sendtoComputeServer(val);
        	cout<<"The AWS sent link ID=<"<<val[0]<<">, size=<"<<val[1]<<">, power=<"<<val[2]
        	<<">, and link information to Backend-Server C using UDP over port <"<<UDPAWS<<">"<<endl;
        	result=udp.receiveResult();		//receive calculated result from serverC
        	cout<<"The AWS received outputs from Backend-Server C using UDP over port <"<<UDPAWS<<">"<<endl;
        	num=result[2];
        	tcp1.sendresult(num);
        	tcp2.sendtomonitor(result);
        	cout<<"The AWS sent delay=<"<<result[2]<<">ms to the client using TCP over port <"<<ClientPort<<">"<<endl;
        	cout<<"The AWS sent detailed results to the monitor using TCP over port <"<<MonitorPort<<">"<<endl;
        	cout<<"_______________________________________________________________________________"<<endl;
        	}
        	else if(buf2[1]!=0)				//check if link id received from server B is not zero
        	{
        		for(int i=3;i<7;i++)
        		val[i]=buf2[i-2];
        	
        	buf2[1]=0;		
        	udp.sendtoComputeServer(val);
        	cout<<"The AWS sent link ID=<"<<val[0]<<">, size=<"<<val[1]<<">, power=<"<<val[2]
        	<<">, and link information to Backend-Server C using UDP over port <"<<UDPAWS<<">"<<endl;
        	result=udp.receiveResult();			//receive calculated result
        	cout<<"The AWS received outputs from Backend-Server C using UDP over port <"<<UDPAWS<<">"<<endl;
        	num=result[2];
        	tcp1.sendresult(num);				//send result to client
        	tcp2.sendtomonitor(result);			//send result to monitor
        	cout<<"The AWS sent delay=<"<<result[2]<<">ms to the client using TCP over port <"<<ClientPort<<">"<<endl;
        	cout<<"The AWS sent detailed results to the monitor using TCP over port <"<<MonitorPort<<">"<<endl;
        	cout<<"_______________________________________________________________________________"<<endl;
        	}
        	else
        	{
        	for(int i=0;i<3;i++)
        		result[i]=0;   		
        	num=0;
        	tcp1.sendresult(num);			//send result to client
        	tcp2.sendtomonitor(result);		//send result to monitor
        	cout<<"The AWS sent 'No Match' to the monitor and the client using TCP over ports <"
        	<<MonitorPort<<"> and <"<<ClientPort<<">, respectively\n"<<endl;
        	cout<<"_______________________________________________________________________________"<<endl;
        	}
          }
       else
       {
    	cout<<"\nWaiting for Client to Connect and send Link-Id..."<<endl;
    	tcp1.socaccept();
    	}
    }
       
    }  
 	
 	tcp1.detach();  		//Close tcp1 connection
	tcp2.detach();			//Close tcp2 connection
        udp.detach();    		//UDP Connection
    return 0;   
}

