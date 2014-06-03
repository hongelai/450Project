#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "common.h"
#define SERVERPORT_PHASE1 1552
#define PORT_MY_ID_LAST_3_DIG 452
#define PORT_PEER_UDP_BASE 3000
#define PORT_OFFSET_PER_PEER 100
#define MY_UDP_PORT(bidderID) (PORT_PEER_UDP_BASE+PORT_OFFSET_PER_PEER*bidderID+PORT_MY_ID_LAST_3_DIG)
#define PORT_PEER_TCP_BASE 4000
#define MY_TCP_PORT(bidderID) (PORT_PEER_TCP_BASE+PORT_OFFSET_PER_PEER*bidderID+PORT_MY_ID_LAST_3_DIG)
#define MAXBUFLEN 100
int bidderID =1;
char loginBuffer[70]="";

int main(){

    int numbytes;
    int TCP_sockfd;
	struct sockaddr_in my_TCP_addr;// 
	struct sockaddr_in their_addr; //server
    struct hostent *he;
	int len = sizeof(struct sockaddr_in);
    
    if ((he=gethostbyname("localhost")) == NULL) {  
		perror("gethostbyname");
		exit(1);
	}
  	/* fork 2 bidders*/
	while(bidderID<2){
		if( fork() == 0 )
			bidderID++;
		else
			break;
		
	}
	/*****************************************
	*            PHASE1
	*****************************************/
	/* Create a TCP socket and get a port number from the kernel*/
	if ((TCP_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("TCPsocket");
		exit(1);
	}

	my_TCP_addr.sin_family = AF_INET;		 // host byte order
	my_TCP_addr.sin_port = INADDR_ANY;
	my_TCP_addr. sin_addr. s_addr = inet_addr( "127.0.0.1") ;
    //my_TCP_addr.sin_addr = *((struct in_addr *)he->h_addr); // automatically fill with my IP
	memset(&(my_TCP_addr.sin_zero), '\0', 8); // zero the rest of the struct

	/*bind TCP port assigned by kernel*/
	if (bind(TCP_sockfd, (struct sockaddr *)&my_TCP_addr,
		sizeof(struct sockaddr)) == -1) {
		perror("bind");
		exit(1);
	}

	/*get the socket info assigned by kernel*/
	if(getsockname(TCP_sockfd,(struct sockaddr *)&my_TCP_addr,&len )<0){
		perror("getsockname");
		exit(1);
	}
    
	if ((he=gethostbyname("localhost")) == NULL) {  
		perror("gethostbyname");
		exit(1);
	}
	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(SERVERPORT_PHASE1);
    //their_addr. sin_addr. s_addr = inet_addr( "127.0.0.1") ;
	their_addr.sin_addr =  *((struct in_addr *)he->h_addr);
	memset(&(their_addr.sin_zero), '\0', 8); 
    printf("<Bidder %d> has TCP port %d  and IP address%s\n",bidderID,(int) ntohs(my_TCP_addr.sin_port),inet_ntoa(my_TCP_addr.sin_addr));
       
	if (connect(TCP_sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
	    perror("connect");
	    exit(1);
	}   
    char ch[1],buffer[60];
    strcat(loginBuffer,"LOGIN#");
    itostr(ch,bidderID);
    char filename[20] ="bidderPass";
    strcat(filename,ch);
    strcat(filename,".txt");
    FILE * file = fopen(filename,"r");
    fscanf(file, "%[^\n]\n",buffer);
    char userName[15],passWord[15],BAccount[20];
    getStrSegment(buffer, userName,2);
    getStrSegment(buffer, passWord,3);
    getStrSegment(buffer, BAccount,4);
    printf("Login request. User#: %d Username:%s  Password:%s  Bank Account:%s \n",bidderID,userName,passWord,BAccount);
    strcat(loginBuffer,buffer);

    fclose(file);
	if (send(TCP_sockfd, loginBuffer, strlen(loginBuffer), 0) == -1)
                perror("send");    
    memset(loginBuffer, '\0',70); 
	if ((numbytes=recv(TCP_sockfd, loginBuffer, 19, 0)) == -1) {
	        perror("ack");
	        exit(1);
	}
    printf("Login request reply:%s",loginBuffer);
    
    close(TCP_sockfd);
    memset(buffer, '\0',60);
    printf("End of Phase 1 for <Bidder%d>.\n",bidderID);
    
/************************************************************************
                             PHASE3
*************************************************************************/
	int UDP_sockfd_P3,client_fd;
	int TCP_sockfd_P3;
	int new_fd_P3;
	socklen_t udp_len = sizeof(struct sockaddr);
	struct sockaddr_in my_UDP_addr_P3;	// my address information
	struct sockaddr_in my_TCP_addr_P3;	// my address information
	struct sockaddr_in their_addr_UDP; // connector's address information
    struct sockaddr_in their_addr_TCP; 
	struct hostent *he_P3;
	int i,j;
    char buffer_udp[MAXBUFLEN],line[100];
	socklen_t sin_size;

	/*open the UDP*/
	if ((UDP_sockfd_P3 = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("UDPsocket");
		exit(1);
	}

	/*create a UDP socket and bind a dedicated port*/
	
	my_UDP_addr_P3.sin_family = AF_INET;		 // host byte order
	my_UDP_addr_P3.sin_port = htons(MY_UDP_PORT(bidderID));	 // short, network byte order
	//my_UDP_addr_P3.sin_addr.s_addr = inet_addr( "127.0.0.1") ;
	my_UDP_addr_P3.sin_addr =  *((struct in_addr *)he->h_addr);
	memset(&(my_UDP_addr_P3.sin_zero), '\0', 8); // zero the rest of the struct
    printf("<Bidder %d> has UDP port %d and IP address: %s\n",bidderID,MY_UDP_PORT(bidderID),inet_ntoa(my_UDP_addr_P3.sin_addr));
	if (bind(UDP_sockfd_P3, (struct sockaddr *)&my_UDP_addr_P3,
		sizeof(struct sockaddr)) == -1) {
		perror("bind");
		exit(1);
	}    
 	if ((numbytes = recvfrom(UDP_sockfd_P3, buffer_udp, MAXBUFLEN-1 , 0,
			(struct sockaddr *)&their_addr_UDP, &udp_len)) == -1) {
			perror("recvfrom");
			exit(1);
	}
	buffer_udp[numbytes] = '\0';   
    printf("Received broadcast list:\n");
    //read broadcast items
    FILE* fp_p3 = fopen(buffer_udp,"r");
    if(!fp_p3) printf("open error");
    fseek(fp_p3,0,SEEK_SET);
    while (fscanf(fp_p3, "%[^\n]\n",line) != EOF) {
        printf("%s\n", line);
        memset(line,'\0',100);
    }
    fclose(fp_p3);
    memset(buffer_udp,'\0',MAXBUFLEN);
    
    char biddingfile[20] ="bidding";
    char ch_p3[3];
    itostr(ch_p3,bidderID);
    strcat(biddingfile,ch_p3);
    strcat(biddingfile,".txt");

    if ((numbytes = sendto(UDP_sockfd_P3, biddingfile, strlen("bidding1.txt"), 0,
         (struct sockaddr *)&their_addr_UDP, sizeof(struct sockaddr))) == -1) {
        perror("sendto");
        exit(1);
    }    

    printf("<Bidder %d>\n",bidderID);
    FILE* fp_p3_bidding = fopen(biddingfile,"r");
    if(!fp_p3_bidding) printf("open error");
    fseek(fp_p3_bidding,0,SEEK_SET);
    while (fscanf(fp_p3_bidding, "%[^\n]\n",line) != EOF)
    {
        printf("%s\n", line);
        memset(line,'\0',100);
    }
    fclose(fp_p3_bidding);
    close(UDP_sockfd_P3);
    sleep(2);
/*************establish TCP to get bidding result***************/

	if ((TCP_sockfd_P3 = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("TCPsocket");
		exit(1);
	}

	my_TCP_addr_P3.sin_family = AF_INET;		 // host byte order
	my_TCP_addr_P3.sin_port = htons(MY_TCP_PORT(bidderID));
	//my_TCP_addr_P3.sin_addr.s_addr = inet_addr( "127.0.0.1") ;
	my_TCP_addr_P3.sin_addr =  *((struct in_addr *)he->h_addr);
	memset(&(my_TCP_addr_P3.sin_zero), '\0', 8); // zero the rest of the struct

    //printf("tcp:%d ,udp: %d\n",MY_TCP_PORT(bidderID),MY_UDP_PORT(bidderID));
    
	//bind TCP port assigned by kernel
	if (bind(TCP_sockfd_P3, (struct sockaddr *)&my_TCP_addr_P3,
		sizeof(struct sockaddr)) == -1) {
		perror("bind");
		exit(1);
	}
	socklen_t addr_len = sizeof(struct sockaddr_in);
    //keep listening
    if (listen(TCP_sockfd_P3, 2) == -1) {
        perror("listen");
        exit(1);
    }
    char buf_P3[1024];
    while(1){
    
        if ((client_fd = accept(TCP_sockfd_P3, (struct sockaddr *)&their_addr_TCP,&addr_len)) == -1) {
            perror("accept");
        } 
		if ((numbytes = recv(client_fd, buf_P3, 1024, 0)) == -1) {
			perror("recvfrom");
			exit(1);
		}

		buf_P3[numbytes] = '\0';
        strPresentHandle(buf_P3);
        printf("Bidding result to bidder%d: \n",bidderID);
        printf("%s\n",buf_P3);
        memset(buf_P3,'\0',1024);
    }
    printf("End of Phase 3 for <Bidder %d>.\n",bidderID);
    return 0;
}
