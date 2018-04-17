#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<error.h>
#include<arpa/inet.h>
#include<netdb.h>
#define MAX 653320
//Team Members Rahul Shetty, Bhavneet Singh, Ishan Khajuria
void proxy_to_rserver(int s);
int main(int arg,char *argc[])
{
	int sock_serv,sock_cli;
	socklen_t cli_len,serv_len;
	size_t l;
	struct sockaddr_in serv,client;
	
	if(arg < 2)
	{
		fprintf(stderr,"Error! Please enter the port");
		exit(1);
	}
	sock_serv = socket(AF_INET,SOCK_STREAM,0); //Creating a socket for the incoming request.

	if(sock_serv == -1)
		{
			perror("socket");
			exit(-1);
		}

	memset(&serv,0,sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_addr.s_addr = INADDR_ANY;
	serv.sin_port = htons(atoi(argc[1]));

	printf("Socket created:\n");
	int bi = bind(sock_serv,(struct sockaddr*)&serv,sizeof(serv)); // Binding the socket.
	if(bi < 0)
		{	
			perror("bind:");
			exit(-1);
		}
	printf("Bind successful\n");
	if(listen(sock_serv,6) == -1) //Listening on the port for incoming request and only 6 requests can wait in queue
	{
		perror("listen:");
		exit(-1);
	}

	cli_len = sizeof(client);
	
	again:
	sock_cli = accept(sock_serv,(struct sockaddr*) &client,&cli_len); //Accepting an incoming request.
	if(sock_cli == -1)
		{
			perror("accept");
			exit(-1);		
		}

	proxy_to_rserver(sock_cli);
	
	printf(" \n ---------Waiting for next connection----------------- \n");
	
	goto again;
	return 0;
}

void proxy_to_rserver(int in_cli)
{
	
	char buffer[MAX],host_str1[MAX],serv_response[9999999];
	int i,j,ti,socket_remote,k=0;
	size_t r;
  	struct addrinfo  *hserv,hints;
	socklen_t cli_len,serv_len;
	struct sockaddr_in client,serv_remote;
 
 
	printf("client connected:\n");
	memset(buffer,0,sizeof(buffer));
	while((r = read(in_cli,buffer,MAX))>0)
	{
		if(r>0)
		{
			printf("--------Incoming Request-----------\n");
			printf("%s",buffer);
		}
		
		memset(host_str1,0,strlen(host_str1));
		int tilda =0;
		
		/* Code for parsing */
		
		for(i=0;i<sizeof(buffer);i++)
		{
			if(buffer[i] == '/'&& buffer[i+1] == '/')
			{
				k=0;
				k=i+2;
				for(j=0;buffer[k] !='/';j++)
				{
					host_str1[j]=buffer[k];

					printf("%c \n",host_str1[j]);
					k++;
				}
				tilda ++;
			}

			if (tilda == 1)
			goto label1;
		}
    		
		label1:
		printf("Parsed URL : %s,\n",host_str1);
		
		/* Code for communicating with remote server */
		
		hints.ai_family = INADDR_ANY;
		hints.ai_socktype = SOCK_STREAM;		
    		if((ti=getaddrinfo(host_str1,"80",&hints,&hserv))!= 0)
			{
				herror("Getaddrinfo:");
				exit(-1);
			}


		
		socket_remote = socket(AF_INET,SOCK_STREAM,0); //Socket creation for communication between proxy and remote server
		if(socket_remote<0)
			{
				perror("socket:");
				exit(-1);
			}
		
		memset(&serv_remote,0,sizeof(serv_remote));
		serv_remote.sin_family = AF_INET;
		serv_remote.sin_port = htons(80);
	
		if(connect(socket_remote,(struct sockaddr *) hserv->ai_addr,hserv->ai_addrlen) < 0) //Connection with remote server
			{
				perror("connect:");
				exit(-1);
			}

		puts("Connected to Remote Server!\n");

		if(send(socket_remote,buffer,strlen(buffer),0) < 0) //Sending request to fetch the page from web server
			{
				perror("send:");
				exit(-1);
			}

		memset(buffer,0,sizeof(buffer));
		puts("Data sent to remote server \n");

		int byte_count,byte_sent; 
		memset(serv_response,0,sizeof(serv_response));
		do
		{
			printf("----In do loop----- \n");
			byte_count = recv(socket_remote,serv_response,sizeof(serv_response),0);
			printf("Byte Count:%d \n",byte_count);	
			if(!(byte_count <= 0))
			{	
				printf("Data at console \n");
				byte_sent = send(in_cli,serv_response,byte_count,0);
				printf("%d bytes of data sent to browser \n",byte_sent);
			}
		}while(byte_count > 0);
		   
		puts("Data sent to browser \n");
		close(socket_remote);	
	}		
}
