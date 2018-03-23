typedef struct inputdata 
{
	int n;
	float r;
	float lambda;
	float mu;
	int B;
	int P;	
	char *filename[1024];
	int packetsenteredQ2;
	int packetendcount;
	int tokentotalcount;
	int tokendropcount;
	int packetdropcount; 
	int packetsarrived;	
	double timeinQ1;
	double timeinQ2;
	double timeinS1;
	double timeinS2;
	double totalinterarrivaltime;
	double totalservicetime;
	double totaltimeinsystem;
	double totaltimeinsystemsquared;
}MyData;


typedef struct packetdata 
{	
  	float lambda;
  	float mu;
	int P;
	int pnumber;
	struct timeval systemarrivaltime;
        struct timeval q1arrivaltime;
	struct timeval q1departuretime;	
	struct timeval q2arrivaltime;
	struct timeval q2departuretime;	
	struct timeval serverarrivaltime;
	struct timeval serverdeparturetime;
}MyPacket;
