/**
 * @sarthakd_assignment3
 * @author  sarthak dundiraj bhat <sarthakd@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 */
#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<stdint.h>
#include <arpa/inet.h>
#include<sys/socket.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include <sys/select.h>
#include <sys/time.h>
#include<stdbool.h>
#include <netinet/in.h>
#include <unistd.h>
#include "../include/global.h"
#include "../include/logger.h"
#define BACKLOG 3
#define STDIN 0
#define MAXBUFLEN 100
/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */

  //[PA3] Update Packet Start
 typedef struct NodeInfo
{
uint32_t ip;
uint16_t port;
uint16_t padding;
uint16_t id;
uint16_t cost;
}NodeInfo;
 //[PA3] Update Packet End

//stores the information fo neigbour nodes
typedef struct NeighbourInfo
{
uint16_t id;
uint16_t cost;
int timestamp;
uint32_t myip;
uint16_t myport;
uint32_t neighbourip;
uint16_t neighbourportno;
uint16_t noofupdate;
bool IsDisable;
bool HasReceivedUDP;
}NeighbourInfo;
struct NextHop //store the information about next hop
{
int Destination;
int NextHop;
}NextHop;

//[PA3] Routing Table Start An array of structure is used for routing table
struct DistanceVector
{
int16_t sourceid;
struct NodeInfo RTofNeighbours[5];
}DistanceVector;
//[PA3] Routing Table End

typedef struct ForwardingTable //All the required data are enclosed in this data structure..It has all the data needed to calculat bellmanford
{
 struct NodeInfo RoutingTable[5];
 struct NextHop NextHopTable[5];//stores id  with next hop
 struct NeighbourInfo NeighbourTable[4];
 struct DistanceVector DVTable[4];  //stores the distances from the neighbours
 int noofnodes;
 int noofneighbours;
 int timeout;
 uint16_t mynodeid;
}ForwardingTable;
//lister and talker code has been copied from beejs guide
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
bool SendUDPPacket( char targetip[INET_ADDRSTRLEN],char portno[5],char *data,char var);
int CalculateCostBellmanFords(struct ForwardingTable *RouterInfo,int16_t serverid,uint16_t basecost,uint16_t nodecost,int nexthopid,uint16_t minval);
void RecalculateCosts(struct ForwardingTable *RouterTable);
int main(int argc, char **argv)
{
	/*Init. Logger*/
	cse4589_init_log();

	/*Clear LOGFILE and DUMPFILE*/
	fclose(fopen(LOGFILE, "w"));
	fclose(fopen(DUMPFILE, "wb"));
	/*Start Here*/

void ReadTopologyFile(int *nodes,int *neighbours,int *mynode,char *argv,int timeout,struct NodeInfo *RoutingTable,struct NeighbourInfo *data,struct NextHop *NextHop,struct DistanceVector *DVTable);
int BindandListen(char* portno);
bool MakeAndSendCostUpdate(int *nodes,int *neighbours,struct NodeInfo *RoutingTable,struct NeighbourInfo *data,bool sendudp);

int totalnodes;//total nodes
int neighbours;//total neighbours
int timeout;//user given timeout value
int MyNodeId; //servers node id is stored in the varialbe
int packets=0;
//Check if no of arguements is 5
if (argc != 5)
        {

            fprintf(stderr,"usage: ./assignment3 ­t <path­to­topology­file> ­i <routing­update­interval> \n");
            return 1;
        }

if ( (!strcmp(argv[1],"-T") || !strcmp(argv[1],"-t")) &&( !strcmp(argv[3],"-i") || !strcmp(argv[3],"-I"))) //Check if the arguements are t and i
    {
         int timeout=atoi(argv[4]);
         if(!timeout)
            {
            printf("Invalid timeout");
            return 1;
            }
         char line [ 128 ];
         FILE   * topology = fopen(argv[2], "r"); //reads the first line and defines the data structure
         fgets ( line, sizeof line, topology );
         totalnodes=atoi(line);
         fgets ( line, sizeof line, topology );
         neighbours=atoi(line);

         struct ForwardingTable RouterTable;
         fclose (topology);
        // printf("The size of table is %d \n",sizeof(RoutingTable));
         int NextHopTable[5][2]; //contains node and next hop destination
        // ReadTopologyFile(&totalnodes,&neighbours,&MyNodeId,argv[2],timeout,RoutingTable,NeighbourTable,NextHopTable);
         //Set all the values in distance vector table to inf
         int16_t x,y;
         for(x=0;x<4;x++) //intializating the values for vector tables
            {


                for(y=0;y<5;y++)
                    {
                        RouterTable.DVTable[x].RTofNeighbours[y].cost=0xffff; //default value is inf

                    }
            }
              for(y=0;y<5;y++)
                    {
              RouterTable.DVTable[x].RTofNeighbours[y].id=y+1;
              RouterTable.NextHopTable[y].Destination=y+1;
               RouterTable.NextHopTable[y].NextHop=y+1;
                    }
               //Reads the input topology file
         ReadTopologyFile(&totalnodes,&neighbours,&MyNodeId,argv[2],timeout,RouterTable.RoutingTable,RouterTable.NeighbourTable,RouterTable.NextHopTable,RouterTable.DVTable);

         RouterTable.mynodeid=MyNodeId;
         int i;
        RouterTable.noofnodes=totalnodes;
        RouterTable.noofneighbours=neighbours;
        RouterTable.DVTable[0].sourceid=MyNodeId;
        memcpy(RouterTable.DVTable[0].RTofNeighbours,RouterTable.RoutingTable,5*sizeof(NodeInfo)); //copis the data into routing table
//TestCODE
//      //****************************update the distance vector table************************
//printf("Neighbour table \n*********");
//        for(x=0;x<=RouterTable.noofneighbours;x++)
//            {
//                printf("\n Source ID:%d ",RouterTable.DVTable[x].sourceid);
////                for(y=0;y<RouterTable.noofnodes;y++)
////                    {
//                     printf("Cost:%d \n",RouterTable.DVTable[x].RTofNeighbours[y].cost);
////                    }
//            }
//            //******************************************************************************
//
//         RouterTable.DVTable[0].sourceid=MyNodeId;
//         memcpy(RouterTable.DVTable[0].RTofNeighbours,RouterTable.RoutingTable,5*sizeof(NodeInfo));
//         for(x=0;x<neighbours;x++)
//            {
//                printf("\n Source ID:%d ",RouterTable.DVTable[x].sourceid);
//                for(y=0;y<totalnodes;y++)
//                    {
//                        printf("Cost:%d \n",RouterTable.NeighbourTable[x].timestamp);
//                         RouterTable.NeighbourTable[x].timestamp=3*timeout;
//                    }
//            }
////
//
////         for(i=0;i<5;i++) //copies the routing table
////            {
////            RouterTable[0].RoutingTable[i]=RoutingTable[i]; //copies each element of Routing Table into forwarding table
////            }
////          for(i=0;i<4;i++) //copy the negibours table
////            {
////            RouterTable.NeighbourTable[i]=NeighbourTable[i];
////            }
//
//;
////
////         Printing the content with the size -TEST METHOD
////         ***********************Unit test begins for parser*******************
//       //  int i;
////         for(i=0;i<totalnodes;i++)
////         {
////         printf("The content is %d,%d,%d,%d \n",RoutingTable[i].id,RoutingTable[i].port,RoutingTable[i].ip,RoutingTable[i].cost);
////         printf("The size of each block is  %d,%d,%d,%d \n",sizeof(RoutingTable[i].id),sizeof(RoutingTable[i].port),sizeof(RoutingTable[i].ip),sizeof(RoutingTable[i].cost));
////         struct sockaddr_in sa;
////         char str[INET_ADDRSTRLEN];
////         memcpy(&sa.sin_addr,&RoutingTable[i].ip,4);
////         inet_ntop(AF_INET, &(sa.sin_addr), str, INET_ADDRSTRLEN);
////         printf("The ip obtained is %s \n",str);
////         }
////         for(i=0;i<neighbours;i++)
////         {
////
////         printf("The neighbour id :%d has cost  %d and timestamp %d sourceip:%d port:%d noofupdates:%d \n",RouterTable.NeighbourTable[i].id,RouterTable.NeighbourTable[i].cost,RouterTable.NeighbourTable[i].timestamp,RouterTable.NeighbourTable[i].myip,RouterTable.NeighbourTable[i].myport,RouterTable.NeighbourTable[i].noofupdate);
////         printf("The destination ip:%d and destination port is %d \n",RouterTable.NeighbourTable[i].neighbourip,RouterTable.NeighbourTable[i].neighbourportno);
////         }
//
//     //    MakeAndSendCostUpdate(&totalnodes,&neighbours,RoutingTable,NeighbourTable);
//         //**********************Unit test ends for parser*************************

//**********************************PARSERE COMPLETE**************************************
//Listen on the port for incoming segments
    int myport=ntohs(RouterTable.NeighbourTable[0].myport); //user port to lisetn on
    char portno[5];
    snprintf(portno,5,"%d",myport);
   //  strcpy(portno,"4950");
    printf("listening on port %s \n",portno);

    int sockfd=BindandListen(portno); //binds and listens to port by creating socket

    int fdmax,sockindex;
    fdmax=sockfd;
    struct timeval tv;
    tv.tv_sec=timeout;
    tv.tv_usec=0;

    fd_set readfds,master;
    FD_ZERO(&readfds);
    FD_SET(STDIN,&readfds);
    FD_SET(sockfd,&readfds);
    master=readfds;
    int serverfd=sockfd;
    printf("The router has started ,Please enter the commands\n");
    while(1)
    { // main accept() loop



        sockfd= serverfd;
        readfds=master;
        if(tv.tv_sec==0)
        {
        tv.tv_sec=timeout;
        tv.tv_usec=0;
        }

    int recv1=select(fdmax+1,&readfds,NULL,NULL,&tv); //main select loop

    if (recv1>=0) //Key borad input or a UDP packet has arrived or a timeout
    {
            if(FD_ISSET(fdmax,&readfds)) //reference beejs guide
                {
                packets++; //Recevice a UDP packet
                int numbytes;
                struct sockaddr_storage their_addr;
                char buf[MAXBUFLEN];
                socklen_t addr_len;
                char s[INET6_ADDRSTRLEN];


                addr_len = sizeof their_addr;
                if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
                    (struct sockaddr *)&their_addr, &addr_len)) == -1) {
                    perror("recvfrom");
                    exit(1);
                }

                printf("listener: got packet from %s\n",
                    inet_ntop(their_addr.ss_family,
                        get_in_addr((struct sockaddr *)&their_addr),
                        s, sizeof s));
                printf("listener: packet is %d bytes long\n", numbytes);
                buf[numbytes] = '\0';
                printf("listener: packet contains \"%s\"\n", buf);
                //    printf("The no of fields:%d \n",*(int16_t*)voidptr);
        //******************* Test code which checks the content of packets received*************************
                char *data=buf;
                char *dataptr=buf;
                void *voidptr=dataptr;
                int offset=12;
                int noofupdate=ntohs(*(int16_t*)voidptr);
           //     printf("The no of update field is:%d \n",noofupdate);
                dataptr=data+2;
                voidptr=dataptr;
                int sourceport=ntohs(*(int16_t*)voidptr);
              //  printf("The portno of sender:%d \n",sourceport);
                dataptr+=2;
                voidptr=dataptr;
                struct sockaddr_in sa;
                char sourceip[INET_ADDRSTRLEN];
                char str[INET_ADDRSTRLEN];
                int32_t source;
                memcpy(&source,dataptr,4);
                memcpy(&sa.sin_addr,dataptr,4);
                inet_ntop(AF_INET, &(sa.sin_addr), str, INET_ADDRSTRLEN);
             //   printf("The ip of sender is %s \n",str);
                int16_t basecost;
                int16_t senderid;
                //loop through neighbours table and find the id of the server
                for(i=0;i<neighbours;i++)
                    {
                         if(source==RouterTable.NeighbourTable[i].neighbourip && sourceport==RouterTable.NeighbourTable[i].neighbourportno)
                            {
                                //print the source server id
                                cse4589_print_and_log("RECEIVED A MESSAGE FROM SERVER %d\n",RouterTable.NeighbourTable[i].id);

                                senderid=RouterTable.NeighbourTable[i].id;
                                RouterTable.NeighbourTable[i].HasReceivedUDP=true;

                                RouterTable.NeighbourTable[i].timestamp=3*timeout; //mark to inf
                                basecost=RouterTable.NeighbourTable[i].cost;
                            }

                    }
            int j=0;
                for(i=0;i<noofupdate;i++)
                    { //printf("****************UPDATE %d**************\n",i+1);
                          dataptr=data+8+offset*i;
                          voidptr=dataptr;
                           memcpy(&sa.sin_addr,dataptr,4);
                           inet_ntop(AF_INET, &(sa.sin_addr), str, INET_ADDRSTRLEN);
                        //  printf("The ip:%s \n",str);
                          voidptr=dataptr+4;
                        //  printf("The portno:%d \n",ntohs(*(int16_t*)voidptr));
                          int destinationport=ntohs(*(int16_t*)voidptr);

                          voidptr=dataptr+6;
                         // printf("The padding:%d \n",*(int16_t*)voidptr);
                          voidptr=dataptr+8;
                        //  printf("The server id:%d \n",ntohs(*(int16_t*)voidptr));
                          int serverid=ntohs(*(int16_t*)voidptr);
                          voidptr=dataptr+10;
                          uint16_t nodecost=ntohs(*(int16_t*)voidptr);
                        //  printf("The cost id:%d \n",ntohs(*(int16_t*)voidptr));

                          cse4589_print_and_log("%15d%15d\n",serverid,nodecost);

                          //copy this information into distance vector table
                           for(j=0;j<=RouterTable.noofneighbours;j++)
                              {
                                    if(senderid==RouterTable.DVTable[j].sourceid)
                                        {
                                        RouterTable.DVTable[j].RTofNeighbours[i].cost=nodecost;

                                        }
                              }

                         //Find the current next hop id
                            int k,nexthop;
                            for(k=0;k<RouterTable.noofnodes;k++)
                                {
                                    if(RouterTable.NextHopTable[k].Destination==serverid)
                                        {
                                        nexthop=RouterTable.NextHopTable[k].NextHop;
                                        }
                                }
                             for(k=0;k<neighbours;k++)
                                    {
                                     if(RouterTable.DVTable[k].sourceid==senderid)
                                     {
                                    RouterTable.DVTable[k].RTofNeighbours[i].cost=nodecost;
                                     }
                                    }

                          //Recalculate cost using bellman fords equation
                        //  CalculateCostBellmanFords(&RouterTable,senderid,basecost,nodecost,serverid,true);


                    }
        //*******************************TEST CODE ENDS****************************
               RecalculateCosts(&RouterTable);
                //update the distance vector table
                 memcpy(RouterTable.DVTable[0].RTofNeighbours,RouterTable.RoutingTable,5*sizeof(NodeInfo));
                }
           else  if(FD_ISSET(STDIN,&readfds))
                {

                    char incoming[128]; //reads the incoming messages
                    char incomingsplit[15][30];
                    char commandinput[50];
                    char incomingcopy[128];
                    memset(incomingsplit,0,sizeof(incomingsplit));
                    memset(incoming,0,sizeof(incoming));
                    memset(commandinput,0,sizeof(commandinput));
                    read(0,incoming,sizeof incoming);
                   // strcpy(incomingcopy,incoming);

                     char *token;
                     token=strtok(incoming," ");
                     strcpy(commandinput,token);

                   char *temp=incoming;//remove the next line feed
                    while(*temp!='\n') //convert to upper case
                        {
                          *temp=toupper(*temp);
                          temp++;
                        }
                    *temp='\0';

                    //Remove the \n character from command input if it cotains
                    temp=commandinput;
                    for(i=0;i<strlen(commandinput);i++)
                        {
                            if(commandinput[i]=='\n')
                                {
                                commandinput[i]='\0';
                                }

                        }
                    //start tokenizer with incoming as input string

                       int count=0;
                       while(token!=NULL)
                        {
                          strcpy(incomingsplit[count],token);
                          token = strtok(NULL, " ");//gets the new value of token
                          count++;
                        }

                    //check for different commands
                   if(strcmp(incomingsplit[0],"UPDATE")==0)
                     {
//                            //****************************update the distance vector table************************
//                                       for(x=0;x<=RouterTable.noofneighbours;x++)
//            {
//                printf("\n Source ID:%d ",RouterTable.DVTable[x].sourceid);
//                for(y=0;y<RouterTable.noofnodes;y++)
//                    {
//                        printf("Cost:%d \n",RouterTable.DVTable[x].RTofNeighbours[y].cost);
//                    }
//            }
//            //******************************************************************************



                       printf("update command received \n");
                       //check if server id2 is a neighbour if yes update teh cost to id2
                       int sourceid=atoi(incomingsplit[1]);
                       int destinationid=atoi(incomingsplit[2]);
                       uint16_t newcost;
                       if(strcmp(incomingsplit[3],"INF")==0)
                        {
                        newcost=0xffff;
                        }
                       else
                        {
                        newcost=atoi(incomingsplit[3]);
                        }
                       int i=0;
                       bool isneighbour=false;

                       if(sourceid==MyNodeId)
                       {
                        //check if destination is a neigbour
                       for(i=0;i<neighbours;i++)
                            {
                                if(RouterTable.NeighbourTable[i].id==destinationid)
                                    {
                                    isneighbour=true;
                                    RouterTable.NeighbourTable[i].cost=newcost;
                                    }
                            }


                        for(i=0;i<totalnodes;i++ && isneighbour)
                            {
                                if(RouterTable.DVTable[0].RTofNeighbours[i].id==destinationid)
                                {
                                RouterTable.DVTable[0].RTofNeighbours[i].cost=newcost;
                                }

                            }
                          if(isneighbour)
                              {


                               printf("Neighobur exists:updating the cost \n");
                              RecalculateCosts(&RouterTable);
                                 cse4589_print_and_log("%s:SUCCESS\n",commandinput);
                              }
                           else
                              {
                                cse4589_print_and_log("Neighbour not found \n");
                               }
                          }
                        else //prints that the source id is invalid
                            {
                            cse4589_print_and_log("%s:%s\n",commandinput,"The source id is invalid");
                            }
                       }
                   else if(strcmp(incomingsplit[0],"STEP")==0)
                        {
                           MakeAndSendCostUpdate(&RouterTable.noofnodes,&RouterTable.noofneighbours,RouterTable.RoutingTable,RouterTable.NeighbourTable,true);
                           cse4589_print_and_log("%s:SUCCESS\n",commandinput);
                        }
                    else if(strcmp(incomingsplit[0],"PACKETS")==0)
                        {
                        cse4589_print_and_log("%d\n",packets);
                        cse4589_print_and_log("%s:SUCCESS\n",commandinput);
                        packets=0;
                        }
                    else if(strcmp(incomingsplit[0],"DISPLAY")==0)
                        {
                            for(i=0;i<totalnodes;i++)
                                {
                                int j,costbasedhop,cost;
                                for(j=0;j<totalnodes;j++) //loop through next hop array to get id
                                    {
                                    if(RouterTable.RoutingTable[i].id==RouterTable.NextHopTable[j].Destination)
                                        {

                                         cost=RouterTable.RoutingTable[i].cost;

                                        if(cost==0xffff)
                                            {
                                                costbasedhop=-1;
                                            }
                                         else
                                            {
                                                costbasedhop=RouterTable.NextHopTable[i].NextHop;
                                            }


                                        }


                                    } //closing inner for
                                cse4589_print_and_log("%15d%15d%15d\n",RouterTable.RoutingTable[i].id,costbasedhop,cost) ;
                                }


                         cse4589_print_and_log("%s:SUCCESS\n",commandinput);
                        }
                    else if(strcmp(incomingsplit[0],"DISABLE")==0)
                        {

                                  int destinationid=atoi(incomingsplit[1]);
                                    bool isfound=false;
                                     for(x=0;x<=neighbours;x++ && !isfound)
                                        {
                                           if(destinationid==RouterTable.NeighbourTable[x].id)
                                            {
                                                    isfound=true;
                                                    RouterTable.NeighbourTable[x].IsDisable=true;
                                                    RouterTable.NeighbourTable[x].cost=0xffff;
                                            }
                                        }//closing for loop


                                        if(!isfound)
                                        {
                                        cse4589_print_and_log("%s:%s\n",commandinput,"Neighobur not found");
                                        }
                                        else
                                        {
                                                 for(x=0;x<totalnodes;x++) //Updating dv table and recalculating costs
                                                    {

                                                    if(RouterTable.DVTable[0].RTofNeighbours[x].id==destinationid)
                                                    {
                                                        RouterTable.DVTable[0].RTofNeighbours[x].cost=0xffff;
                                                    }

                                                    }

                                              RecalculateCosts(&RouterTable);
                                               cse4589_print_and_log("%s:SUCCESS\n",commandinput);
                                        }

                         }//closing disable if

                    else if(strcmp(incomingsplit[0],"CRASH")==0)
                        {
                            while(1)
                                {
                                }
                        }
                    else if(strcmp(incomingsplit[0],"DUMP")==0)
                        {

                            MakeAndSendCostUpdate(&RouterTable.noofnodes,&RouterTable.noofneighbours,RouterTable.RoutingTable,RouterTable.NeighbourTable,false);
                            cse4589_print_and_log("%s:SUCCESS\n",commandinput);
                        }
                    else if(strcmp(incomingsplit[0],"ACADEMIC_INTEGRITY")==0)
                        {
                          cse4589_print_and_log("%s:%s",commandinput,"I have read and understood the course academic integrity policy located at http://www.cse.buffalo.edu/faculty/dimitrio/courses/cse4589_f14/index.html#integrity");
                          cse4589_print_and_log("%s:SUCCESS\n",commandinput);
                          printf("\n");
                        }
                     else
                        {
                           printf("Command not found \n");
                        }
                }

           else if (recv1==0)
                {
                    //Reduce the timestamp values of each neighbour and chekc if its 0
                    for(i=0;i<=neighbours && RouterTable.NeighbourTable[i].HasReceivedUDP;i++ )
                    {
                      //  printf("Old timeo out value is %d \n",RouterTable.NeighbourTable[i].timestamp);

                        RouterTable.NeighbourTable[i].timestamp-=timeout;
                        //printf("Decrementing counter for ServerID %d /n",RouterTable.NeighbourTable[i].id);
                      //  printf("New timeout value is %d",RouterTable.NeighbourTable[i].timestamp);
                        if(RouterTable.NeighbourTable[i].timestamp<0)
                            {
                                //mark the neighbours cost as inf


                               RouterTable.NeighbourTable[i].cost=0xffff;
                               int id=RouterTable.NeighbourTable[i].id-1;
                               RouterTable.DVTable[0].RTofNeighbours[id].cost=0xffff;
                               RecalculateCosts(&RouterTable);
                               RouterTable.NeighbourTable[i].timestamp=3*timeout;
                             //  printf("###############MARKINF ###################### for %d Packts :%d\n",RouterTable.NeighbourTable[i].id,packets);

                            }

                          // cse4589_print_and_log("%s:SUCCESS\n",commandinput);


                    }
                    //If zero than mark the neigbour cost as inf in both routing and neighbours table
                    //re calculate the costs of all nodes
                    //Send the updated table to all the nighbours

                    MakeAndSendCostUpdate(&RouterTable.noofnodes,&RouterTable.noofneighbours,RouterTable.RoutingTable,RouterTable.NeighbourTable,true);

                }


      }//closing if
      }//closing while
    }//Closing main if
else
    {
        printf(" Invalid arguements \n");
    }

//main returns 0
	return 0;

}
void ReadTopologyFile(int *nodes,int *neighbours,int *mynode,char *filepath,int timeout,struct NodeInfo *RoutingTable,struct NeighbourInfo *NeighbourTable,struct NextHop *NextHop,struct DistanceVector *DVTable)
{
//Start reading the topology file present in argv[2];
            FILE   * topology = fopen(filepath, "r");
            char line [ 128 ];
            memset(line,0,sizeof(line));
        //Read the time interval and check if its integer

            if (topology!=NULL && timeout!=0)
                {
               // printf("You are all set \n");
                //Read the first two lines and get the values of total nodes and total neighbours Reference-https://www.daniweb.com/software-development/c/code/216411/reading-a-file-line-by-line

                int linecount=1;
                while ( fgets ( line, sizeof line, topology ) != NULL ) /* read a line */
                    {

                    if(linecount==1)
                    {
                       *nodes=atoi(line);
                       unsigned int totalnodes=*nodes;

                    }
                    else if(linecount==2)
                    {
                      *neighbours=atoi(line);
                    }
                    else if(linecount<=*nodes+2) //reads the ip address and port of all the neighbours and updates the data structure
                    {
                                              //Create an array of structure to store the data

                       char *token;
                       int count=0;
                       token=strtok(line," ");
                    int id;
                       while( token != NULL )  //copy the content of file into array of strcuture
                       {
                         // printf("token=%s\n", token);
                          if(count==0) //copy the 16 bit ID of the server
                           {
                              id=atoi(token);
                               RoutingTable[id-1].id=atoi(token);
                               RoutingTable[id-1].cost=0xffff;
				               RoutingTable[id-1].padding=0;
				               NextHop[id-1].NextHop=-1; //marks next hope as -1
				               NextHop[id-1].Destination=atoi(token);//marks next hop sourece id
                               int16_t var=atoi(token);
                             //  printf("%d",var);
                           }
                           else if(count==1)//copy the 32 bit ip
                           {
                               struct sockaddr_in sa;
                               if(inet_pton(AF_INET,token,&(sa.sin_addr)))
                               {
                                    //RoutingTable[linecount-3].ip=sa.sin_addr;
                                   memcpy(&RoutingTable[id-1].ip,&sa.sin_addr,sizeof(sa.sin_addr));
                               }
                               else
                               {
                                    printf("/n invalid ip");
                               }
                           }
                           else if(count==2) //copy the 16 bit port number
                           {
                                RoutingTable[id-1].port=htons(atoi(token));
                           }

                          token = strtok(NULL, " ");
                          count++;
                       }

                    }

                    else //Read the last portion of the topology file and update the costs
                    {
                       char *token;
                       int count=0;
                       int i=0;
                       token=strtok(line," ");
                       *mynode=atoi(token); //gets your node id
                       token=strtok(NULL," ");
                       int16_t nodeid=atoi(token);//gets the neighbours id

                      // RoutingTable[0][0]=*mynode;
                // NextHopTable[*mynode-1][0]=*myno
                       NeighbourTable[linecount-(*nodes+3)].id=atoi(token); //updates the node id into neighbour talbe
                       DVTable[linecount-(*nodes+2)].sourceid=atoi(token);

                       int neighbour=atoi(token);//gets the neighbours node id
                       token=strtok(NULL," ");
                       int16_t cost=atoi(token);
                       NeighbourTable[linecount-(*nodes+3)].cost=atoi(token);//updates the cost into neighbour table
                       NeighbourTable[linecount-(*nodes+3)].timestamp=3*timeout;

                       for(i=0 ;i<*nodes;i++) //updates the Routing table with the cost to its neigbours
                       {
                           NeighbourTable[linecount-(*nodes+3)].IsDisable=false;
                           NeighbourTable[linecount-(*nodes+3)].HasReceivedUDP=false;
                           if(RoutingTable[i].id==nodeid)
                           {
                                RoutingTable[i].cost=cost;//updates the cost into routing table
                                NeighbourTable[linecount-(*nodes+3)].neighbourip=RoutingTable[i].ip;
                                NeighbourTable[linecount-(*nodes+3)].neighbourportno=ntohs(RoutingTable[i].port);
                           }

                           if(RoutingTable[i].id==*mynode)
                           {
                               NeighbourTable[linecount-(*nodes+3)].myip=  RoutingTable[i].ip;//updates the cost into routing table
                               NeighbourTable[linecount-(*nodes+3)].myport=  RoutingTable[i].port;


                               int16_t noofupdate=*nodes;
                               NeighbourTable[linecount-(*nodes+3)].noofupdate= noofupdate;//*neighbours changed to *no of update
                              // NeighbourTable[linecount-(*nodes+3)].cost=0;
                               RoutingTable[i].cost=0;//updates the cost to itself as 0
                            }

                            if(NextHop[i].Destination==nodeid) //copies the next hop for all the neighbours
                                {
                                NextHop[i].NextHop=nodeid;
                                }
                            if(NextHop[i].Destination==*mynode)
                                {
                               NextHop[i].NextHop=*mynode;
                                }
                       }
                    }
                 //   fputs ( line, stdout ); /* write the line */
                    linecount++;
                    }

                }
            else
                {
                printf("Invalid file path or timeout \n");
                }
fclose (topology);
}

int BindandListen(char* portno) //Method copied from beejs guide
    {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, portno, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);


   //close(sockfd);

    return sockfd;
    }

    bool MakeAndSendCostUpdate(int *nodes,int *neighbours,struct NodeInfo *RoutingTable,struct NeighbourInfo *NeighbourTable,bool sendudp)
    {
    int i,j;
    int nofnodes=*nodes;
    int var=nofnodes*sizeof(NodeInfo);
    var=var+8;

    char *data=malloc(var*sizeof(char));

    void *voidptr=data;

   //short data[100];
  //  memset(data,0,100);

   // printf("The size of memory is %d \n",sizeof(data));
    char *dataptr=data;
    NeighbourTable[0].noofupdate=htons(NeighbourTable[0].noofupdate);
//copies the first 64 bits of the packet
    memcpy(dataptr,&NeighbourTable[0].noofupdate,2);
    NeighbourTable[0].noofupdate=ntohs(NeighbourTable[0].noofupdate);
    memcpy(dataptr+2,&NeighbourTable[0].myport,2);

    memcpy(dataptr+4,&NeighbourTable[0].myip,4);


    dataptr=dataptr+8;

    int offset=12;//size of each block to be appended

   // for(i=0;i<*neighbours;i++) //send an update to all the negihbours
     //   {
           for(j=0;j<*nodes;j++) //search each node in the nodes table and for more information needed in pakcet
              {
       //       if(NeighbourTable[i].id==RoutingTable[j].id)
         //       {
                    //Create a packet with routing table [j].id
                  //  printf("j value:%d \n",j);
              RoutingTable[j].id=htons(RoutingTable[j].id); //converts to network format
              RoutingTable[j].cost=htons(RoutingTable[j].cost);
			  memcpy(dataptr+offset*j,&RoutingTable[j],sizeof(NodeInfo));
		      RoutingTable[j].id=ntohs(RoutingTable[j].id); //converts back to host format
              RoutingTable[j].cost=htons(RoutingTable[j].cost);
           //     }
              }
     //   }
  //--------------------------------test code----------------------------
//    printf("The no of fields:%d \n",*(int16_t*)voidptr);
//    dataptr=data+2;
//    voidptr=dataptr;
//    printf("The portno of sender:%d \n",*(int16_t*)voidptr);
//    dataptr+=2;
//    voidptr=dataptr;
//    printf("The ip of sender:%d \n",*(int32_t*)voidptr);
//
//    for(i=0;i<*neighbours;i++)
//        {
//              dataptr=data+8+offset*i;
//              voidptr=dataptr;
//              printf("The ip:%d \n",*(int32_t*)voidptr);
//              voidptr=dataptr+4;
//              printf("The portno:%d \n",*(int16_t*)voidptr);
//              voidptr=dataptr+6;
//              printf("The padding:%d \n",*(int16_t*)voidptr);
//              voidptr=dataptr+8;
//              printf("The server id:%d \n",*(int16_t*)voidptr);
//              voidptr=dataptr+10;
//              printf("The cost id:%d \n",*(int16_t*)voidptr);
//        }
    //--------------------------------test cose-------------------------------


if(sendudp) //make a udp socket and send the update to all the neighbours
    {

           for(i=0;i<*neighbours && !NeighbourTable[i].IsDisable;i++ )
                {
                        int myport=NeighbourTable[i].neighbourportno;
                        char portno[5];
                        snprintf(portno,5,"%d",myport);
                        char destip[INET_ADDRSTRLEN];
                        struct sockaddr_in sa;

                         memcpy(&sa.sin_addr,&NeighbourTable[i].neighbourip,4);
                         inet_ntop(AF_INET, &(sa.sin_addr),destip , INET_ADDRSTRLEN);

                        printf("Sending packet to %s \n",portno);
                        SendUDPPacket(destip,portno,data,var);
                }
    }
 else
    {
    int ret_val=cse4589_dump_packet(data,var);
    if(ret_val)
        {
        return true;
        }
    else
        {
        return false;
        }
    }
    free(data);
    }

bool SendUDPPacket( char targetip[INET_ADDRSTRLEN],char portno[5],char *data,char var)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;

//reference beejs gudie for talker and listener code
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(targetip, portno, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "talker: failed to bind socket\n");
        return 2;
    }
char msg[10];
strcpy(msg,"Hello world");
    if ((numbytes = sendto(sockfd, data, var, 0,
             p->ai_addr, p->ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1);
    }

    freeaddrinfo(servinfo);

    printf("talker: sent %d bytes to %s and port no %s\n", numbytes,targetip,portno);
    close(sockfd);

    return true;

}
//compares the current cost with new cost and updates the relavant tables
int CalculateCostBellmanFords(struct ForwardingTable *RouterInfo,int16_t serverid,uint16_t basecost,uint16_t nodecost,int nexthopid,uint16_t minval)
{

    int i=0;
    for(i=0;i<RouterInfo[0].noofnodes;i++)
        {
            if(RouterInfo[0].RoutingTable[i].id==nexthopid)
                {
                    //using bellmanfords equtation to calcualte minimum cost
//                    if(i=0) //code added 26th nov 7:17
//                        {
//                        RouterInfo[0].RoutingTable[i].cost=0xffff;
//                        }

                    int totalcost=basecost+nodecost;
                 //  printf("Comparing old cost:%d with new cost %d+%d \n",minval,basecost,nodecost);
                    if(minval>=basecost+nodecost)
                        {
                            //update the routing table and change the next hop to servre id
                        //    printf("Changing the costs \n");

                            if(basecost+nodecost>65534)
                            {
                               RouterInfo[0].RoutingTable[i].cost=0xffff;
                            }
                            else
                            {
                                RouterInfo[0].RoutingTable[i].cost=basecost+nodecost;
                                minval=basecost+nodecost;
                            }

                            int destnode=RouterInfo[0].RoutingTable[i].id;
                            int k;
                            for(k=0;k<RouterInfo[0].noofnodes;k++)
                                {
                                    if(RouterInfo[0].NextHopTable[k].Destination==destnode)
                                        {
                                        RouterInfo[0].NextHopTable[k].NextHop=serverid;
                                        }
                                }

                        }

                   // minval=RouterInfo[0].RoutingTable[i].cost;
                }

        }
 return minval;
}

void RecalculateCosts(struct ForwardingTable *RouterTable)
    {
        int i,j,k,x,y;
        uint16_t basecost,nodecost,destnode;
//
//        for(i=0;i<RouterTable[0].noofneighbours;i++)
//         {
//
//         printf("The neighbour id :%d has cost  %d and timestamp %d sourceip:%d port:%d noofupdates:%d \n",RouterTable[0].NeighbourTable[i].id,RouterTable[0].NeighbourTable[i].cost,RouterTable[0].NeighbourTable[i].timestamp,RouterTable[0].NeighbourTable[i].myip,RouterTable[0].NeighbourTable[i].myport,RouterTable[0].NeighbourTable[i].noofupdate);
//         printf("The destination ip:%d and destination port is %d \n",RouterTable[0].NeighbourTable[i].neighbourip,RouterTable[0].NeighbourTable[i].neighbourportno);
//         }

        // current node refers to the neighbour and itself,base cost is the cost form my node to currentnode
        //destnode refers to the final distination node,node cost refers to cost from current node to destination node
    memcpy(RouterTable[0].RoutingTable,RouterTable[0].DVTable[0].RTofNeighbours,5*sizeof(NodeInfo));
        for(x=0;x<=RouterTable[0].noofneighbours;x++)
            {
               // printf("Source ID:%d \n",RouterTable[0].DVTable[x].sourceid);
                for(y=0;y<RouterTable[0].noofnodes;y++)
                    {
                      //  printf("%15d",RouterTable[0].DVTable[x].RTofNeighbours[y].cost);
                      //  printf("ID:%d \n",RouterTable[0].DVTable[x].RTofNeighbours[y].id);
                    }
                 printf("\n");
            }


        for(i=0;i<RouterTable[0].noofnodes;i++) //iterates through each column of distance vector
            {

               //RouterTable[0].RoutingTable[i].cost=0xffff;
               uint16_t minval=0xffff;
               int count;
               for(count=0;count<RouterTable[0].noofneighbours;count++)
                {
                    if(RouterTable[0].NeighbourTable[count].id==RouterTable[0].DVTable[0].RTofNeighbours[i].id)
                        {
                            minval=RouterTable[0].NeighbourTable[count].cost;
                          // printf("The min val obtained initially is %d \n",minval);
                        }
                    else if(RouterTable[0].DVTable[0].RTofNeighbours[i].id==RouterTable[0].mynodeid)
                        {
                        //   printf("Entering elseif :Node id found is %d\n",RouterTable[0].mynodeid);
                            minval=0;
                        }
                }

            for(j=1;j<=RouterTable[0].noofneighbours;j++) //iterates through each row of distance vector
                {
                    //printf("i:%d,j:%d \n",i,j);

                    uint16_t currentnode=RouterTable[0].DVTable[j].sourceid;//gets the row value
                    //basecost=RouterTable[0].DVTable[0].RTofNeighbours[currentnode-1].cost;


                    for (k=0;k<RouterTable[0].noofneighbours;k++)
                    {
                        if(RouterTable[0].NeighbourTable[k].id==currentnode)
                            {
                                basecost=RouterTable[0].NeighbourTable[k].cost;
                            }



                    } //closing for

                  //  basecost=RouterTable[0].DVTable[j].RTofNeighbours[i].cost;//gets the cost to get to row value
                    destnode=RouterTable[0].DVTable[0].RTofNeighbours[i].id; //gets the column value
                  // pr+intf("Destnode:%d \n",destnode);
                    for(k=0;k<=RouterTable[0].noofneighbours;k++)
                        {
                            if(RouterTable[0].DVTable[k].sourceid==currentnode)
                                {
                                    nodecost=RouterTable[0].DVTable[k].RTofNeighbours[destnode-1].cost;
                                }
                        }

              // printf("############Calculating cost to %d through %d ############\n",destnode,currentnode);
                   minval= CalculateCostBellmanFords(RouterTable,currentnode,basecost,nodecost,destnode,minval);


                }


            }
  memcpy(RouterTable[0].DVTable[0].RTofNeighbours,RouterTable[0].RoutingTable,5*sizeof(NodeInfo));

    }
















