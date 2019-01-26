//Darshan Patil
//Add all required libraries
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
#include <fstream>
#include <vector>
#include <sstream>

using namespace std;

#define UDPPortB 22834   // ServerB Port number
#define UDPAWS 24834	//  AWS Server Port number
#define SERVER "127.0.0.1" //Local IP address

class UDPServer			//UDP Class
{
	public:
	struct sockaddr_in soc_server, soc_client;
   	int s, i , recv_len;
        unsigned int slen;
        double msg;
	void setup();			//Function to setup UDP Connection
	void detach();			//Function to close UDP Connection
	void sendtoaws(double *);	//Function to send data to AWS
	double receive();		//Function to receive data in AWS
};

class File		//File Class
{
	public:
	int j,k, row, col;
	double arr[5];
	ifstream myFile;
    	string line, val;
    	vector < vector<double> > vec;		//Vector array to store data from .csv file
    	void readfile();			//Function to read data from file
    	void displayfile();			//Function to display data from file
    	double *search(double l);		//Function to search link-id related data in file
    	void print(double *arr);		//Function to print the data related to link-id
    	void open();				//Function to open the file
    	void close();  
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
    soc_server.sin_port = htons(UDPPortB);
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

double UDPServer::receive()
{
	//try to receive some data, this is a blocking call
    slen = sizeof(soc_server);
    recv_len = recvfrom(s, &msg, sizeof(msg), 0, (struct sockaddr *) &soc_server, &slen);
    if (recv_len == -1)
    {
            cerr << "Error while receiving data from AWS!!" << endl;
            exit(0);
    }

    return msg;
}

void UDPServer::sendtoaws(double *bf)
{
	//Send data related to link-id to AWS
	int s1;
	slen = sizeof(soc_client);
	
	s1= sendto(s, bf, 5*sizeof(double), 0, (struct sockaddr*) &soc_client, slen);
	if (s1 == -1)
	    {
            cout<<"Error while sending data from Server B to AWS!!!"<< endl;
            exit(0);
	    }
	else
	{
    	cout<<"The Server B has finished sending the output to AWS"<<endl;
    	cout<<"____________________________________________________"<<endl;
    	}
}

void UDPServer::detach()
{
	close(s);
}

void File::readfile()
{
   //Read .csv file and store the data in Vector v[][]
	row=0;
    while (getline(myFile,line)) 
    {
        vector<double> v;                 		// row vector v
        col=0;
        stringstream s(line);                  // stringstream line
        
        while (getline (s, val, ','))          // get each value (',' delimited)
        {
            v.push_back(atof(val.c_str()));    // add to row vector
            col++;
        }
     vec.push_back(v);
     row++;
    }
}

void File::displayfile()
{
	//Display the data in database file stored in vector vec[][]
	for(int m=0;m<vec.size();m++)
    {
    		for(int n=0;n<vec[m].size();n++)
    		{
        		cout<<"  "<<vec[m][n];
        	} 
        cout<<"\n";
    }
    cout<<"\nrow: "<<row<<" col: "<<col<<"\n\n";
}

void File::open()
{
	myFile.open("database_b.csv");
	if(!myFile.is_open()) 
    	cout <<"ERROR!! File Not Opened!\n";
}

void File::close()
{
	myFile.close();
}

double *File::search(double l)
{
	//Search for data related to link-id in Vector vec[][]
	int j=0;
	int flag=0;
	for(int j=0; j<5;j++)
		arr[j]=0;
	
	for(int m=0;m<vec.size();m++)
    {
        if(vec[m][0] == l)
        {    
        	flag=1;   	
        	for(int j=0; j<5;j++)
			arr[j]=vec[m][j];
        }
    } 
    cout<<"The Server B has found "<<flag<<" match"<<endl;
    return arr;
}

void File::print(double *al)
{
	cout<<"DATA related to Link ID: ";
	for(int l=0;l<5;l++)
	{
		cout<<"  "<<al[l];
		//cout<<"link ID: "<<al[0]<<"\nBW: "<<vec[m][1];
        	//cout<<"\nlength: "<<vec[m][2]<< "\nVelocity: "<<vec[m][3]<<"\nNoise Power: "<<vec[m][4]<<endl;
	}
	cout<<"\n";
}

int main()
{
	double mes;
	UDPServer udp;
	File file;
	double *ar;
	double result=0;
    
	file.open();	//Open the database_b.csv file
	udp.setup();	//Setup UDP PORT
	cout<<"The Server A is up and running using UDP on port <"<<UDPPortB<<">"<<endl;
	file.readfile();	//Read data from file store in vector
//    file.displayfile();	//Uncomment to display data related to link id
	file.close();		//Close the database file
    
    while(1)
    {     
        mes= udp.receive();		//Receive link id from AWS
        cout<<"The Server B received input <"<<mes<<">"<<endl;  
        ar= file.search(mes);		//Search the link id info
//        file.print(ar);     		//print the array for my reference
	udp.sendtoaws(ar);  		//send the link id if found or not found    
    }
 
   udp.detach();				//close udp connection
 
   return 0;
}
