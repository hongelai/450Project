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
#define SERVERPORT_PHASE2 1652
#define PORT_MY_ID_LAST_3_DIG 452
#define PORT_OFFSET_PER_PEER 100
#define PORT_PEER_TCP_BASE 2000
#define MY_TCP_PORT(sellerID) (PORT_PEER_TCP_BASE+PORT_OFFSET_PER_PEER*sellerID+PORT_MY_ID_LAST_3_DIG)
#define MAXBUFLEN 100

int sellerID =1;
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
  	/* fork 2 sellers*/
	while(sellerID<2){
		if( fork() == 0 )
			sellerID++;
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
	//my_TCP_addr. sin_addr. s_addr = inet_addr( "127.0.0.1") ;
    my_TCP_addr.sin_addr = *((struct in_addr *)he->h_addr); // automatically fill with my IP
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
    printf("<Seller %d> has TCP port %d  and IP address%s\n",sellerID,(int) ntohs(my_TCP_addr.sin_port),inet_ntoa(my_TCP_addr.sin_addr));
       
	if (connect(TCP_sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
	    perror("connect");
	    exit(1);
	}   
    char ch[3],buffer[60];
    strcat(loginBuffer,"LOGIN#");
    itostr(ch,sellerID);
    char filename[20] ="sellerPass";
    strcat(filename,ch);
    strcat(filename,".txt");
    FILE * file = fopen(filename,"r");
    fscanf(file, "%[^\n]\n",buffer);
    char userName[15],passWord[15],BAccount[20];
    getStrSegment(buffer, userName,2);
    getStrSegment(buffer, passWord,3);
    getStrSegment(buffer, BAccount,4);
    printf("Login request. User#: %d Username:%s  Password:%s  Bank Account:%s \n",sellerID,userName,passWord,BAccount);
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
    printf("Auction Server has IP Address %s and PreAuction TCP Port Number %d \n",inet_ntoa(their_addr.sin_addr),SERVERPORT_PHASE2);
    close(TCP_sockfd);
    memset(buffer, '\0',60);
    printf("End of Phase 1 for <Seller%d>.\n",sellerID);
    sleep(5);
 /*************************************************************************
                            phase2
***************************************************************************/
    int TCP_sockfd_p2;
    struct sockaddr_in my_TCP_addr_p2;// 
	struct sockaddr_in their_addr_p2; //server
    struct hostent *he_p2;
    
	if ((TCP_sockfd_p2 = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("TCPsocket");
		exit(1);
	}
	my_TCP_addr_p2.sin_family = AF_INET;		 // host byte order
	my_TCP_addr_p2.sin_port = INADDR_ANY;
	//my_TCP_addr_p2. sin_addr. s_addr = inet_addr( "127.0.0.1") ;
    my_TCP_addr_p2.sin_addr = *((struct in_addr *)he->h_addr); // automatically fill with my IP
	memset(&(my_TCP_addr_p2.sin_zero), '\0', 8); // zero the rest of the struct

	/*bind TCP port assigned by kernel*/
	if (bind(TCP_sockfd_p2, (struct sockaddr *)&my_TCP_addr_p2,
		sizeof(struct sockaddr)) == -1) {
		perror("bind");
		exit(1);
	}

	/*get the socket info assigned by kernel*/
	if(getsockname(TCP_sockfd_p2,(struct sockaddr *)&my_TCP_addr_p2,&len )<0){
		perror("getsockname");
		exit(1);
	}
    
	if ((he=gethostbyname("localhost")) == NULL) {  
		perror("gethostbyname");
		exit(1);
	}
	their_addr_p2.sin_family = AF_INET;
	their_addr_p2.sin_port = htons(SERVERPORT_PHASE2);
    //their_addr_p2. sin_addr. s_addr = inet_addr( "127.0.0.1") ;
	their_addr_p2.sin_addr =  *((struct in_addr *)he->h_addr);
	memset(&(their_addr_p2.sin_zero), '\0', 8); 
    
	if (connect(TCP_sockfd_p2, (struct sockaddr *)&their_addr_p2, sizeof(struct sockaddr)) == -1) {
	    perror("connect");
	    exit(1);
	}   
    printf("Auction Server has IP Address %s and PreAuction TCP Port Number %d \n",inet_ntoa(their_addr_p2.sin_addr),SERVERPORT_PHASE2);
    char buffer_p2[60],itemfilename[20] ="itemList";
    char ch_p2[3];
    itostr(ch_p2,sellerID);
    strcat(itemfilename,ch_p2);
    strcat(itemfilename,".txt");
    FILE * itemfile = fopen(itemfilename,"r");
    if (send(TCP_sockfd_p2, itemfilename, strlen("itemList1.txt"), 0) == -1)
                perror("send");

    printf("<Seller %d> send item lists:\n",sellerID);
    while (fscanf(itemfile, "%[^\n]\n",buffer_p2) != EOF)
    {
            printf("%s\n", buffer_p2);
            memset(buffer_p2,'\0',60);
    }
    fclose(itemfile);
    printf("End of Phase 2 for <Seller%d>.\n",sellerID);
/************************************************************************************
                                PHASE3
*************************************************************************************/
    int TCP_sockfd_p3,client_fd;
    struct sockaddr_in my_TCP_addr_p3;// 
	struct sockaddr_in their_addr_p3; //server

	if ((TCP_sockfd_p3 = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	my_TCP_addr_p3.sin_family = AF_INET;		 
	my_TCP_addr_p3.sin_port = htons(MY_TCP_PORT(sellerID));	 
	my_TCP_addr_p3.sin_addr.s_addr = inet_addr( "127.0.0.1"); //using local host IP address
	memset(&(my_TCP_addr_p3.sin_zero), '\0', 8);     
 
    /*binds the TCP socket*/
	if (bind(TCP_sockfd_p3, (struct sockaddr *)&my_TCP_addr_p3,sizeof(struct sockaddr)) == -1) {
		perror("bind");
		exit(1);
	}

    //keep listening
    if (listen(TCP_sockfd_p3, 2) == -1) {
        perror("listen");
        exit(1);
    }
    char buf_P3[1024];
    while(1){
    
        if ((client_fd = accept(TCP_sockfd_p3, (struct sockaddr *)&their_addr_p3,&len)) == -1) {
            perror("accept");
        } 
		if ((numbytes = recv(client_fd, buf_P3, 1024 , 0)) == -1) {
			perror("recvfrom");
			exit(1);
		}

		buf_P3[numbytes] = '\0';
        strPresentHandle(buf_P3);
        printf("Bidding result to seller%d: \n",sellerID);
        printf("%s\n",buf_P3);
        memset(buf_P3,'\0',1024);
    }
    printf("End of Phase 3 for <Seller%d>.\n",sellerID);
    return 0;
}
