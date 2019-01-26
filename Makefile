all: output

output: serverA1 serverB1 serverC1 aws1 monitor1 client1

serverA1: serverA.cpp
	 g++ -o serverAoutput serverA.cpp

serverB1: serverB.cpp
	 g++ -o serverBoutput serverB.cpp

serverC1: serverC.cpp
	 g++ -o serverCoutput serverC.cpp

aws1: aws.cpp
	 g++ -o awsoutput aws.cpp

monitor1: monitor.cpp
	 g++ -o monitoroutput monitor.cpp

client1: client.cpp
	 g++ -o client client.cpp

serverA: serverA1
	 ./serverAoutput

serverB: serverB1
	 ./serverBoutput

serverC: serverC1
	 ./serverCoutput

monitor: monitor1
	 ./monitoroutput
aws: aws1
	 ./awsoutput

.phony: serverA serverB serverC aws client
