//Add all libraries
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
#include <math.h>
#include <fstream>
#include <vector>
#include <sstream>

using namespace std;

#define UDPPortC 23834   //The port on which to listen for incoming data
#define UDPAWS 24834	//AWS Client Port
#define SERVER "127.0.0.1"

class UDPServer		//UDPServer Class where all functions and variables related to UDP are defined
{
	public:
	struct sockaddr_in soc_server, soc_client;
   
	int s, i , recv_len;
	unsigned int slen;
	double msg[7],rlt[3];
	void setup();					//Setup UDP Port
	void detach();					//Close UDP Connection
	void sendtoaws(double *);		//Send data to AWS
	double *receive();				//Receive result
	double *compute(double *a);		//Compute results related to link id
	float round(float);			//Function to round-off
};

void UDPServer::setup()
{
	//create a UDP socket
    s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == -1)
    {
        cerr << "Error establishing the server UDP socket" << endl;
        exit(0);
    }
     
    // zero out the structure
    memset((char *) &soc_server, 0, sizeof(soc_server));
     
    soc_server.sin_family = AF_INET;
    soc_server.sin_port = htons(UDPPortC);
    soc_server.sin_addr.s_addr = htonl(INADDR_ANY);
    
    memset((char *) &soc_client, 0, sizeof(soc_client));     
    soc_client.sin_family = AF_INET;
    soc_client.sin_port = htons(UDPAWS);
    
     
    //bind socket to port
    if( bind(s , (struct sockaddr*)&soc_server, sizeof(soc_server) ) == -1)
    {
        cerr << "Error binding the server socket" << endl;
        exit(0);
    }

}

double *UDPServer::receive()
{
	//try to receive some data, this is a blocking call
	slen = sizeof(soc_server);
    recv_len = recvfrom(s, msg, 7*sizeof(double), 0, (struct sockaddr *) &soc_server, &slen);
    if (recv_len == -1)
    {
            cerr << "Error while receiving data from AWS!!"<< endl;
            exit(0);
    }
    return msg;
}

void UDPServer::sendtoaws(double *bf)
{
	int s1;
	//double mes;
	slen = sizeof(soc_client);
	//stringstream s(bf);
     //   s>>mes;
	s1= sendto(s, bf, 3*sizeof(double), 0, (struct sockaddr*) &soc_client, slen);
	if (s1 == -1)
    {
            cout<<"Error while sending data from ServerC to AWS!!"<<endl;
            exit(0);
    }
    else
    {
    cout<<"The Server C has finished sending the output to AWS"<<endl;
    cout<<"____________________________________________________"<<endl;
    }
}

//Compute Delay
double *UDPServer::compute(double *a)
{
	double r=0,C=0,B=0,S=0,N=0,F=0;		//C=channel capacity in bps, B=bandwidth in Hz,S=signal power in Watt
										//N= Noise Power in Watt, F= file size in bits
	double dtrans=0,dprop=0,dl=0,d1=0;			//dtrans= transmission delay, dprop=propogation delay
	double  NP=0,SP=0,b=0,L=0,V=0;
//	cout<<"Computing the result...\n";
	//Store data from array into individual variable.
	 F=a[1];		//File size in bit
	SP=a[2];		//Signal power in dB
	 b=a[3];		//Bandwidth in MHz
	 L=a[4];		//Length in Km
	 V=a[5];		//Velocity in 10^7
	 NP=a[6];		//Noise Power in dB
	 
	 N=pow(10,(NP/10));	//Convert dB to Watt
	 S=pow(10,(SP/10));	//Convert dB to Watt
	 B=b*pow(10,6);		//Convert Bandwidth
	 V=V*pow(10,7);		//Convert V in m/sec
	 L=L*pow(10,3);		//Convert Length in meters
	 ////Calculate channel capacity using Shannon's theorem
	
	 //Calculate rate C using shannon's theorem	
	 dl=S/N;		//Signal to Noise power
	 C=B*log2(1+dl);
	 //Calculate delays
	 dtrans=(F/C)*pow(10,3);
	 d1=log2(pow(2,3));
	 dprop=(L/V)*pow(10,3);
//	 cout<<"dl,C,d1: "<<dl<<"   "<<C<<"  "<<d1<<endl;
	 r=(dtrans+dprop);
	 //print for my reference
//	 cout<<"calculation variables: Signal,Noise,Bandwidth in Hz,Channel capacity(C),dtrans,dprop= "<<S
//	 <<", "<<N<<", "<<B<<", "<<C<<", "<<dtrans<<", "<<dprop<<endl;
//	 cout<<"end-to-end delay: "<<r<<endl;
			
	 rlt[0]=round(dtrans);
	 rlt[1]=round(dprop);
	 rlt[2]=round(r);
	 //rlt contains all delays 
	 cout<<"The Server C finished the calculation for link <"<<a[0]<<">"<<endl;
	 return rlt;	
}

float UDPServer::round(float var) 
{ 
    // 37.66666 * 100 =3766.66 
    // 3766.66 + .5 =37.6716    for rounding off value 
    // then type cast to int so value is 3766 
    // then divided by 100 so the value converted into 37.66 
    float value = (int)(var * 100 + .5); 
    return (float)value / 100; 
}
void UDPServer::detach()
{
	close(s);
}


int main()
{
	UDPServer udp;
	double *ar;
	double *result;
    
    udp.setup();
    cout<<"The Server C is up and running using UDP on port <"<<UDPPortC<<">"<<endl;
    
    while(1)
    {    
        ar=udp.receive();  
        cout<<"The Server C received Link information of link: <"<<ar[0]<<"> file size <"
        <<ar[1]<<"> signal power <"<<ar[2]<<">"<<endl;
  
 //       for(int i=0;i<7;i++)  	//print for my reference
 //      cout<<" "<<ar[i];
//        cout<<"\n"; 
        
        result=udp.compute(ar);		//Compute results i.e. Delay and store in result
	udp.sendtoaws(result);      	//send delay to AWS
    }
    
   udp.detach(); 		// Close the UDP Connection
   return 0;
}
