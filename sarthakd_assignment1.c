#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#define BACKLOG 3

#include"../include/socket_header.h"
#define STDIN 0



char* GetIP_Port_Host(char* IP,char* Port,char *Host);
int BindandListen(char* portno);
int ServerSideScript(char * portno);
int ClientSideScript(char * portno);
int RegisterToServer(char*ip,char*portno);
int MY_IPPORT(char * port_num,char* command);

//int UploadFile(int sockfd,char *filename,ServerIPList *connectedpeers);
bool CheckIP(char *ip);
typedef struct ServerIPList
{
int ID;
char IP[INET6_ADDRSTRLEN];
char PortNo[5];
char Hostname[1024];
double filesize;
bool IsConnected;
int total_uploads;
int total_downloads;
double avg_upspeed;
double avg_dwnspeed;
bool sockbit;
int sockfd;
}ServerIPList;
int TerminateConnection(int ID,ServerIPList *Connected_clients);
int UpdateAllClients(char* IP,char* Host,char *Port,ServerIPList* ConnectedClients,int sockfd,char *msg);
int UpdateAllClientsfromServer(ServerIPList* ConnectedClients);

void * get_in_addr(struct  sockaddr *sa)
{
    if (sa->sa_family==AF_INET)
        {
        return &((struct sockaddr_in*)sa)->sin_addr;
        }
    return &((struct sockaddr_in6*)sa)->sin6_addr;
}

void * get_in_port(struct  sockaddr *sa)
{
    if (sa->sa_family==AF_INET)
        {
        return &((struct sockaddr_in*)sa)->sin_port;
        }
    return &((struct sockaddr_in6*)sa)->sin6_port;
}

void sigchld_handler(int s)
    {
        while(waitpid(-1, NULL, WNOHANG) > 0);
    }


int main(int argc,char * argv[])
{

/** Production code**/


if (argc != 3)
        {
           // printf("%d \n",argc);
            fprintf(stderr,"usage:  client/server portnumber\n");
            return 1;
        }

    char datain=*argv[1];



    if((datain=='S') || (datain=='s'))
            {
            //Call the server program
           printf("PA1]$>Entering server /n");

           char s[20];
           strcpy(s,argv[2]);

              //checking valid integer reference stack overflow
            bool valid=true;
            int i;
            for (i = 0; i < strlen(s); ++i)
            {
                if (!isdigit(s[i]))
                {
                    valid = false;
                    break;
                }

            }
            if(valid)
            {
           ServerSideScript(argv[2]);
           }
           else{

           printf("invalid port number /n");
           }
            // ServerSideScript(3490);
            }
      else if((datain=='c') || (datain=='C'))
            {

            char s[20];
            strcpy(s,argv[2]);

            bool valid=true;
            int i;
            for (i = 0; i < strlen(s); ++i)
            {
                if (!isdigit(s[i]))
                {
                    valid = false;
                    break;
                }

            }
            //Call the client program
            if(valid)
            {
              printf("[PA1]$> Client mode on \n");
             // printf("[PA1]$");
             // fflush(stdin);
            ClientSideScript((char*)argv[2]);
            }

              else
                {
                printf("invalid port number /n");

                }
            }
      else
            {
                printf("Invalid Arguement %s \n",argv[0]);
            }
    return 0;
}

int ServerSideScript(char * portno)
    {
    //char s[INET6_ADDRSTRLEN];
    int sockfd,sockindex,client_fd,serverfd;
    socklen_t sin_size;
    char *buf=(char*)malloc(sizeof(char)*100);
    struct sockaddr_storage their_addr;
    int var=0;
char *Client_input[4];
     ServerIPList connected_machines[6];
    memset(connected_machines,0,sizeof(connected_machines));
    bool a[4];

    for(var=0;var<4;var++)
    {
    connected_machines[var].IsConnected=false;
    a[var]=connected_machines[var].IsConnected;
    }

    sockfd=BindandListen(portno);
    serverfd=sockfd;
    printf("server: waiting for connections....\n");
    //printf("\n[PA1]$>");
    // fflush(stdin);
    int fdmax;
    fdmax=sockfd;

    fd_set readfds,master;
        FD_ZERO(&readfds);
        FD_SET(STDIN,&readfds);
        FD_SET(sockfd,&readfds);
        master=readfds;
    while(1)
    { // main accept() loop

        readfds=master;
        sockfd= serverfd;

       // printf("\n Server socket is %d",serverfd);

    int recv1=select(fdmax+1,&readfds,NULL,NULL,NULL);
    if (recv1>0)
    {
        for (sockindex=0;sockindex<=fdmax;sockindex++)
        {

            if(FD_ISSET(sockindex,&readfds))
                {

                    if(sockindex==0) //Recive from server keyboard
                    //Implement code for MYIP and MYPORT
                        {

                        read(0,buf,sizeof buf);
                        // useless - scanf("%s",buf);

                        char *temp=buf;
                        while(*temp!='\n')
                            {
                            *temp=toupper(*temp);
                            temp++;
                            }
                            *temp = '\0';
                    //Code below handles the first 4 tasks
                    if(strcmp(buf,"EXIT")==0)
                    {
                        printf("Server exiting \n");
                        return 1;

                    }

                    if(strcmp(buf,"LIST")==0)
                        {

                                int host_id,i;
                                char hostname[1024];
                                char ip_addr[INET6_ADDRSTRLEN];
                                char portno[5];

                                for(i=0;i<4;i++)
                                    {
                                        if(connected_machines[i].IsConnected)
                                            {
                                            host_id=i+1;
                                            strcpy(hostname,connected_machines[i].Hostname);
                                            strcpy(ip_addr,connected_machines[i].IP);
                                            strcpy(portno,connected_machines[i].PortNo);

                                           // printf("%-5d%-35s%-20s%-8s\n",host_id,hostname,ip_addr,portno);
                                             int port_num=atoi(portno);

                                           printf("%-5d%-35s%-20s%-8d\n",host_id,hostname,ip_addr,port_num);



                                            }

                                    }

                            }
                        else
                            {
                                int ret=MY_IPPORT(portno,buf);
                                if(ret!=1)
                                    {
                                            if(ret==2)
                                            {
                                            return 0; //for the exit commmand
                                            }
                                            printf("Invalid command \n");
                                    }
                            }
                        }

                    else if(sockindex==serverfd)
                    {
                        int count=0,p;

                    //process request only if there are less than 4     clients

                        sin_size = sizeof their_addr;
                        client_fd = accept(serverfd, (struct sockaddr *)&their_addr, &sin_size);
                        char Cli_Msg_Pn[25];
                        count= recv(client_fd,(char*)Cli_Msg_Pn,sizeof(Cli_Msg_Pn),0);
                           // printf("RECEIVED %s \n",Cli_Msg_Pn);


                        if(client_fd>fdmax)
                        {
                        fdmax=client_fd;
                        FD_SET(client_fd,&master);

                        }

                        //tokenize command and port no
                       char  *token;//=strtok("REGISTER"," ");
                              int  i=0;





                        socklen_t len;
                        struct sockaddr_storage addr;
                        char ipstr[INET6_ADDRSTRLEN];
                        int port;
                        len = sizeof addr;
                        getpeername(client_fd, (struct sockaddr*)&addr, &len);  //copied from beejs guide
                        // deal with both IPv4 and IPv6:
                        if (addr.ss_family == AF_INET) {
                        struct sockaddr_in *s = (struct sockaddr_in *)&addr;
                        port = ntohs(s->sin_port);
                        inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
                        }
                        else
                        { // AF_INET6
                        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
                        port = ntohs(s->sin6_port);
                        inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
                        }
                        printf("Peer IP address: %s\n", ipstr);
                        printf("Peer port: %s\n", Cli_Msg_Pn);


                            //obtain hostname  copied from beejs guide

                          //  struct sockaddr_in sa; // could be IPv4 if you want
                            char host[1024];
                            char service[20];
                            memset(host,0,sizeof(host));
                            // pretend sa is full of good information about the host and port...
                            getnameinfo((struct sockaddr*)&addr, sizeof addr, host, sizeof host, service, sizeof service, 0);


                            count=0;
                             for(p=0;p<4;p++)
                                    {
                                        if(connected_machines[p].IsConnected)
                                            {
                                                count++;
                                            }

                                    }

                            if(count<5)
                                    {
                                    //printf("client has been registered \n");


                                    char msg[500];

                                            if(UpdateAllClients(ipstr,host,Cli_Msg_Pn,connected_machines,client_fd,msg))
                                            {
                                           printf("Successfully updated all clients \n");

                                            for(i=0;i<4;i++)
                                            {
                                                if(connected_machines[i].IsConnected)
                                                    {
                                                        send(connected_machines[i].sockfd,msg,sizeof(msg),0);

                                                    }

                                            }



                                            }
                                            else
                                            {
                                            printf("Update to client failed \n");
                                            }

                                           // count= send(client_fd,"SUCCESS",sizeof("SUCCESS"),0);

                                     }
                        else
                                 {
                                        printf("Server cannot accept more than 3 clients..try agin \n");
                                 }







                    }


                else
                {

                                 client_fd=sockindex;
                                 char msg[500];
                                 int i;
                                 memset(msg,sizeof(msg),0);
                                 for(i=0;i<4;i++)
                                 {
                                 if(connected_machines[i].sockfd==sockindex)
                                    {
                                        connected_machines[i].IsConnected=false;
                                        close(sockindex);
                                        FD_CLR(sockindex,&master);
                                        printf("The machine %s has unregistered \n",connected_machines[i].Hostname);
                                        break;

                                    }

                                 }

                                 UpdateAllClientsfromServer(connected_machines);



                } //closing third else

                }//closing for
        }


   }
}
free(buf);
close(serverfd);
return 0;
}

int ClientSideScript(char * portno)
{

    int sockindex;
    int host_id_s=7;
    int i=0,yes=1;
    char incoming[30];
    char *temp;
    char buf[100];
    char serveripbuf[500];
    socklen_t sin_size_c;
    ServerIPList Connected_peers[4];
  memset(Connected_peers,0,sizeof(Connected_peers));
    struct sockaddr_storage their_addr_c;
    char *Client_input[12];

    char *Server_input[20];
    char *upload;
    memset(Server_input,0,sizeof(Server_input));
    //ELEMENTS: command ,ip ,portno.
    printf("Client: Please register to the server...\n");
   // fflush(stdin);

    bool togflag=true;
    int Client_fd= BindandListen(portno);
    int server_fd;
    int sockfd=Client_fd;
    fd_set readfds,master;
    FD_ZERO(&readfds);
    FD_ZERO(&master);
    FD_SET(STDIN,&readfds);
    FD_SET(Client_fd,&readfds);
    master=readfds;
    int fdmax=Client_fd;

        while(1)
    { // main accept() loop

        readfds=master;
        sockfd= Client_fd;
       // printf("\n Server socket is %d",serverfd);
        int recv1=select(fdmax+1,&readfds,NULL,NULL,NULL);
        if (recv1>0)
            {
                for (sockindex=0;sockindex<=fdmax;sockindex++)
                {

                    if(FD_ISSET(sockindex,&readfds))
                        {
                           if(sockindex==0)  //enters when keyborad is active
                               {
                                i=0;
                                bool done=false;
                                memset(incoming,0,sizeof(incoming));
                                read(0,incoming,sizeof buf); //reads keyborad input
                                temp=incoming;
                                memset(Client_input,0,sizeof(Client_input));


                        char *temp=incoming;
                        while(*temp!='\n' & *temp!=' ')
                            {
                           // printf("%c",*temp);
                            *temp=toupper(*temp);
                            temp++;
                            }
                           if( *temp == '\n')
                            {
                                *temp='\0';
                            }

                            if(MY_IPPORT(portno,incoming))
                                {
                                    done =true;

                                }
                            else if(strcmp(incoming,"LIST")==0)
                                    {
                                    done =true;

                                    int x=0;
                                        for(x=0;x<4;x++)
                                            {
                                            if(Connected_peers[x].IsConnected)
                                                {
                                                int host_id=Connected_peers[x].ID;
                                                char *hostname=Connected_peers[x].Hostname;
                                                char hostname1[20];
                                                memset(hostname1,0,sizeof(hostname1));
                                                strcpy(hostname1,Connected_peers[x].Hostname);

                                               // char *ip_addr=Connected_peers[x].IP;
                                                char ip_addr[16];
                                                memset(ip_addr,0,sizeof(ip_addr));
                                                strcpy( ip_addr,Connected_peers[x].IP);

                                               char port[6];
                                                strcpy(port,Connected_peers[x].PortNo);
                                                int port_num=atoi(port);

                                                 printf("%-5d%-35s%-20s%-8d\n",host_id,ip_addr,hostname,port_num);

                                                }
                                            }
                                    }
                           else if(strcmp(incoming,"STATISTICS")==0)
                                    {

                                    done =true;


                                    char *host="HOSTNAME";
                                    char *totup="TOTAL U";
                                    char *totdwn="TOTAL DOWNLAODS";
                                    char *avgup="AVERGARE UP";
                                    char *avgdwn="AVERAGE DWN";
                                   // printf("%-35s%-5s%-8s%-5s%-8f\n",host,totup,avgup,totdwn,avgdwn);
                                   printf("Hostname                            TotalU        AvgUp      TotalD       AvgD \n");

                                    for(i=0;i<4;i++)
                                        {
                                            if(Connected_peers[i].IsConnected)
                                                {
                                                   char *hostname=Connected_peers[i].Hostname;
                                                   int totalup=Connected_peers[i].total_uploads;
                                                   int totaldwn=Connected_peers[i].total_downloads;
                                                   float avgup=Connected_peers[i].avg_upspeed*8;
                                                   float avgdwn=Connected_peers[i].avg_dwnspeed*8;


                                                  printf("%-35s%-5d%-8f%-5d%-8f\n",hostname,totaldwn,avgdwn,totalup,avgup);


                                                }

                                        }



                                    }

                            else
                              {
                               char *temp=incoming;
                                while(*temp!='\n')
                                    {
                                    //*temp=toupper(*temp);
                                    temp++;
                                    }
                                    *temp = ' ';


                            char *token;
                            token=strtok(incoming," ");



                            if(token!=NULL)
                            {

                            if(strcmp(token,"REGISTER")==0)
                             {


                             }
                            else if(strcmp(token,"UPLOAD")==0)
                             {


                             }
                            else if(strcmp(token,"DOWNLOAD")==0)
                             {
                             }
                            else if(strcmp(token,"CONNECT")==0)
                             {
                             }
                            else if(strcmp(token,"TERMINATE")==0)
                             {}
                            else if(strcmp(token,"EXIT")==0)
                             {}
                             else {
                                done=true;
                                printf("Invalid input \n");
                                //fflu  {}sh(stdin);
                                break;
                                }
                            }

                                while( token != NULL)
                                    {
                                       // printf( " %s\n", token );
                                        Client_input[i]=token;
                                        token = strtok(NULL, " ");
                                        i++;
                                    }


                                }


                             // ret=MY_IPPORT(portno,Client_input[0]) //checks if its my ip help


//                                    if(MY_IPPORT(portno,Client_input[0]))
//                                    {
//
//                                        //do nothing
//
//                                    }


                    if(!done)
                    {

                                    if(strcmp(Client_input[0],"REGISTER")==0)
                                            {
                                        if(CheckIP(Client_input[1]))
                                        {

                                        struct addrinfo hints,*res;
                                        memset(&hints,0,sizeof hints);
                                        hints.ai_family=AF_INET;
                                        hints.ai_socktype=SOCK_STREAM;


                                        int status=getaddrinfo((char*)Client_input[1],(char*)Client_input[2],&hints,&res);
                                         //int status=getaddrinfo("192.168.0.3","4500",&hints,&res);
                                        if(status!=0)
                                            {
                                            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
                                            return 2;
                                            }

                                        //code to create to connect to an ip address
                                         if ((server_fd=socket(res->ai_family,res->ai_socktype,res->ai_protocol))==-1) //syntax error on beejs guide
                                            {
                                                perror("server:socket \n");

                                            }
                                        if(setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof (int))==-1)
                                            {

                                                perror("set sockopt ");
                                                exit(1);
                                            }

                                            //increasing fd max
                                            if(server_fd>fdmax)
                                            {
                                            fdmax=fdmax+1;
                                            FD_SET(server_fd,&master);
                                            }
                                        //Connects to server
                                         if( connect(server_fd, res->ai_addr, res->ai_addrlen)!=0)
                                         {
                                             perror("accept failed \n");
                                              return 2;
                                         }

        /** Portion of code is to send portnumber of client to server **/
                                        char serv_msg[8];
                                        memset(serv_msg,0,sizeof(serv_msg));
                                        strcpy(serv_msg,Client_input[2]);
                                      //  char *temp=Client_input[2];
                                        char *temp=portno;


                                        char  *token=strtok(incoming," ");
                                        while( token != NULL)
                                            {
                                                printf( " %s\n", token );
                                                Client_input[i]=token;
                                                token = strtok(NULL, " ");

                                            }


                                                i=0;
                                        while(*temp!='\0')
                                                    {
                                                    serv_msg[i]=*temp;
                                                    temp++;
                                                    i++;
                                                    }

                                           //sending port no to server
                                           send(server_fd,serv_msg,sizeof(serv_msg),0);

                                        // Check if client is there in ServerIP List
                                        socklen_t sin_size_c=sizeof(their_addr_c);
                                       // int var2=send(server_fd,(char*)serv_msg,sizeof(serv_msg),0);
                                       memset(Connected_peers,0,sizeof(Connected_peers));

                                                Connected_peers[0].ID=1;
                                                strcpy(Connected_peers[0].IP,Client_input[1]);
                                                Connected_peers[0].IsConnected=true;
                                                strcpy(Connected_peers[0].PortNo,Client_input[2]);
                                                Connected_peers[0].sockfd=server_fd;
                                                //add to materlist and increase


                                                struct hostent *he;
                                                struct in_addr ipv4addr;


                                               struct sockaddr_storage addr;

                                                socklen_t len = sizeof addr;
                                                getpeername(server_fd, (struct sockaddr*)&addr, &len);

                                                char host[100];
                                                memset(host,0,sizeof(host));
                                                char service[200];
                                                // pretend sa is full of good information about the host and port...
                                                getnameinfo((struct sockaddr*)&addr, sizeof addr, host, sizeof host, service, sizeof service, 0);

                                              //  printf("host: %s\n", host);

                                               // printf("[PA1]$>Registration to the server completed successfully \n");
                                                strcpy(Connected_peers[0].Hostname,host);
                                                printf("REGISTER SUCCESS \n");


                                            }



                                       // close(server_fd);
                                        }

                                    else if(strcmp(Client_input[0],"LIST")==0)
                                        {
                                        int x=0;
                                        for(x=0;x<4;x++)
                                            {
                                            if(Connected_peers[x].IsConnected)
                                                {
                                                int host_id=Connected_peers[x].ID;
                                                char *hostname=Connected_peers[x].Hostname;
                                                char hostname1[20];
                                                memset(hostname1,0,sizeof(hostname1));
                                                strcpy(hostname1,Connected_peers[x].Hostname);

                                               // char *ip_addr=Connected_peers[x].IP;
                                                char ip_addr[16];
                                                memset(ip_addr,0,sizeof(ip_addr));
                                                strcpy( ip_addr,Connected_peers[x].IP);

                                               char port[6];
                                                strcpy(port,Connected_peers[x].PortNo);
                                                int port_num=atoi(port);

                                                 printf("%-5d%-35s%-20s%-8d\n",host_id,hostname,ip_addr,port_num);

                                                }
                                            }

                                    //    printf("%­5d%­35s%­20s%­8d\n", host_id, hostname, ip_addr, atoi(port_num));

                                        }
                                    else  if(strcmp(Client_input[0],"CONNECT")==0)
                                        {
                                        if(CheckIP(Client_input[1]))
                                           {

                                                    struct addrinfo hints,*res;
                                                    memset(&hints,0,sizeof hints);
                                                    hints.ai_family=AF_INET;
                                                    hints.ai_socktype=SOCK_STREAM;
                                                    int Client_to_Client;
                                                    bool IsConnectedPeer=false;
                                                    int status=getaddrinfo((char*)Client_input[1],(char*)Client_input[2],&hints,&res);
                                                    bool stop=false;
                                                   // check if server ip is there in iplist
                                                    for(i=0;i<4 & !false ;i++)
                                                    {
                                                        if(Server_input[3*i+3]!=NULL)
                                                        {
                                                                //check if ip matches
                                                                //printf("Comparing %s with %s and %s",Client_input[1],Server_input[3*i+3],Server_input[3*i+1]);
                                                               if(strcmp(Client_input[1],Server_input[3*i+3])==0 ||strcmp(Client_input[1],Server_input[3*i+1]) ) //match for ip or hostname
                                                                    {

                                                                       stop=true;
                                                                    }
                                                        }
                                                    }
                                        if(stop)
                                                {
                                                            if(status!=0)
                                                                {
                                                                fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
                                                                return 2;
                                                                }
                                                             bool flag=false;

                                                             if ((Client_to_Client=socket(res->ai_family,res->ai_socktype,res->ai_protocol))==-1) //syntax error on beejs guide
                                                                {
                                                                    perror("server:socket \n");

                                                                }
                                                            if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof (int))==-1)
                                                                {

                                                                    perror("set sockopt ");
                                                                    exit(1);
                                                                }

                                                            //Connects to client
                                                             if( connect(Client_to_Client, res->ai_addr, res->ai_addrlen)!=0)
                                                             {
                                                                 perror("accept failegethostnamed \n");
                                                                  return 2;
                                                             }
                                                             //add socket to master
                                                             if(Client_to_Client>fdmax)
                                                                {
                                                                    fdmax=Client_to_Client;
                                                                    FD_SET(Client_to_Client,&master);

                                                                }


//                                                            char serv_msg[500];
//                                                            memset(serv_msg,0,sizeof(serv_msg));
//
//                                                            strcat(serv_msg,"CONNECT>>");
//
//                                                            strcat(serv_msg,portno);
//                                                            strcpy(serv_msg,">>");
//
//                                                            strcat(serv_msg,portno);
//                                                            strcpy(serv_msg,">>");
//                                                            printf("%s is sent",serv_msg);



                                                          //  int sizeofmsg=send(Client_to_Client,(char*)serv_msg,sizeof(serv_msg),0);
                                                            //printf("%d sent message %s to peer \n",sizeofmsg,serv_msg);
                                                         //   memset(serv_msg,0,sizeof(serv_msg));

                                                            //recv(Client_to_Client,serv_msg,sizeof(serv_msg),0);

                                                              for(i=0;i<4;i++)
                                                               {
                                                                        if(Connected_peers[i].IsConnected==true)

                                                                            {
                                                                         //   if(ConnectedClients[i].Hostname==Host)  //production code change it to hostname
                                                                         if(Connected_peers[i].PortNo==Client_input[2])
                                                                                {

                                                                                IsConnectedPeer=true;
                                                                                continue;
                                                                                }
                                                                            }
                                                                }



                                                                    socklen_t len;
                                                                    struct sockaddr_storage addr;
                                                                    char ipstr[INET6_ADDRSTRLEN];

                                                                    len = sizeof addr;
                                                                    getpeername(Client_to_Client, (struct sockaddr*)&addr, &len);  //copied from beejs guide
                                                                    // deal with both IPv4 and IPv6:
                                                                    if (addr.ss_family == AF_INET) {
                                                                    struct sockaddr_in *s = (struct sockaddr_in *)&addr;
                                                                   // port = ntohs(s->sin_port);
                                                                    inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
                                                                    }
                                                                    else
                                                                    { // AF_INET6
                                                                    struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
                                                                   // port = ntohs(s->sin6_port);
                                                                    inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
                                                                    }



                                                                    //obtain hostname  copied from beejs guide

                                                                  //  struct sockaddr_in sa; // could be IPv4 if you want
                                                                    char host[1024];
                                                                    char service[50];
                                                                    // pretend sa is full of good information about the host and port...
                                                                    getnameinfo((struct sockaddr*)&addr, sizeof addr, host, sizeof host, service, sizeof service, 0);

                                                                    printf("host: %s\n", host);
//                                                                    char hostname[128]; //copied from beejs
//                                                                    gethostname(hostname, sizeof hostname);

                                                                    bool notadded=true;
                                                                    //checks if peer is connected

                                                                    if(IsConnectedPeer==false)
                                                                        {
                                                                            for(i=0;(i<4) & (notadded);i++)
                                                                                {
                                                                                if(Connected_peers[i].IsConnected==false)
                                                                                  {
                                                                                    Connected_peers[i].IsConnected=true;
                                                                                    Connected_peers[i].ID=i+1;
                                                                                    strcpy(Connected_peers[i].IP,Client_input[1]);
                                                                                    strcpy(Connected_peers[i].PortNo,Client_input[2]);
                                                                                    strcpy(Connected_peers[i].Hostname,host);
                                                                                       Connected_peers[i].sockfd=Client_to_Client;
                                                                                       printf("Successful connection sent to client \n");

                                                                                    notadded=false;
                                                                                //    close(server_fd);

                                                                                  }

                                                                                }
                                                                        }
                                                                    else
                                                                        {
                                                                            printf("Duplicate connections not allowed \n");

                                                                        }


                                                           // close(server_fd);
                                            }
                                            else
                                                {

                                                printf("Client is not registered to server or a possible self connection,try again \n");
                                                }

                                        }




                                         }
                                    else if(strcmp(Client_input[0],"TERMINATE")==0)
                                                    {
                                                        int Con_Id=atoi(Client_input[1]);//Converts connections id to integer
                                                        if((Con_Id<5) & (Con_Id > 0))
                                                          {
                                                                if (Connected_peers[Con_Id-1].IsConnected )
                                                                    {
                                                                    //Call the subroutine to terminate
                                                                    Connected_peers[Con_Id-1].IsConnected=false;
                                                                    int terminatesok=Connected_peers[Con_Id-1].sockfd;
                                                                    FD_CLR(terminatesok,&master);
                                                                    close(terminatesok);
                                                                    printf("TERMINATE success\n");
//                                                                        if( TerminateConnection(Con_Id-1,Connected_peers))
//                                                                           {
//                                                                           printf("Successfully terminated connection id %d \n",Con_Id);
//                                                                           }
                                                                    }
                                                         }
//                                                                else
//                                                                    {
//                                                                    printf("Invalid connection ID \n");
//                                                                    }
                                                    }


                                    else if(strcmp(Client_input[0],"EXIT")==0)
                                                     {

                                                        for(i=3;i>=0;i--)
                                                            {
                                                                if(Connected_peers[i].IsConnected)
                                                                    {
                                                                   Connected_peers[i-1].IsConnected=false;
                                                                    int terminatesok;
                                                                    terminatesok=Connected_peers[i+1].sockfd;
                                                                    FD_CLR(terminatesok,&master);
                                                                   // printf("Closing sock %d \n",terminatesok);
                                                                    close(terminatesok);
                                                                    printf("TERMINATE success\n");


                                                                    }


                                                            }

                                                          //TerminateConnection_Server(0,Connected_peers,portno);

                                                          printf("Good Bye,Client Exiting \n");
                                                          return 1;
                                                      }

                                     else if(strcmp(Client_input[0],"UPLOAD")==0)
                                                    {

                                                    //check if the id is connected

                                                    int i=0;
                                                    int socket_id;
                                                    int id=atoi(Client_input[1])-1;
                                                    char filename[20];
                                                    //Connected_peers[id].total_uploads
                                                    strcpy(filename,Client_input[2]);
                                                    if(Connected_peers[id].IsConnected)
                                                    {
                                                    socket_id=Connected_peers[id].sockfd;
                                                    Connected_peers[id].total_uploads+=1;
                                                    UploadFile(socket_id,filename,Connected_peers);
                                                    }
                                                    else
                                                    {
                                                        printf("Connection does not exist,invlaid id \n");
                                                    }

                                                    }
                                    else if(strcmp(Client_input[0],"DOWNLOAD")==0)
                                                {
                                                int j=1;
                                                int i=0;


                                                while(Client_input[j]!=NULL)
                                                    {
                                                      int id=atoi(Client_input[j]);

                                                    bool found=false;
                                                    for(i=0;i<4;i++)
                                                        {
                                                           if(Connected_peers[i].ID==id)
                                                                {
                                                                found=true;
                                                                int socket_id;

                                                                char filename[20];
                                                                //Connected_peers[id].total_uploads
                                                                strcpy(filename,Client_input[j+1]);
                                                                //send message
                                                                char msg[500];
                                                                memset(msg,0,sizeof(msg));
                                                                strcat(msg,"DOWNLOAD>>");
                                                                strcat(msg,filename);
                                                                strcat(msg,">>");
                                                                Connected_peers[i].total_downloads+=1;


                                                                int size=send(Connected_peers[i].sockfd,msg,sizeof(msg),0);
                                                                if(size)
                                                                    {
                                                                        printf("Reuest to download file %s from host %s sent using sockid %d\n",filename,Connected_peers[i].Hostname,Connected_peers[i].sockfd);

                                                                    }


                                                                }



                                                        }

                                                        if(!found)
                                                            {
                                                                printf("The following id could not be found or not connected:%s \n",Client_input[j]);

                                                            }

                                                        j+=2;



                                                    }

                                                }

                                    else
                                        {
                                        printf("Invalid command \n");
                                        //fflush(stdin);

                                        }
                                }

                         printf("[PA1]$>");
                        //            fflush(stdin);
                    } //sok 0 closing


                            else if (sockindex==Client_fd) //listener
                                {
                                int var2;
                                int j=0;
                                bool flag=true;
                                char serv_msg[200];
                                char filename[20];
                                memset(serv_msg,0,sizeof(serv_msg));
                                int kkk=recv(Client_fd,serv_msg,sizeof(serv_msg),0);

                                memset(serv_msg,0,sizeof(serv_msg));
                              //  memset(Server_input,0,sizeof(Server_input));

                                   sin_size_c = sizeof their_addr_c;
                                int serverfd=accept(Client_fd,(struct sockaddr*)&their_addr_c,&sin_size_c);
                                  if (serverfd>fdmax)
                                                    {
                                                    FD_SET(serverfd,&master);
                                                    //printf("FD MAX %D",fdmax);
                                                    fdmax++;
                                                    }
                            if(serverfd<0)
                            {
                            perror("Accept failed:");

                            }


                            socklen_t len;
                            struct sockaddr_storage addr;
                            char ipstr[INET6_ADDRSTRLEN];
                            int port;
                            len = sizeof addr;
                            char ack[10];
                            char buf[6];
                            strcpy(ack,"SUCCESS");
                            //recv(serverfd,buf,sizeof(buf),0);
        //                        int var=send(serverfd,(char*)ack,sizeof(ack),0);
        //                          if(var<0);
        //                                    {
        //                                        perror("SendFailed at peer:");
        //                                    }
                                printf("accept passed \n");
                            getpeername(serverfd, (struct sockaddr*)&addr, &len);  //copied from beejs guide
                            // deal with both IPv4 and IPv6:
                            if (addr.ss_family == AF_INET) {
                            struct sockaddr_in *s = (struct sockaddr_in *)&addr;
                            port = ntohs(s->sin_port);
                            inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
                            }
                            else
                            { // AF_INET6
                            struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
                            port = ntohs(s->sin6_port);
                            inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
                            }
                            printf("Peer IP address: %s\n", ipstr);
                           // printf("Peer port: %d\n", port);


                                //obtain hostname  copied from beejs guide

                              //  struct sockaddr_in sa; // could be IPv4 if you want
                                char host[1024];
                                char service[20];
                                // pretend sa is full of good information about the host and port...
                                getnameinfo((struct sockaddr*)&addr, sizeof addr, host, sizeof host, service, sizeof service, 0);

                                    //Count the number of peers and proceed if less than 4 and peer is not already connected
                                    int PeerCount=0;
                                    for(i=0;i<4;i++)
                                        {
                                        if(Connected_peers[i].IsConnected)
                                            {
                                            PeerCount++;

                                            }

                                        }



                                    //send(serverfd,ack,sizeof(ack));
                                    if(PeerCount>3)
                                        {
                                        printf("Client server is full and cannot accept connections \n");
                                        //send(serverfd,"CONNECTIONS_FULL",sizeof("CONNECTIONS_FULL"),0);
                                        return 1;
                                        }


                                    bool stop=false;
                                    for(i=0;(i<4)&(!stop);i++)
                                            {
                                                if(Connected_peers[i].IsConnected==false)
                                                        {

                                                        printf("Adding the client details \n");


                                                          Connected_peers[i].IsConnected=true;
                                                         // printf("Adding hostname :%s \n",host);
                                                           strcpy( Connected_peers[i].Hostname,host);
                                                           printf("Adding ip %s \n",ipstr);
                                                           strcpy(Connected_peers[i].IP,ipstr);
                                                         //  printf("Data added ip and hostname \n");


                                                           Connected_peers[i].ID=i+1;
                                                           Connected_peers[i].sockfd=serverfd;
                                                           char portno[5];
                                                           int k=0;
                                                           //loop through server ip list and update port no


                                                         for(k=0;k<4 & !stop;k++)
                                                            {
                                                                       // char hostname[128];
                                                                        char ip_addr[INET6_ADDRSTRLEN];


                                                                        char *sp=Server_input[((3*k)+2)];

                                                                     //   printf("SERVER givin ip:%s and %s",sp,ipstr);


                                                                        if(strcmp(ipstr,Server_input[((3*k))])==0)
                                                                        {


                                                                       strcpy(portno,Server_input[3*k+1]);
                                                                    //   printf("Adding %s as port \n",portno);


                                                                            strcpy(Connected_peers[i].PortNo,portno);
                                                                            printf("Successful connection completed with client \n");
                                                                            stop=true;
                                                                    }
                                                              // strcpy(Connected_peers[i].PortNo,port);


                                                            }//clsoing for
                                                        }//closing if

                                                   else
                                                        {
                                                         continue;
                                                        }


                                                }


                        } //closing listener
                            else
                                {
                              int nbytes;


                              memset(serveripbuf,0,sizeof(serveripbuf));
                              if ((nbytes = recv(sockindex, serveripbuf, sizeof serveripbuf, 0)) <= 0) {
                                // got error or connection closed by client
                                if (nbytes == 0) {



                                FD_CLR(sockfd,&master);
                                printf("socket %d has been closed by the client\n", sockindex);
                                }
                                close(sockindex); // bye!
                                FD_CLR(sockindex, &master); // remove from master set

                               } else {

                                int j;//copied from beejs guide
                                for(j = 0; j <= fdmax; j++) {
                                // send to everyone!
                                if (FD_ISSET(j, &master)) {
                                // except the listener and ourselves
                                if (j != Client_fd && j != sockindex) {

                                //find the details of connected peer
                                int index;
                                bool stop=false;





                                for(index=0;index<4 & !stop;index++)
                                    {
                                    if(Connected_peers[index].sockfd==sockindex)
                                        {
                                           stop=true;
                                           i--;

                                        }


                                    }


                                   if(sockindex==4) //assume message from server which is server ip list

                                            {

                                                                    int host_id,k=0,j=0;
                                                                    bool stop=false;
                                                                    printf("Recived data from server:SERVERIPLIST\n");
                                                                   // printf("%s \n",serveripbuf);
                                                                    char  *token;
                                                                    memset(Server_input,0,sizeof(Server_input));


                                                                    token=strtok(serveripbuf,">>");
                                                                    //token=strtok(NULL,">>");

                                                                    while( token != NULL)
                                                                        {
                                                                            Server_input[k]=token;
                                                                            token=strtok(NULL,">>");
                                                                            k++;
                                                                        }
                                                                   //  send(serverfd,serv_msg,sizeof(serv_msg),0);
                                                                    for(k=0;k<4 & !stop;k++)
                                                                        {
                                                                            char hostname[128];
                                                                            char ip_addr[INET6_ADDRSTRLEN];
                                                                            char portno[5];

                                                                            host_id=k+1;
                                                                            int x,y,z;

                                                                            strcpy(hostname,Server_input[((3*k)+0)]);
                                                                            strcpy(ip_addr,Server_input[3*k+2]);
                                                                            strcpy(portno,Server_input[3*k+1]);
                                                                             int port=atoi(portno);
                                                                            printf("%-5d%-35s%-20s%-8d\n",host_id,hostname,ip_addr,port);
                                                                            if(Server_input[3*k+4]==NULL)
                                                                                {
                                                                                    stop=true;
                                                                                }
                                                                        }

                                                }//closing server ip list if


                                else  //upload and download portion
                                {
                                    char message[500];//copyin serveripbuf
                                    char *token;
                                    strcpy(message,serveripbuf);
                                    token=strtok(message,">>");
                                    char process[8];
                                    strcpy(process,token);
                                    char filename[20];
                                    token=strtok(NULL,">>");
                                    strcpy(filename,token);
                                    bool upload=!strcmp(process,"UPLOAD");
                                    bool download=!strcmp(process,"DOWNLOAD");

                                    if(togflag & upload)
                                        {
                                            //time to download

                                  //  printf("File is being uploaded to the server on socket %d \n",sockindex);
                                    FILE *fp;
                                    char * token;
                                    printf("RECVIVING bytes:%d from the server\n",nbytes);
                                    //add to the download count
                                    Connected_peers[index].filesize+=nbytes;

                                    //    fp=fopen("workman1","a");
                                    int bytesReceived;
                                    char recvBuff[500];
                                    time_t start;
                                    time_t end;

                                   // struct timezone tz;



                                    char sizeofstream[10];
                                    token=strtok(NULL,">>");
                                    strcpy(sizeofstream,token);


                                      if((strcmp(sizeofstream,"SOF")==0) )  //checks if the message is start of stream
                                                      {
                                                      printf("File %s is being downloaded to the server on socket %d:Download has begun \n",filename,sockindex);
                                                      time(&start);
                                                      Connected_peers[index].filesize+=nbytes;



                                                      }


                                        else if((strcmp(sizeofstream,"EOS")==0) ) //checks if the message is end of stream
                                                      {
                                                     printf("File %s is completley downloaded successfully \n",filename);
                                                      time(&end);

                                                     double downloadtime=difftime(end,start);
                                                    // printf("The download time is %f",downloadtime);
                                                     char myhost[30];
                                                     size_t len=30;
                                                     double siz=Connected_peers[index].filesize;
                                                     Connected_peers[index].total_downloads+=1;
                                                     char *desthost=Connected_peers[index].Hostname;
                                                     double rx_rate=siz/downloadtime;
                                                     Connected_peers[index].total_uploads+=1;
                                                     gethostname(myhost,len);
                                                     Connected_peers[index].avg_dwnspeed=rx_rate;
                                                     printf("Rx:%s -> %s File size:%f Time taken:%f Rx Rate:%f \n",desthost,myhost,siz,downloadtime,rx_rate);


                                                      }

                                        else if(strcmp(process,"UPLOAD")==0)     //starts receiving the message
                                                      {
                                                         int datasize=atoi(sizeofstream);
                                                         char datapacket[500];
                                                         memset(datapacket,0,sizeof(datapacket));
                                                         int header=50;
                                                         int i=0;
                                                         int packetlen=strlen(serveripbuf);
                                                         datasize=packetlen-header;

                                                         while(header<strlen(serveripbuf)) //read the content of the buffer
                                                         {
                                                            datapacket[i]=serveripbuf[header-1];
                                                            header++;
                                                            i++;
                                                         }



                                                          FILE * hope=fopen(filename,"ab");
                                                          if(hope!=NULL)
                                                          {
                                                          // fputs(datapacket ,hope );
                                                            fwrite(datapacket,1,datasize,hope);

                                                            fclose(hope);
                                                          }
                                                      }

                                               else
                                                    {
                                                        printf("The download process failed \n");

                                                    }







                                            } //closing else
                                     else if(download)
                                        {
                                          for(i=0;i<4;i++)
                                            {
                                            if(Connected_peers[i].sockfd==sockindex)
                                                {


                                                printf("Request by %s for download of file %s \n",Connected_peers[index].Hostname,filename);
                                                        UploadFile(Connected_peers[i].sockfd,filename,Connected_peers);
                                                }
                                            }



                                        }
                                    togflag=!togflag;

                                    } //close upload download
                                }
                                }
                                }






                            }//closing else

                }//closing for


                        } //closing if of stdin

            }//closing while

     }
   //  close(Client_fd);
    } //closing while
} //closing client server script






int UploadFile(int sockfd,char *filename,ServerIPList *connected_peers)
{
int id;
    char sendbuffer[500];
    char filebuf[450];
    int datarate=400;
    int headsize=50;
    int bytesReceived;
    char recvBuff[500];
    time_t start;
    time_t end;
    int res,i;
    res=1;

    for(i=0;i<4;i++)
        {
        if(connected_peers[i].sockfd==sockfd)
            {
                connected_peers[i].total_uploads+=1;
                break;

            }


        }
  //  time_t start ;

           FILE   * rm = fopen(filename, "rb");


           //check if file exists

                time(&start); //start the timer
           //toekn*ize file name
           if(rm!=NULL)
           {


                                       printf("File opened \n");
                                       char *token;
                                       char dupfile[20];
                                       strcpy(dupfile,filename);
                                       token=strtok(dupfile,"/");
                                       while(token!=NULL)
                                        {
                                           filename=token;
                                           token= strtok(NULL,"/");

                                         }


                                         //send a notification to receiver that the file upload is starting
                                            memset(sendbuffer,0,sizeof(sendbuffer));
                                            strcat(sendbuffer,"UPLOAD>>");
                                            strcat(sendbuffer,filename);
                                            strcat(sendbuffer,">>SOF>>");
                                            double bytes=send(sockfd,sendbuffer,sizeof(sendbuffer),0); //send start of stream message
                                            connected_peers[i].filesize=connected_peers[i].filesize+bytes;
                                            bool upflag;


                                          //strcpy(buf,"UPLOAD ");
                                          while(!feof(rm))
                                          {


                                              memset(filebuf,0,sizeof(filebuf));
                                               memset(sendbuffer,0,sizeof(sendbuffer));
                                               strcat(sendbuffer,"UPLOAD>>");
                                               strcat(sendbuffer,filename);
                                               strcat(sendbuffer,">>400>>");
                                               int headerlen=strlen(sendbuffer);

                                               while(headerlen<headsize-1)
                                                {
                                                strcat(sendbuffer,">");
                                                headerlen++;

                                                }


                                               //tansmission rate

                                               //strcat(sendbuffer,">>320")

                                              int k=0;

                                          // res = fgets(filebuf, datarate, rm);

                                            res=fread(filebuf,1,datarate,rm);
                                            //add the details to sendbuffer



                                           // printf("DATASENT:%s \n\n",filebuf);
                                            strcat(sendbuffer,filebuf);
//                                            char *temp=filebuf;
//                                            while(k<datarate)
//                                                {
//                                                sendbuffer[headsize+k]=*temp;
//                                                temp++;
//                                                k++;
//                                                }





                                        // printf("SENDING MSG:%S \n\n",sendbuffer);
                                         upflag=false;
                                        int bytes=send(sockfd,sendbuffer,sizeof(sendbuffer),0);
                                          printf("SENDING DATA IN BYTES:%d \n",bytes);
                                           connected_peers[i].filesize=connected_peers[i].filesize+bytes;
                                            if(bytes<0)
                                                {
                                                   printf("Uloading failed \n");
                                                     //fclose(rm);
                                                     upflag=true;
                                                  //   fclose(rm);

                                                }


                                        }

                                        //send an end of stream notification the receiver
                       if(!upflag)
                                    {

                                            memset(sendbuffer,0,sizeof(sendbuffer));
                                            strcat(sendbuffer,"UPLOAD>>");
                                            strcat(sendbuffer,filename);
                                            strcat(sendbuffer,">>EOS>>");

                                            int sent=send(sockfd,sendbuffer,sizeof(sendbuffer),0);
                                            connected_peers[i].filesize+=sent;
                                            connected_peers[i].filesize=connected_peers[i].filesize+bytes;

                                        }



                                            fclose(rm);



                                            //calculate upload time

                                         time(&end);

                                         double uploadtime=difftime(end,start);
                                        // printf("The download time is %f",downloadtime);
                                         char myhost[30];
                                         size_t len=30;
                                         double siz=connected_peers[i].filesize;
                                         char *desthost=connected_peers[i].Hostname;
                                         double tx_rate=siz/uploadtime;
                                         gethostname(myhost,len);
                                         connected_peers[i].avg_dwnspeed=tx_rate;
                                         printf("Tx:%s -> %s File size:%f Time taken:%f Tx Rate:%f \n",myhost,desthost,siz,uploadtime,tx_rate);

           }
       else
            {
                printf("Destination file not found \n");
            }

}



bool CheckIP(char *ip) //reference beejs guide
{
 int i;
struct hostent *he;
struct in_addr **addr_list;

if ((he = gethostbyname(ip)) == NULL) {
herror("gethostbyname");
return false;
}

// get the host info
// print information about this host:
printf("Official name is: %s\n", he->h_name);
 ip=he->h_name;
 return true;

printf("\n");

}

int BindandListen(char* portno)
    {
    struct addrinfo hints,*serverinfo,*p;
    int rv,sockfd,yes=1;
    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_flags=AI_PASSIVE;
    rv=getaddrinfo(NULL,portno,&hints,&serverinfo);

    if(rv!=0)
    {
        fprintf(stderr, "getaddrinfo: %s \n", gai_strerror(rv));
        return 1;
    }
    for(p=serverinfo;p!=NULL;p=p->ai_next)
    {

        if ((sockfd=socket(p->ai_family,p->ai_socktype,p->ai_protocol))==-1) //syntax error on beejs guide
        {
            perror("server:socket \n");
            continue;
        }
    if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof (int))==-1)
        {

            perror("set sockopt ");
            exit(1);
        }
    if(bind(sockfd,p->ai_addr,p->ai_addrlen)==-1)
        {
            close(sockfd);
            perror("server: bind");
            continue;
        }

    break;
}

//printf("The local ip is %s \n",localip->sin_addr);

    if (p == NULL)
        {
            fprintf(stderr, "server: failed to bind\n");
            return 2;
        }
        freeaddrinfo(serverinfo); // all done with this structure


    if(listen(sockfd,BACKLOG)==-1)
    {
    perror("listen");
    exit(1);
    }

    return sockfd;
    }

int MY_IPPORT(char* port_num,char * command)
    {

        if (strcmp(command,"MYIP")==0)
            {
            // Reference for this function is beejs guide and http://jhshi.me/2013/11/02/how-to-get-hosts-ip-address/
            int sockfd,status;
            struct addrinfo hints,*res;
            char ip_addr[INET6_ADDRSTRLEN];
            struct sockaddr_in hostaddr;
            socklen_t hostlen = sizeof(hostaddr);
            memset(&hints,0,sizeof hints);
            hints.ai_family=AF_INET;
            hints.ai_socktype=SOCK_STREAM;


            if ((status = getaddrinfo("8.8.8.8", "53", &hints, &res)) != 0)
            {
                fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
                return 2;

            }

            sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
            status = connect(sockfd, res->ai_addr, res->ai_addrlen);

            if (status != 0)
            {
                fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
                return 2;
            }



            getsockname(sockfd,(struct sockaddr*)&hostaddr,&hostlen);
            void *addr;
            addr = &(hostaddr.sin_addr);
            inet_ntop(res->ai_family, addr, ip_addr, sizeof ip_addr);


            printf("IP address:%s \n", ip_addr);
            close(sockfd);
            return 1;
           }
        else if (strcmp(command,"MYPORT")==0)
            {

               printf("Port number:%s \n",port_num);
                return 1;
            }
         else if (strcmp(command,"HELP")==0)
            {
                printf(" 1:Creator \n 2:Help \n 3:MyPort \n 4:MyIp \n");
                return 1;
            }
       else if(strcmp(command,"CREATOR")==0)

            {
                printf("SARTHAK DUNDIRAJ BHAT\n");
                printf("SARTHAKD\n");
                printf("SARTHAKD@BUFFALO.EDU\n");
                printf("I have read and understood the course academic integrity policy located at http://www.cse.buffalo.edu/faculty/dimitrio/courses/cse4589_f14/index.html#integrity \n");




                return 1;
            }
                return 0;

    }
int UpdateAllClients(char* IP,char* Host,char *Port,ServerIPList* ConnectedClients,int sockfd,char *msg)
    {
            char ip_in[INET6_ADDRSTRLEN];
            char ip_in_cat[INET6_ADDRSTRLEN+1];
            memset((char*)ip_in,0,sizeof(ip_in));
            char port_in[5];
            memset((char*)port_in,0,sizeof(port_in));
            char host_in[1024];
            memset((char*)host_in,0,sizeof(host_in));
            int i=0;
            bool existingserver=false;
            char *temp;
            temp=IP;
            while(*temp!='\0')

                    {
                    ip_in[i]=*temp;
                    i++;
                    temp++;
                    }




            temp=Port;
            i=0;
            while(*temp!='\0')
                        {
                        port_in[i]=*temp;

                        i++;
                        temp++;
                        }

            i=0;
            temp=Host;
            while(*temp!='\0')
                        {
                       host_in[i]=*temp;
                        i++;
                        temp++;
                        }
    for(i=0;i<4;i++)
        {
                     //   if(ConnectedClients[i].Hostname==Host)  //production code change it to hostname
                     //Checks if the client is already connected to the database and updates existing server if true
                     if(ConnectedClients[i].PortNo==Port)
                            {

                                existingserver=true;
                                continue;
                            }

        }

           if(existingserver!=true)  //if client is not added it checks for free space in array and adds it
                        {       bool stop=false;
                                for(i=0;i<4 & !stop;i++)
                                    {
                                          if(ConnectedClients[i].IsConnected==false)
                                                {
                                                    ConnectedClients[i].IsConnected=true;
                                           //     printf("%s hostname obtained \n",host_in);
                                                   strcpy( ConnectedClients[i].Hostname,host_in);
                                             //      printf("%s ip obtained \n");
                                                   // ConnectedClients[i].Hostname=host;
                                                   strcpy(ConnectedClients[i].IP,ip_in);
                                               //    printf("copied ip and hostname");
                                                   strcpy(ConnectedClients[i].PortNo,port_in);
                                                   ConnectedClients[i].sockfd=sockfd;

                                                    stop=true;
                                                }
                                           else
                                                {
                                                 continue;
                                                }

                                    }
                        }



  for(i=0;i<4;i++)  //sends message for all the clients connected
   {
            if(ConnectedClients[i].IsConnected==true)

                {
                    int j;
                    //char msg[100];
                    memset(msg,0,sizeof(msg));
                    //strcpy(msg,"SERVERIPLIST ");
                    for(j=0;j<4;j++)
                        {
                            if(ConnectedClients[j].IsConnected)  //send the list of connectedclients to the clients
                                {
                                        strcat(msg,">>");
                                    strcat(msg,ConnectedClients[j].IP);

                                        strcat(msg,">>");

                                    strcat(msg,ConnectedClients[j].PortNo);


                                         strcat(msg,">>");

                                    strcat(msg,ConnectedClients[j].Hostname);


                                     //printf("%d is the sockfd \n",ConnectedClients[j].sockfd);

                               }
                        }
return 1;
                   // printf("%s \n",msg);
//                    if(send(sockfd,(char*)msg,sizeof(msg),0))
//                        {
//                            printf("Updating the clients :%s \n",msg);
//
//                        }
//                    else
//                        {
//                            printf("Sending failed for IP %s with port %s",ConnectedClients[i].IP,ConnectedClients[i].PortNo);
//                        }
        //for engs
                     // close(server_fd);
                 }
              //  ConnectedClients++;
   }

//return 1;

}

int UpdateAllClientsfromServer(ServerIPList* ConnectedClients)
    {

 int i;
  for(i=0;i<4;i++)  //sends message for all the clients connected
   {
            if(ConnectedClients[i].IsConnected==true)

                {
                int server_fd;

                    int j=0;
                    char msg[200];
                    memset(msg,0,sizeof(msg));
                 //   strcat(msg,"SERVER_IP_LIST ");
                    for(j=0;j<4;j++)
                        {
                            if(ConnectedClients[j].IsConnected)  //send the list of connectedclients to the clients
                                {
                                    strcat(msg,">>");
                                    strcat(msg,ConnectedClients[j].IP);

                                    strcat(msg,">>");
                                    strcat(msg,ConnectedClients[j].PortNo);


                                    strcat(msg,">>");
                                    strcat(msg,ConnectedClients[j].Hostname);
					//strcat(msg,">>");
                               }
                        }

                   // printf("%s \n",msg);
                    if(send(ConnectedClients[i].sockfd,(char*)msg,sizeof(msg),0))
                        {
                       // printf("Server sent :%s",msg);
                         //  printf("Updating the clients  \n");
                       //  printf("%s \n",msg);
                        }
                    else
                        {
                            printf("Sending failed for IP %s with port %s",ConnectedClients[i].IP,ConnectedClients[i].PortNo);
                        }
        //for engs
                     // close(server_fd);
                 }
              //  ConnectedClients++;
   }

return 1;

}


int CreateSocket(char IP[20],char portno[5])
    {

        struct addrinfo hints,*res;
        memset(&hints,0,sizeof hints);
        hints.ai_family=AF_INET;
        hints.ai_socktype=SOCK_STREAM;
        int Client_to_Client;
        bool IsConnectedPeer=false;
        int yes =1;
        int status=getaddrinfo((char*)IP,(char*)portno,&hints,&res);
        bool stop=false;
         if(status!=0)
                    {
                    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
                    return 2;
                    }
                 //bool flag=false;

                 if ((Client_to_Client=socket(res->ai_family,res->ai_socktype,res->ai_protocol))==-1) //syntax error on beejs guide
                    {
                        perror("server:socket \n");

                    }
                if(setsockopt(Client_to_Client,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof (int))==-1)
                    {

                        perror("set sockopt ");
                        exit(1);
                    }

                //Connects to client
               if( connect(Client_to_Client, res->ai_addr, res->ai_addrlen)!=0)
                     {
                         perror("accept failegethostnamed \n");
                          return 2;
                     }

        return Client_to_Client;

    }

int TerminateConnection(int ID,ServerIPList *Connected_clients)
    {
        //Removing connection from the source client
        Connected_clients[ID].IsConnected=false;


        struct addrinfo *res,hints;
        memset(&hints,0,sizeof hints);
        hints.ai_family=AF_INET;
        hints.ai_socktype=SOCK_STREAM;
        hints.ai_flags=AI_PASSIVE;
        int Client_to_Client;
        int status=getaddrinfo((char*)Connected_clients[ID].IP,(char*)Connected_clients[ID].PortNo,&hints,&res);
        if(status!=0)
            {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
            return 2;
            }
         bool flag=false;

         if ((Client_to_Client=socket(res->ai_family,res->ai_socktype,res->ai_protocol))==-1) //syntax error on beejs guide
            {
                perror("server:socket \n");

            }

        //Connects to client
         if( connect(Client_to_Client, res->ai_addr, res->ai_addrlen)!=0)
         {
             perror("accept failed \n");
              return 2;
         }
        char buf[15];
        strcpy(buf,"TERMINATE");
//        if(i=0) //unregister from server
//            {
//                str
//            }
       int var= send(Client_to_Client,buf,sizeof(buf),0);
       //printf("TERMINATE sent %d \n",var);

        return 1;
    }

int TerminateConnection_Server(int ID,ServerIPList *Connected_clients,char portno[5])
    {
        //Removing connection from the source client
        Connected_clients[ID].IsConnected=false;


        struct addrinfo *res,hints;
        memset(&hints,0,sizeof hints);
        hints.ai_family=AF_INET;
        hints.ai_socktype=SOCK_STREAM;
        hints.ai_flags=AI_PASSIVE;
        int Client_to_Client;
        int status=getaddrinfo((char*)Connected_clients[ID].IP,(char*)Connected_clients[ID].PortNo,&hints,&res);
        if(status!=0)
            {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
            return 2;
            }
         bool flag=false;

         if ((Client_to_Client=socket(res->ai_family,res->ai_socktype,res->ai_protocol))==-1) //syntax error on beejs guide
            {
                perror("server:socket \n");

            }

        //Connects to client
         if( connect(Client_to_Client, res->ai_addr, res->ai_addrlen)!=0)
         {
             perror("accept failed \n");
              return 2;
         }
        char buf[15];
        strcpy(buf,"TERMINATE ");


        strcat(buf,portno);

       int var= send(Client_to_Client,buf,sizeof(buf),0);
       //printf("TERMINATE sent %d \n",var);

        return 1;
    }


int RegisterToServer(char*ip,char*portno)
    {
        struct addrinfo hints,*res;
        int sockfd,status;
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        getaddrinfo(ip, portno, &hints, &res);
            // make a socket:
        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if(sockfd==-1)
        {
            perror("set sockopt ");
            exit(1);
        }
                // connect!
        status=connect(sockfd, res->ai_addr, res->ai_addrlen);
         if(status==-1)
        {
            perror("set connect ");
            exit(1);
        }



    return sockfd;
    }
