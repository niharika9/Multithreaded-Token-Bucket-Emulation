#include <math.h>
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#define __USE_XOPEN
#include <ctype.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "defs.h"
#include "my402list.h"
#include "cs402.h"
#include "warmup2.h"
#define LINE 1026
#define threads 4
#define _GNU_SOURCE
extern int errno;

struct timeval starttime;
int counttoken;
My402List Q1,Q2;
pthread_t thr[threads];
pthread_t ctrlthread;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
sigset_t set;
bool ctrlCcheck;
bool server1isServicing,server2isServicing;
static char gszProgName[MAXPATHLENGTH];

static
void Usage()
{
    fprintf(stderr, "Malformed Command:\n" );
    fprintf(stderr,
            "Usage: %s %s %s %s %s %s %s %s\n",
            gszProgName, "[-lambda lambda] ","[-mu mu] ","[-r r] ","[-B B] ","[-P P] ","[-n num] ","[-t tsfile]");
    exit(-1);
}

static void SetProgramName(char *s)
{
  char *ptr=strrchr(s, DIR_SEP);
    if (ptr == NULL) 
     {
        strcpy(gszProgName, s);
    } else 
     {
        strcpy(gszProgName, ++ptr);
    }
}

int isintvalue(char *str)
{
   char s[strlen(str)];
   strcpy(s,str);  
   int ans=0;
   int i;
	if(strlen(str)>10)
	ans = 2;
	else if (strlen(str)==10)
	{
		if(s[0]>50 || s[1]>49 || s[2]>52 || s[3]>55|| s[4]>52|| s[5]>56|| s[6]>51 || s[7]>54 || s[8]>52 || s[9]>55) 
		ans =2; 
	}else
	{
		for(i=0;i<strlen(s);i++)
		{
			if(!isdigit(s[i]))
			 { 
			   ans = 1;
			   break;
	    		 }
		}
	}
       	
   
	return ans;
}
bool isvalidnum(char *str)
{
   char s[strlen(str)];
   strcpy(s,str);  
   bool ans=true;
   int i;
	for(i=0;i<strlen(s);i++)
	{
		if(!isdigit(s[i]))
		 { 
		   ans = false;
		   break;
    		 }
	}	
   
	return ans;
}
bool isrealvalue(char *str)
{
   char s[strlen(str)];
   strcpy(s,str); 
   char *part1,*part2; 
   bool ans=true;

   	char *scopy=(char *)malloc(strlen(s)+ 1);;
  	strcpy(scopy,s);
  	char *dotptr = strchr(scopy,'.');
    	int dots=0;
	     while(dotptr != NULL)
		{
			dots++;
			dotptr++;
			scopy = dotptr;
			dotptr = strchr(scopy,'.');
		}
	if(dots > 1)
	{
	 ans = false;
	}else if(dots == 1)
	 {
		strcpy(scopy,s);
		dotptr = strchr(scopy,'.');
		*dotptr = '\0';	
		dotptr++;	
		part1 = scopy;
		part2 = dotptr;
		if(isvalidnum(part1)!=true || isvalidnum(part2)!=true)
		ans = false;		
	 }else if(isvalidnum(str) != true)
	 {	
		ans=false;
   	 }
  
	return ans;
}
double returntime(struct timeval t)
{
  struct timeval diff;
  diff.tv_sec = t.tv_sec -  starttime.tv_sec;
  diff.tv_usec = t.tv_usec - starttime.tv_usec;
  if(diff.tv_usec < 0 )
  {
   diff.tv_usec += 1000000;  
   diff.tv_sec -= 1;
  } 
     return ((diff.tv_sec * 1000000) + diff.tv_usec);
}

void *packetthread(void *arg)
{
        MyData *d = (MyData *)arg;
  	FILE *fp,*fpcopy;
  	fp = fopen((char  *)d->filename,"r");
	fpcopy = fp;
	char buffer[LINE];	
        int i;
	struct timeval currtime;
	double prevtime;
	char str[6];
        int check = 0; 
	bool b;
 	float packetarrivalrate,pt;

	if(fp != NULL)
	{
	   	if(fgets(buffer,sizeof(buffer),fp)!=NULL)
		{
			char *str,*space;
			str = (char *)malloc(strlen(buffer)+1);				
			strcpy(str, buffer);
			space=strchr(str,'\n');
			space++;
			int numpackets;								
			numpackets = atoi(str);
			d->n = numpackets;      	
       		}	
	}else
	{	
	  if(d->lambda < 0.1)
	  {
		d->lambda=0.1;
	  }
	  packetarrivalrate = 1/(d->lambda);	  
	  pt = packetarrivalrate*1000000;		
	}
		          
        char *rateptr;
	char *packetnum;
	char *servicerate;
	int countspaceinline;
	int j=0;
	for(i=0;i < (d->n) ;i++)
	{            
		if(fpcopy!=NULL)
		{
			
		  	if(fgets(buffer,sizeof(buffer),fp)!=NULL)
			{
				countspaceinline=0;
				if(strlen(buffer) >1024)
				{
		  		fprintf(stderr,"Malformed Input : Length of the line exceeds more than 1024 on line %d !\n",i+1);
		  		exit(0);
				}
				char *space_ptr;          
			  	char *string_copy;
				bool var=false;
			  	string_copy = (char *)malloc(strlen(buffer)+ 1);
			  	strcpy(string_copy,buffer);
				
				for(j=0;j<strlen(buffer);j++)
				{
					if(isspace(string_copy[j]) && var!=true)
					{
						var = true;
						countspaceinline++;
					}else if(!isspace(string_copy[j]))
					var =false;
				}	

				if((countspaceinline-1) > 2)
				{
					fprintf(stderr,"Malformed input: More than 3 arguments on line %d\n",i+1);
					exit(0);
				}

				if(string_copy==NULL)
				{
				    fprintf(stderr,"Malformed Input : Input is empty on line number %d !\n",i+1);
				    exit(0);
				}

		  		char *start_str = string_copy;
	  			space_ptr = strchr(start_str,' ');				 
				if(space_ptr!=NULL)
				{
					*space_ptr = '\0';
				  	space_ptr++;
				  	rateptr = start_str;
					b = isvalidnum(rateptr);
					if(b != true)
					{
						fprintf(stderr,"Malformed input: line in %d has no valid value for packetarrivalrate\n",i+1);
						exit(0);
					}
                                  	pt = atof(rateptr) * 1000; 	  
				}
				while(isspace(*space_ptr))
				space_ptr++;

				start_str = space_ptr;
				space_ptr = strchr(start_str,' ');
				
				if(space_ptr!=NULL)
				{
					*space_ptr = '\0';
				  	space_ptr++;
				  	packetnum = start_str;
					b = isvalidnum(packetnum);
					if(b != true)
					{
						fprintf(stderr,"Malformed input: line in %d has no valid value for packets per token\n",i+1);
						exit(0);
					}
                                  	d-> P = atoi(packetnum);
				}
				while(isspace(*space_ptr))
				space_ptr++;

				start_str = space_ptr;
				space_ptr = strchr(start_str,'\n');
				
				if(space_ptr!=NULL)
				{
					*space_ptr = '\0';
				  	space_ptr++;
				  	servicerate = start_str;
					b = isvalidnum(servicerate);
					if(b != true)
					{
						fprintf(stderr,"Malformed input: line in %d has no valid value for packet service rate\n",i+1);
						exit(0);
					}
                                  	d-> mu = atof(servicerate);
					
				}
			        				
			}	
		}  

		
			usleep(pt);								 			 	
  	        	MyPacket *p = (MyPacket *)malloc(sizeof(MyPacket));
			gettimeofday(&currtime,NULL);
			p->systemarrivaltime = currtime;
			p->lambda = d->lambda;
			p->mu = d->mu;
			p->P = d->P;
			p->pnumber = i+1;		
								
			if(p->P <= 1)
			 strcpy(str,"token");
			 else
	 		 strcpy(str,"tokens");

		
	
		if(d->P > d->B){
				if(p->pnumber == 1)
				{
				 	fprintf(stdout,"%012.3fms: p%d arrives, needs %d %s, inter-arrival time = %.6gms, dropped\n", returntime(p->systemarrivaltime)/(double)1000,p->pnumber, d->P,str,returntime(p->systemarrivaltime)/(double)1000);
				}else
				{
					fprintf(stdout,"%012.3fms: p%d arrives, needs %d %s, inter-arrival time = %.6gms, dropped\n", returntime(p->systemarrivaltime)/(double)1000,p->pnumber, d->P,str,returntime(p->systemarrivaltime)/(double)1000 - prevtime/(double)1000);
				}		       		  			
			  d->packetdropcount++;
			  d->packetsarrived++;
			  prevtime = returntime(p->systemarrivaltime);
			pthread_mutex_lock(&m);
			  if(d->packetendcount == ((d->n)-(d->packetdropcount)))
				pthread_cond_broadcast(&cv);
			pthread_mutex_unlock(&m);
			
		}else{	
				pthread_mutex_lock(&m);
				pthread_cleanup_push(pthread_mutex_unlock, &m);	
				if(p->pnumber == 1)
				{
					 fprintf(stdout,"%012.3fms: p%d arrives, needs %d %s, inter-arrival time = %.6gms\n",returntime(p->systemarrivaltime)/(double)1000,p->pnumber, d->P,str,returntime(p->systemarrivaltime)/(double)1000);	
					d->totalinterarrivaltime = d->totalinterarrivaltime + returntime(p->systemarrivaltime);					 
				}else
				{
				 	fprintf(stdout,"%012.3fms: p%d arrives, needs %d %s, inter-arrival time = %.6gms\n",returntime(p->systemarrivaltime)/(1000.00),p->pnumber, d->P,str,returntime(p->systemarrivaltime)/(double)1000 - prevtime/(double)1000);				
					d->totalinterarrivaltime = d->totalinterarrivaltime + returntime(p->systemarrivaltime)-prevtime;				
				}			
				prevtime = returntime(p->systemarrivaltime);
				d->packetsarrived++;	
			
				
			
				gettimeofday(&currtime,NULL);    
				p->q1arrivaltime = currtime;		
				My402ListAppend(&Q1,p);				
				fprintf(stdout,"%012.3fms: p%d enters Q1\n",returntime(p->q1arrivaltime)/(1000.00),p->pnumber);	
				 
				My402ListElem *ptr = My402ListFirst(&Q1);
				MyPacket *first = (MyPacket *)(ptr->obj);
				if(first->P <= counttoken)
				{
					My402ListUnlink(&Q1,ptr);   
					counttoken-= (first->P);
					gettimeofday(&currtime,NULL);
					first->q1departuretime = currtime;
					if(counttoken<=1)
					strcpy(str,"token");
					else
					strcpy(str,"tokens");					
					fprintf(stdout,"%012.3fms: p%d leaves Q1, time in Q1 = %.6gms, token bucket now has %d %s\n",returntime(first->q1departuretime)/(double)1000,first->pnumber,returntime(first->q1departuretime)/(double)1000-returntime(first->q1arrivaltime)/(double)1000,counttoken,str);
					d->timeinQ1 = d->timeinQ1 + returntime(first->q1departuretime) - returntime(first->q1arrivaltime);					
					if(My402ListEmpty(&Q2) && d->packetendcount < ((d->n)-(d->packetdropcount)))
					check = 1;
					else
		 			check = 0;
					
		
					gettimeofday(&currtime,NULL);
					first->q2arrivaltime = currtime;
 					My402ListAppend(&Q2,first);
						
					fprintf(stdout,"%012.3fms: p%d enters Q2\n",returntime(first->q2arrivaltime)/(double)1000,first->pnumber);
					d->packetsenteredQ2++;					
					
					if(check == 1){				
				          pthread_cond_broadcast(&cv);
					} 
												
				}  
			 	
				pthread_mutex_unlock(&m);
			pthread_cleanup_pop(1);	       	
		   }	
	 	
	}		
        return NULL;
}

void *tokenthread(void *arg)
{
	counttoken = 0;
        MyData *d = (MyData *)arg;
	float tokenrate = d->r;
	if(tokenrate < 0.1)
	tokenrate  = 0.1;
	float interarrivaltime,it;
        interarrivaltime = 1/tokenrate;  
	int check=0;
	it = interarrivaltime*1000000;  
	int tokennumber=0;
	char str[6];
	for(;;)
	{
	        tokennumber++;
		usleep(it);
		struct timeval currtime;
		gettimeofday(&currtime,NULL);		
		if((d->packetsenteredQ2 + d->packetdropcount) >= (d->n))
		{
		 break;
		}
		else 
		{
			pthread_mutex_lock(&m);
			pthread_cleanup_push(pthread_mutex_unlock, &m);
			if(counttoken < (d->B))
			{
			 	counttoken++;
				if(counttoken<=1)
				strcpy(str,"token");
				else
				strcpy(str,"tokens"); 								
				fprintf(stdout,"%012.3fms: token t%d arrives, token bucket now has %d %s\n",returntime(currtime)/(double)1000, tokennumber,counttoken,str);
			}else
			{
			   	fprintf(stdout,"%012.3fms: token t%d arrives, dropped\n",returntime(currtime)/(double)1000, tokennumber);	
			  	d->tokendropcount++;	
			}
			d->tokentotalcount++;
		   if(!My402ListEmpty(&Q1))
			{
				My402ListElem *ptr = My402ListFirst(&Q1);
				MyPacket *first = (MyPacket *)(ptr->obj);
				if(first->P <= counttoken)
				{
					My402ListUnlink(&Q1,ptr);  
					
 					counttoken-= (first->P); 		 			
					if(counttoken<=1) 
					strcpy(str,"token");
					else 
					strcpy(str,"tokens");

					gettimeofday(&currtime,NULL);
					first->q1departuretime = currtime;
					fprintf(stdout,"%012.3fms: p%d leaves Q1, time in Q1 = %.6gms, token bucket now has %d %s\n",returntime(first->q1departuretime)/(double)1000,first->pnumber,returntime(first->q1departuretime)/(double)1000-returntime(first->q1arrivaltime)/(double)1000,counttoken,str);
					d->timeinQ1 = d->timeinQ1 + returntime(first->q1departuretime) - returntime(first->q1arrivaltime);								
				       if(My402ListEmpty(&Q2) && d->packetendcount < ((d->n)-(d->packetdropcount)))				
					 check = 1;
					 else
					 check = 0;
					 
					gettimeofday(&currtime,NULL);
					first->q2arrivaltime = currtime;
 					My402ListAppend(&Q2,first);					
					fprintf(stdout,"%012.3fms: p%d enters Q2\n",returntime(first->q2arrivaltime)/(double)1000,first->pnumber);				
					d->packetsenteredQ2++;	
					if(check == 1)
 					pthread_cond_broadcast(&cv); 							
				}	
		       }
			pthread_mutex_unlock(&m);
			pthread_cleanup_pop(1);
		 }
	}	
  return NULL;
}

void *serverthread1(void *arg)
{	
     MyData *d = (MyData *)arg;
     struct timeval currtime;	
	while(1)
	{	
		
		pthread_mutex_lock(&m);
		pthread_cleanup_push(pthread_mutex_unlock, &m);
		
		while(My402ListEmpty(&Q2) && d->packetendcount < ((d->n)-(d->packetdropcount)) && ctrlCcheck != true)   
			{					
				pthread_cond_wait(&cv,&m);
			}
		pthread_cleanup_pop(0);
		
		if(!My402ListEmpty(&Q2))
		   {
			My402ListElem *ptr = My402ListFirst(&Q2);
			MyPacket *first = (MyPacket *)(ptr->obj);
			 	
			My402ListUnlink(&Q2,ptr);
						
			gettimeofday(&currtime,NULL);
			first->q2departuretime = currtime;
			fprintf(stdout,"%012.3fms: p%d leaves Q2, time in Q2 = %.6gms\n",returntime(first->q2departuretime)/(double)1000,first->pnumber,returntime(first->q2departuretime)/(double)1000 - returntime(first->q2arrivaltime)/(double)1000);				
			float servicetime = (first->mu)*1000;
			gettimeofday(&currtime,NULL);
			first->serverarrivaltime = currtime;			
			fprintf(stdout,"%012.3fms: p%d begins service at S1, requesting %.6gms of service\n",returntime(first->serverarrivaltime)/(double)1000,first->pnumber,first->mu);
			pthread_mutex_unlock(&m);
			server1isServicing = true;						
			
			usleep(servicetime);
			pthread_mutex_lock(&m);
			gettimeofday(&currtime,NULL);
			first->serverdeparturetime=currtime;
			fprintf(stdout,"%012.3fms: p%d departs from S1, service time = %.6gms, time in system = %.6gms\n",returntime(first->serverdeparturetime)/(double)1000,first->pnumber,returntime(first->serverdeparturetime)/(double)1000-returntime(first->serverarrivaltime)/(double)1000,returntime(first->serverdeparturetime)/(double)1000-returntime(first->systemarrivaltime)/(double)1000);
			
		
			d->timeinQ2 = d->timeinQ2 + returntime(first->q2departuretime) - returntime(first->q2arrivaltime);		
			d->timeinS1 = d->timeinS1 + returntime(first->serverdeparturetime) - returntime(first->serverarrivaltime);
			d->totaltimeinsystem = d->totaltimeinsystem + returntime(first->serverdeparturetime) - returntime(first->systemarrivaltime);
			double timeinsys = returntime(first->serverdeparturetime) - returntime(first->systemarrivaltime);
			d->totaltimeinsystemsquared = d->totaltimeinsystemsquared + pow(timeinsys/(double)1000000 ,2.0);
			d->packetendcount++;

			server1isServicing = false;
			if(d->packetendcount == ((d->n)-(d->packetdropcount)))
			{	
					pthread_cond_broadcast(&cv);		
			}
		    pthread_mutex_unlock(&m);
		 }
		else{
		  	pthread_mutex_unlock(&m);
			break;
		}					
	}   
   return NULL; 
}

void *serverthread2(void *arg)
{
	MyData *d = (MyData *)arg;
	struct timeval currtime;
    	while(1)   
	{			
		pthread_mutex_lock(&m);
			pthread_cleanup_push(pthread_mutex_unlock, &m);
			while(My402ListEmpty(&Q2) && d->packetendcount < ((d->n)-(d->packetdropcount)) && ctrlCcheck != true)   
				{					
					pthread_cond_wait(&cv,&m);
				}
		pthread_cleanup_pop(0);	
	
		if(!My402ListEmpty(&Q2))
		{	
			My402ListElem *ptr = My402ListFirst(&Q2);
			MyPacket *first = (MyPacket *)(ptr->obj);			 	
			My402ListUnlink(&Q2,ptr);				
			gettimeofday(&currtime,NULL);
			first->q2departuretime= currtime;
			fprintf(stdout,"%012.3fms: p%d leaves Q2, time in Q2 = %.6gms\n",returntime(first->q2departuretime)/(double)1000,first->pnumber,returntime(first->q2departuretime)/(double)1000 - returntime(first->q2arrivaltime)/(double)1000);		
					
			float servicetime = (first->mu)*1000;
			gettimeofday(&currtime,NULL);
			first->serverarrivaltime=currtime;			
			fprintf(stdout,"%012.3fms: p%d begins service at S2, requesting %.6gms of service\n",returntime(currtime)/(double)1000,first->pnumber,first->mu);
			pthread_mutex_unlock(&m);			
			server2isServicing = true;
			
			usleep(servicetime);
			
			pthread_mutex_lock(&m);
				gettimeofday(&currtime,NULL);
				first->serverdeparturetime=currtime;
		
				fprintf(stdout,"%012.3fms: p%d departs from S2, service time = %.6gms, time in system = %.6gms\n",returntime(first->serverdeparturetime)/(double)1000,first->pnumber,returntime(first->serverdeparturetime)/(double)1000-returntime(first->serverarrivaltime)/(double)1000,returntime(first->serverdeparturetime)/(double)1000 - returntime(first->systemarrivaltime)/(double)1000);
			 	
				
			 			 
				d->timeinQ2 = d->timeinQ2 + returntime(first->q2departuretime) - returntime(first->q2arrivaltime);	
			 	d->timeinS2 = d->timeinS2 + returntime(first->serverdeparturetime) - returntime(first->serverarrivaltime);
				d->totaltimeinsystem = d->totaltimeinsystem + returntime(first->serverdeparturetime) - returntime(first->systemarrivaltime);
				double timeinsys = returntime(first->serverdeparturetime) - returntime(first->systemarrivaltime);
				d->totaltimeinsystemsquared = d->totaltimeinsystemsquared + pow(timeinsys/(double)1000000 ,2.0);
			 	d->packetendcount++;
				
				server2isServicing = false;
					
				if(d->packetendcount == ((d->n)-(d->packetdropcount)))
				{	
					pthread_cond_broadcast(&cv);		
				}
		        pthread_mutex_unlock(&m); 
		}
		else{
			pthread_mutex_unlock(&m);
			break;
		}			      
	}   
	return NULL;   
}

void *ctrlCthread(void *arg)
{	
	struct timeval currtime;
	int sig = 0;
	sigwait(&set,&sig);    
 	pthread_mutex_lock(&m);	
	      	ctrlCcheck = true;
		pthread_cond_broadcast(&cv);  
	pthread_mutex_unlock(&m);
	
	pthread_cancel(thr[0]); 
	pthread_cancel(thr[1]);  
  	fprintf(stdout,"\n");
	while(!My402ListEmpty(&Q1))
	{
		My402ListElem *ptr = My402ListFirst(&Q1);
		MyPacket *first = (MyPacket *)(ptr->obj);			 	
		My402ListUnlink(&Q1,ptr);
		gettimeofday(&currtime,NULL);	
		fprintf(stdout,"%012.3fms: p%d dropped from Q1\n",returntime(currtime)/(double)1000,first->pnumber);
	}
	while(!My402ListEmpty(&Q2))
	{
		My402ListElem *ptr = My402ListFirst(&Q2);
		MyPacket *first = (MyPacket *)(ptr->obj);			 	
		My402ListUnlink(&Q2,ptr);
		gettimeofday(&currtime,NULL);	
		fprintf(stdout,"%012.3fms: p%d dropped from Q2\n",returntime(currtime)/(double)1000,first->pnumber);
	}	
	while(server1isServicing==true || server2isServicing==true);								
	pthread_cancel(thr[2]);
	pthread_cancel(thr[3]);	
 return NULL;
}
int main(int argc, char *argv[])
{

SetProgramName(*argv);    
int l;
if(argc < 3)
{
	l = 1;
	if(!(strcmp(argv[l],"-lambda") == 0 || strcmp(argv[l],"-mu") == 0 || strcmp(argv[l],"-r") == 0 ||  strcmp(argv[l],"-B") == 0 || 
strcmp(argv[l],"-n") == 0 || strcmp(argv[l],"-P") == 0 || strcmp(argv[l],"-t") == 0 ) )
		{
			 Usage();
		}
  fprintf(stderr,"Malformed input: very few arguments provided\n");
	exit(0); 
}
if (argc > 15)
    {
 	fprintf(stderr,"Malformed input: more than allowed arguments provided\n");
	exit(0);      
    }

MyData* data = (MyData*)malloc(sizeof(MyData));
data->n = 20;
data->r = 1.5;
data->lambda = 1;
data->mu = 0.35;
data->B = 10;
data->P = 3;
int nofile=1;
   if(data==NULL)
	{
 	  fprintf(stderr,"Error Message : Insufficient memory, structure couldn't be initialised !\n");
	  exit(0);
    	}
bool check;
int intcheck;
for(l=1;l<argc;l+=2)
{	if(argc%2 ==0 && l==(argc-1))
	{
	  if(strcmp(argv[l],"-lambda") == 0 || strcmp(argv[l],"-mu") == 0 || strcmp(argv[l],"-r") == 0 ||  strcmp(argv[l],"-B") == 0 || 
strcmp(argv[l],"-n") == 0 || strcmp(argv[l],"-P") == 0 || strcmp(argv[l],"-t") == 0 )
		{
			 Usage();
		}
        }
   	if(strcmp(argv[l],"-lambda") == 0)
   	{		
		check = isrealvalue(argv[l+1]);
		if(check != true)
		{
		  fprintf(stderr,"Malformed input : improper lambda value !\n");
		  exit(0);
		}
		data->lambda = atof(argv[l+1]);
		if(data->lambda < 0.1)
		{			
			data->lambda = 0.1; 
		}
   	}else if(strcmp(argv[l],"-mu") == 0)
	{
		check = isrealvalue(argv[l+1]);
		if(check != true)
		{
		  fprintf(stderr,"Malformed input : improper mu value !\n");
		  exit(0);
		}
		data->mu = atof(argv[l+1]);
		if(data->mu < 0.1)
		{
			 data->mu =0.1;
		}
	}else if(strcmp(argv[l],"-r") == 0)
	{
		check = isrealvalue(argv[l+1]);
		if(check != true)
		{
		  fprintf(stderr,"Malformed input : improper r value !\n");
		  exit(0);
		}
		data->r = atof(argv[l+1]);
		if(data->r < 0.1)
		{
			 data->r =0.1;
		}
	}else if(strcmp(argv[l],"-B") == 0)
	{
		intcheck = isintvalue(argv[l+1]);
		if(intcheck == 1)
		{
		  fprintf(stderr,"Malformed input : improper bucket size B value, must be an integer !\n");
		  exit(0);
		}else if(intcheck ==2 )
		{
		  fprintf(stderr,"Malformed input :  B value has more than max size allowed for integer !\n");
		  exit(0);
		}
		data->B = atoi(argv[l+1]);
	}else if(strcmp(argv[l],"-P") == 0)
	{
		intcheck = isintvalue(argv[l+1]);
		if(intcheck == 1)
		{
		  fprintf(stderr,"Malformed input : improper packets per token P value, must be an integer !\n");
		  exit(0);
		}else if(intcheck ==2 )
		{
		  fprintf(stderr,"Malformed input :  P value has more than max size allowed for integer !\n");
		  exit(0);
		}	
		data->P = atoi(argv[l+1]);
	}else if(strcmp(argv[l],"-n") == 0)
	{
		intcheck = isintvalue(argv[l+1]);
		if(intcheck == 1)
		{
		  fprintf(stderr,"Malformed input : improper pnumber of packets n value, must be an integer !\n");
		  exit(0);
		}else if(intcheck ==2 )
		{
		  fprintf(stderr,"Malformed input :  n value has more than max size allowed for integer !\n");
		  exit(0);
		}	
		data->n = atoi(argv[l+1]);	
	}else if(strcmp(argv[l],"-t") == 0)
	{			
		strcpy((char *)data->filename, argv[l+1]);
		FILE *fp;
		char buffer[LINE];
  		fp = fopen((char  *)data->filename,"r");
		int err_num;
		struct stat statbuf;

		    stat(argv[l+1], &statbuf);
		    
		    if (S_ISDIR (statbuf.st_mode)) 
		    {
		     fprintf (stdout,"Malformed Input : %s is a directory !\n",argv[2]);
		     exit(0);
		    }

		if(fp != NULL)
		{
		   	if(fgets(buffer,sizeof(buffer),fp)!=NULL)
			{
				char *str,*space;
				str = (char *)malloc(strlen(buffer)+1);				
				strcpy(str, buffer);
				space=strchr(str,'\n');
				*space = '\0';
				space++;								
				if(isvalidnum(str) != true)
				{
					fprintf(stderr,"Malformed input : line 1 is not just a number in the file\n");
					exit(0);
				}
				int numpackets;
				str = (char *)malloc(strlen(buffer) + 1);
				strcpy(str, buffer);
				numpackets = atoi(str);
				data->n = numpackets;      	
	       		}
			nofile=0;	
		}else
	        {
			err_num = errno;
			fprintf(stderr, "Malformed Input : Could not open the file %s !\n",strerror(err_num));
			exit(0);
		}
			
		fclose(fp);
	}
	else 
		Usage();

}


memset(&Q1, 0, sizeof(My402List));
memset(&Q2, 0, sizeof(My402List));

if(My402ListInit(&Q1) == 0 || My402ListInit(&Q2) == 0)
{
  fprintf(stderr,"Error Message : Insufficient memory, node couldn't be intialized !\n");
  exit(0);
}

int i;
int err;

sigemptyset(&set);
sigaddset(&set, SIGINT);
sigprocmask(SIG_BLOCK, &set, 0);    

data->packetsenteredQ2 = 0;
ctrlCcheck = false;
data->packetendcount=0;
data->packetdropcount=0;
data->tokentotalcount=0;
data->tokendropcount=0;
data->packetsarrived=0;
data->timeinQ1=0.0;
data->timeinQ2=0.0;
data->timeinS1=0.0;
data->timeinS2=0.0;
data->totalinterarrivaltime=0.0;
data->totaltimeinsystem=0.0;
data->totaltimeinsystemsquared=0.0;

fprintf(stdout,"Emulation Parameters:\n");
fprintf(stdout,"    number to arrive = %d\n",data->n);
if(nofile==1)
fprintf(stdout,"    lambda = %.6g\n",data->lambda);
if(nofile==1)
fprintf(stdout,"    mu = %.6g\n",data->mu);
fprintf(stdout,"    r = %.6g\n",data->r);
fprintf(stdout,"    B = %d\n",data->B);
if(nofile==1)
fprintf(stdout,"    P = %d\n",data->P);
if(nofile==0)
fprintf(stdout,"    tsfile = %s\n",(char *)data->filename);
fprintf(stdout,"\n");
gettimeofday(&starttime,NULL);
fprintf(stdout,"00000000.000ms: emulation begins\n");

err = pthread_create(&thr[0],0,packetthread,(void *)data);
if(err != 0)
{
fprintf(stderr,"pthread error : %s", strerror(err));
exit(1);	   		
}
err = pthread_create(&thr[1],0,tokenthread,(void *)data);
if( err!=0 )
{
   fprintf(stderr,"pthread error : %s", strerror(err));
      exit(1);
}
err = pthread_create(&thr[2],0,serverthread1,(void *)data);
if( err!=0 )
{
   fprintf(stderr,"pthread error : %s", strerror(err));
      exit(1);
}
err = pthread_create(&thr[3],0,serverthread2,(void *)data);
if( err!=0 )
{
   fprintf(stderr,"pthread error : %s", strerror(err));
      exit(1);
}
err = pthread_create(&ctrlthread,0,ctrlCthread,(void *)data);
if(err != 0)
{
 fprintf(stderr,"pthread error : %s", strerror(err));
      exit(1);
}


for(i=0;i<threads;i++)
pthread_join(thr[i],0);

struct timeval endtime;
gettimeofday(&endtime,NULL);
long emulationtime = returntime(endtime);
fprintf(stdout,"%012.3fms: emulation ends\n",emulationtime/(1000.0));

data->totalservicetime = (data->timeinS1 + data->timeinS2);
int packetservicedcount = (data->n)-(data->packetdropcount);
	
	fprintf(stdout,"\n");
	fprintf(stdout,"Statistics:\n");
	fprintf(stdout,"\n");
	if(data->packetsarrived!=0)
	fprintf(stdout,"    average packet inter-arrival time = %.6gs\n",((double)data->totalinterarrivaltime)/(data->packetsarrived * (double)1000000));
	else
	fprintf(stdout,"    average packet inter-arrival time = N/A no packets arrived at this facility\n");
	if(packetservicedcount != 0)
	fprintf(stdout,"    average packet service time = %.6gs\n",((double)data->totalservicetime)/(packetservicedcount * (double)1000000));
	else
	fprintf(stdout,"    average packet service time = N/A no packets serviced in the server\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"    average number of packets in Q1 = %.6g\n",((double)data->timeinQ1)/emulationtime);
	fprintf(stdout,"    average number of packets in Q2 = %.6g\n",((double)data->timeinQ2)/emulationtime);
	fprintf(stdout,"    average number of packets at S1 = %.6g\n",((double)data->timeinS1)/emulationtime);
	fprintf(stdout,"    average number of packets at S2 = %.6g\n",((double)data->timeinS2)/emulationtime);
	fprintf(stdout,"\n");
	if(packetservicedcount != 0)
	{
		double variance,part1,part2,part3,stddev;
		part1 = (data->totaltimeinsystemsquared)/(packetservicedcount);
  
		part2  = (data->totaltimeinsystem)/(packetservicedcount * (double)1000000);   		
		part3 = pow(part2,2.0); 
 
		variance = fabs(part1 - part3);   
		stddev = sqrt(variance);

	fprintf(stdout,"    average time a packet spent in system = %.6gs\n",part2);
	fprintf(stdout,"    standard deviation for time spent in system = %.6gs\n",stddev);
	}else
	{
	fprintf(stdout,"    average time a packet spent in system = N/A no packets serviced in the server\n");
	fprintf(stdout,"    standard deviation for time spent in system = N/A no packets serviced in the server\n");	
	}
	fprintf(stdout,"\n");
	if(data->tokentotalcount !=0)
	fprintf(stdout,"    token drop probability = %.6g\n",((double)data->tokendropcount)/((double)data->tokentotalcount));
	else
	fprintf(stdout,"    token drop probability = N/A no tokens arrived at this facility\n");
	if(data->packetsarrived!=0)
	fprintf(stdout,"    packet drop probability = %.6g\n",((double)data->packetdropcount)/((double)data->packetsarrived));
	else
	fprintf(stdout,"    packet drop probability = N/A no packets arrived at this facility\n");

free(data);
return 0;
}
