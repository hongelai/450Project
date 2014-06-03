#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "common.h"
#define PORT_PHASE1 1552	// the port users will be connecting to
#define PORT_PHASE2 1652
#define SELLER1_TCP_PORT 2552
#define SELLER2_TCP_PORT 2652
#define BIDDER1_TCP_PORT 4552
#define BIDDER2_TCP_PORT 4652
#define BIDDER1_UDP_PORT 3552
#define BIDDER2_UDP_PORT 3652
#define MAXBUFLEN 100
char BidderIDchecked[2];//check how many peers are already registered
char SellerIDchecked[2];//check how many providers are already checked.
char AcceptedPeer[4][15];
int AcceptedPeerNum=0;
void init(void)
{
	memset(BidderIDchecked, '\0',2); 
	memset(SellerIDchecked, '\0',2); 
}

/********************************************
int LoginProcedure(char* buf)
func: check the username , password and bankaccount of the user
para:
	buf, the whole message received from providers.
return:
	if authentication is sueccess return 1, else return 0
********************************************/
int LoginProcedure(char* buf)
{

    FILE *fp = fopen("Registration.txt", "r");
	char tmpStr[15];
	char tmpStr2[16];
    char tmpStr3[10];
    char buffer[100];
        
	getStrSegment(buf, tmpStr,2);// get the provider's username
	getStrSegment(buf, tmpStr2,3);// get the password
    getStrSegment(buf, tmpStr3,4);// get the bank Account
    if (fp != NULL) {

        while (fscanf(fp, "%[^\n]\n",buffer) != EOF) {
            char userName[15]="";
            char password[16]="";
            char bAccount[15]="";  
            
            getStrSegment(buffer, userName,1);// read a username
            getStrSegment(buffer, password,2);// read a password
            getStrSegment(buffer, bAccount,3);// read a bank Account
            char account[10];
            int i;
            for(i=0; i<9;i++) account[i]=bAccount[i];
            account[i]='\0';
            memset(buffer,'\0',100);
            
            if(strcmp(userName,tmpStr) == 0){
                if(strcmp(password,tmpStr2) == 0){
                    if(strcmp(account,tmpStr3) == 0){
                        strcpy(AcceptedPeer[AcceptedPeerNum++],userName);
                        fclose(fp);
                        return 1;
                    }
                }
            }//if
        }//whlile
    fclose(fp);
	return 0;
    }
}
/********************************************
int PEERs_All_Registered(void)
func: check if all peers are registered.
return:
	if all peers are registered, return 1, else return 0
*********************************************/
int All_Registered(void)
{

	int i;
	for(i=0;i<2;i++){
		if(BidderIDchecked[i] == '\0')
			return 0;
	}
    for(i=0;i<2;i++){
		if(SellerIDchecked[i] == '\0')
			return 0;
	}
	printf("Registration completed!\n");
	return 1;
}
/********************************************
int checkType(char *buf)
func: check if it is a bidder or seller
return:
	if it is bidder return 1, otherwise return 2
*********************************************/
int checkType(char *buf)
{
    char tmpStr[2];
	getStrSegment(buf, tmpStr,1);
    if(strcmp("1",tmpStr) == 0)
        return 1;
    else 
        return 2;
}

int main(void)
{
	int sockfd;
	int sellerAccepted = 0;
    int bidderAccepted = 0;
	struct sockaddr_in my_addr;	// my address information
	struct sockaddr_in their_addr; // connector's address information
	struct sockaddr_in seller_addr[2]; // connector's address information
	struct sockaddr_in bidder_addr[2]; // connector's address information
    struct sockaddr_in peer_addr[4];
	socklen_t addr_len;
	int numbytes;
    int new_fd;
	char buf[MAXBUFLEN];
	FILE *fp;
	int i;
	
	init();
     
	/*Open a TCP socket */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	my_addr.sin_family = AF_INET;		 
	my_addr.sin_port = htons(PORT_PHASE1);	 
	my_addr.sin_addr.s_addr = INADDR_ANY; //using local host IP address
	memset(&(my_addr.sin_zero), '\0', 8);     
 
    /*binds the TCP socket*/
	if (bind(sockfd, (struct sockaddr *)&my_addr,sizeof(struct sockaddr)) == -1) {
		perror("bind");
		exit(1);
	}
	addr_len = sizeof(struct sockaddr_in);
    //keep listening
    if (listen(sockfd, 4) == -1) {

        perror("listen");
        exit(1);
    }
/********************************************
                 PHASE1 
*********************************************/     
    printf("Auction server has TCP port number %d and IP address%s\n",(int) ntohs(my_addr.sin_port),inet_ntoa(my_addr.sin_addr));
	/*waiting for all Bidders to register*/
    int useCount=1;
	while(!All_Registered()){
        
        if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr,&addr_len)) == -1) {
            perror("accept");
        } 
      
		if ((numbytes = recv(new_fd, buf, MAXBUFLEN-1 , 0)) == -1) {
			perror("recvfrom");
			exit(1);
		}

		buf[numbytes] = '\0';   
        stripHeader(buf);
        char userName[15],passWord[15],BAccount[20];
        getStrSegment(buf, userName,2);
        getStrSegment(buf, passWord,3);
        getStrSegment(buf, BAccount,4);
        
        if(checkType(buf)==1)//bidder type
        {

            if(LoginProcedure(buf)==1)
            {   
                memcpy(&bidder_addr[bidderAccepted],&their_addr,sizeof(struct sockaddr_in));
                BidderIDchecked[bidderAccepted] = 0x1;
                bidderAccepted++;  
                if(send(new_fd, "Accepted#\n", 10, 0) == -1){
                     perror("send");
                     exit(1);
                }
                close(new_fd);
                
                printf("Authentication request. User#: %d Username:%s  Password:%s  Bank Account:%s User IP Addr:%s . Authorized: Accepeted\n",
                useCount,userName,passWord,BAccount,inet_ntoa(their_addr.sin_addr));
            }else{
                 if(send(new_fd, "Rejected#\n", 10, 0) == -1){
                     perror("send");
                     exit(1);
                }
                close(new_fd);
                printf("Authentication request. User#: %d Username:%s  Password:%s  Bank Account:%s User IP Addr:%s . Authorized: Rejected\n",
                useCount,userName,passWord,BAccount,inet_ntoa(their_addr.sin_addr));
            }
        }else{

            if(LoginProcedure(buf)==1)
            {
                memcpy(&seller_addr[sellerAccepted],&their_addr,sizeof(struct sockaddr_in));
                SellerIDchecked[sellerAccepted] = 0x1;
                sellerAccepted++;
                if(send(new_fd, "Accepted#\n", 10, 0) == -1){
                     perror("send");
                     exit(1);
                }
                close(new_fd);
                printf("Authentication request. User#: %d Username:%s  Password:%s  Bank Account:%s User IP Addr:%s . Authorized: Accepeted\n",
                useCount,userName,passWord,BAccount,inet_ntoa(their_addr.sin_addr));
            }else{
                 if(send(new_fd, "Rejected#\n", 10, 0) == -1){
                     perror("send");
                     exit(1);
                 }
                 close(new_fd);
                 printf("Authentication request. User#: %d Username:%s  Password:%s  Bank Account:%s User IP Addr:%s . Authorized: Rejected\n",
                useCount,userName,passWord,BAccount,inet_ntoa(their_addr.sin_addr));
            }         
        }
        useCount++;
	}//while 
    close(sockfd);
    printf("End of Phase 1 for Auction Server\n");
/********************************************************************
                  PHASE2
*********************************************************************/ 
    
     //printf("IP address is: %s\n", inet_ntoa(seller_addr[0].sin_addr));
    char buf_phase2[MAXBUFLEN];
    int sockfd_p2,new_fd2,j;
    char sellerName[15];
	if ((sockfd_p2 = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	my_addr.sin_family = AF_INET;		 
	my_addr.sin_port = htons(PORT_PHASE2);	 
	my_addr.sin_addr.s_addr = INADDR_ANY; //using local host IP address
	memset(&(my_addr.sin_zero), '\0', 8);     
    printf("Auction Server IP Address: %s PreAuction TCP Port Number: %d\n",inet_ntoa(my_addr.sin_addr),(int)ntohs(my_addr.sin_port));
    /*binds the TCP socket*/
	if (bind(sockfd_p2, (struct sockaddr *)&my_addr,sizeof(struct sockaddr)) == -1) {
		perror("bind");
		exit(1);
	}
	addr_len = sizeof(struct sockaddr_in);
    //keep listening
    if (listen(sockfd_p2, 4) == -1) {
        perror("listen");
        exit(1);
    }
    int count=0;
    
    while(count < sellerAccepted)
    {
        if ((new_fd2 = accept(sockfd_p2, (struct sockaddr *)&their_addr,&addr_len)) == -1) 
        {
            perror("accept");
        } 

        memset(buf_phase2, '\0',MAXBUFLEN); 
		if ((numbytes = recv(new_fd2, buf_phase2, MAXBUFLEN-1 , 0)) == -1) 
        {
			perror("recvfrom");
			exit(1);
		}
		buf_phase2[numbytes] = '\0';
        
        FILE* fp_p2 = fopen(buf_phase2,"r");
        if(!fp_p2) printf("open error");
        fseek(fp_p2,0,SEEK_SET);
        fscanf(fp_p2, "%[^\n]\n",sellerName);
        strHandle(sellerName);
        for(j=0;j<AcceptedPeerNum;j++)
        {
            if(strcmp(sellerName,AcceptedPeer[j])==0) break;
        }

        //seller registration record not found, abandon this seller
        if(j >= AcceptedPeerNum)
        {
            continue;
        }
        printf("<Seller%d> send item lists.\n",count+1);
        
        char line[50],output[70]="";
        FILE* bfile = fopen("broadcastList.txt","a+");
        while (fscanf(fp_p2, "%[^\n]\n",line) != EOF) 
        {
            strcat(output,sellerName);
            strcat(output," ");
            strcat(output,line);
            //strHandle_phase2(output);
            fprintf(bfile,"%s\n",output);
            memset(line,'\0',50);
            memset(output,'\0',70);
        }
        fprintf(bfile,"%c\n",'\r');
        fclose(fp_p2);
        fclose(bfile);
        //display items
        FILE* fp_pp = fopen(buf_phase2,"r");
        if(!fp_pp) printf("open error");
        fseek(fp_pp,0,SEEK_SET);
        char uname[20];
        fscanf(fp_p2, "%[^\n]\n",uname);
        while (fscanf(fp_pp, "%[^\n]\n",line) != EOF)
        {
            printf("%s\n", line);
            memset(line,'\0',50);
        }    
        count++; //update the connected seller amount
        close(new_fd2);
    }
    close(sockfd_p2);
    printf("End of Phase 2 for Auction Server\n");
    /*****************************************************************************
                                 PHASE3
     ****************************************************************************/
    
	int UDP_sockfd_P3;
	int TCP_sockfd_P3[4];
	int new_fd_P3;
    FILE* fp_p3;
	socklen_t udp_len = sizeof(struct sockaddr);
	struct sockaddr_in to_UDP_addr_b[2];	// my address information
    struct sockaddr_in from_UDP_addr_b[2];
	struct sockaddr_in my_TCP_addr_P3;	// my address information
    
	/*open the UDP*/
	if ((UDP_sockfd_P3 = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("UDPsocket");
		exit(1);
	}

    char bidder[2][30],line[50];
    char bidbuffer[30];

    i =0;
    int flag=0;
	while( i < bidderAccepted ){

        if(BidderIDchecked[i] != '\0'){
            int port = (i==0?BIDDER1_UDP_PORT:BIDDER2_UDP_PORT);
            
            to_UDP_addr_b[i].sin_family = AF_INET;		 // host byte order
            to_UDP_addr_b[i].sin_port = htons(port);	 // short, network byte order
            to_UDP_addr_b[i].sin_addr.s_addr = seller_addr[0].sin_addr.s_addr;
            //to_UDP_addr_b[i].sin_addr.s_addr = inet_addr( "127.0.0.1") ;
	        //to_UDP_addr_b[i].sin_addr =  *((struct in_addr *)he->h_addr);
            memset(&(to_UDP_addr_b[i].sin_zero), '\0', 8); // zero the rest of the struct
             
            if ((numbytes = sendto(UDP_sockfd_P3, "broadcastList.txt", strlen("broadcastList.txt"), 0,
			 (struct sockaddr *)&to_UDP_addr_b[i], sizeof(struct sockaddr))) == -1) {
                perror("sendto");
                exit(1);
            }

            if ((numbytes = recvfrom(UDP_sockfd_P3, bidder[i], 30, 0,
                    (struct sockaddr *)&from_UDP_addr_b[i], &udp_len)) == -1) {
                    perror("recvfrom");
                    exit(1);
            }
            bidder[i][numbytes] = '\0'; 
            if(flag++ < 1){
                printf("Auction Server IP Address: %s Auction UDP Port Number: %d\n",inet_ntoa(from_UDP_addr_b[0].sin_addr),(int)ntohs(from_UDP_addr_b[0].sin_port));
                //broadcast list
                fp_p3 = fopen("broadcastList.txt","r");
                if(!fp_p3) printf("open error");
                fseek(fp_p3,0,SEEK_SET);
                while (fscanf(fp_p3, "%[^\n]\n",line) != EOF)
                {
                    printf("%s\n", line);
                    memset(line,'\0',50);
                }
                fclose(fp_p3);
            }
            
            printf("Auction Server received a bidding from <Bidder%d>\n",i+1);
            //read bidding information from bidder i;
            fp_p3 = fopen(bidder[i],"r");
            if(!fp_p3) printf("open error");
            fseek(fp_p3,0,SEEK_SET);
            while (fscanf(fp_p3, "%[^\n]\n",line) != EOF) {
                printf("%s\n", line);
                memset(line,'\0',50);
            }
            fclose(fp_p3);
        }
        i++;
   }
   
    close(UDP_sockfd_P3);
    /*****************broadcast bidding result**********/
    for(i =0; i< AcceptedPeerNum;i++)
    {
        if ((TCP_sockfd_P3[i] = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("TCPsocket");
            exit(1);
        }    
        
    }
    bidder_addr[0].sin_port = htons(BIDDER1_TCP_PORT);
    bidder_addr[1].sin_port = htons(BIDDER2_TCP_PORT);  
    seller_addr[0].sin_port = htons(SELLER1_TCP_PORT); 
    seller_addr[1].sin_port = htons(SELLER2_TCP_PORT);
    int peercount=0;
    if(BidderIDchecked[0] != '\0'){
        memcpy(&peer_addr[peercount++],&bidder_addr[0],sizeof(struct sockaddr_in));
        }
    if(BidderIDchecked[1] != '\0'){
        memcpy(&peer_addr[peercount++],&bidder_addr[1],sizeof(struct sockaddr_in));
    }
    if(SellerIDchecked[0] != '\0'){
        memcpy(&peer_addr[peercount++],&seller_addr[0],sizeof(struct sockaddr_in));
    }
    if(SellerIDchecked[1] != '\0'){
        memcpy(&peer_addr[peercount++],&seller_addr[1],sizeof(struct sockaddr_in));
    }

    sleep(3);
    i=0;
    while(i<peercount){
        if (connect(TCP_sockfd_P3[i++], (struct sockaddr *)&peer_addr[i], sizeof(struct sockaddr)) == -1) {
            perror("connect");
            exit(1);
        }  
    }
    FILE* bidfile1;
    FILE* bidfile2;
    FILE* bcFile = fopen("broadcastList.txt","r");
    char line1[50],line2[50];
    if(!bcFile) printf("open error");
    fseek(bcFile,0,SEEK_SET);
    if(BidderIDchecked[0] != '\0'){
        bidfile1 = fopen(bidder[0],"r");
        if(!bidfile1) printf("open error");
        fseek(bidfile1,0,SEEK_SET);    
    }
    if(BidderIDchecked[1] != '\0'){
        bidfile2 = fopen(bidder[1],"r");
        if(!bidfile2) printf("open error");
        fseek(bidfile2,0,SEEK_SET); 
    }
    char sendBuffer[2048]="";
    while (fscanf(bcFile, "%[^\n]\n",line) != EOF)
    {
    	char name[20],item[20],baseprice[20];
        int price = 0,highestPrice=0,winner;
    	getStrSegment(line,name,1);
    	getStrSegment(line,item,2);
    	getStrSegment(line,baseprice,3);
    	price = atoi(baseprice);
        
        if(BidderIDchecked[1] != '\0'){
            while (fscanf(bidfile1, "%[^\n]\n",line1) != EOF)
            {
                char part1[20],part2[20],part3[20];
                int myprice=0;
                getStrSegment(line1,part1,1);
                getStrSegment(line1,part2,2);
                getStrSegment(line1,part3,3);
                if(strcmp(name,part1)==0)
                    if(strcmp(item,part2)==0){
                        myprice = atoi(part3); 
                        if(myprice >= price && myprice >highestPrice){   
                            highestPrice = myprice;
                            winner =1;
                         }                      
                    }
                memset(line1,'\0',50);
            }
            fseek(bidfile1,0,SEEK_SET);  
        }
        if(BidderIDchecked[1] != '\0'){
            while (fscanf(bidfile2, "%[^\n]\n",line2) != EOF)
            {
                char part1[20],part2[20],part3[20];
                int myprice=0;
                getStrSegment(line2,part1,1);
                getStrSegment(line2,part2,2);
                getStrSegment(line2,part3,3);
                if(strcmp(name,part1)==0)
                    if(strcmp(item,part2)==0){
                        myprice = atoi(part3); 
                        if(myprice >= price && myprice > highestPrice){   
                            highestPrice = myprice;
                            winner =2;
                         }   
                                               
                    }
                memset(line2,'\0',50);
            }
            fseek(bidfile2,0,SEEK_SET);
        }
        memset(line,'\0',50);
        
        //recursive send bidding result
        char output[150]="";
        if(price <= highestPrice){
            
            char finalprice[10];
            strcat(output,"item( ");
            strcat(output,name);
            strcat(output,"  ");
            strcat(output,item);
            strcat(output,")");
            strcat(output,"was sold at price: ");
            itostr(finalprice,highestPrice);
            strcat(output,finalprice);
            strcat(output,"  ");
            printf("%s\n",output);
            strcat(output,"#");
            strcat(sendBuffer,output);
             memset(output, '\0',150);    
        }  
    }//while
            i=0;
            while(i<peercount){
                if (send(TCP_sockfd_P3[i++], sendBuffer, strlen(sendBuffer), 0) == -1)
                            perror("send");    
            }   
 memset(sendBuffer, '\0',2048);               
    fclose(bcFile);
    fclose(bidfile1);
    fclose(bidfile2);
    printf("End of Phase 3 for Auction Server\n");
    
    return 0;
}
