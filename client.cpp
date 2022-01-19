#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int local_time=0;
using namespace std;
int calculate_difference(int x);
int calculate_difference(int x) //function to calculate the time difference between clients and time daemon
{
    int daemonstime = x;
    int diff= daemonstime-local_time;
    return diff;   
}


int main()
{   
    //creating client socket
    int csock = socket(AF_INET, SOCK_STREAM,0);
    if (csock == -1)
    {
        cerr << "socket not created\n";
    }

    int port = 54004;
    string IP = "127.0.0.1";
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, IP.c_str(), &hint.sin_addr);

    //connecting to the server
    int connreq = connect(csock,(sockaddr *)&hint, sizeof(sockaddr_in));
        srand(time(NULL));
        local_time = rand() % 24;
        cout<<"Local time: "<<local_time<<endl;
        int daemon_time=0;        
        int receive_time_from_daemon = read(csock, &daemon_time, sizeof(daemon_time)); //receiving time from daemon
        if(receive_time_from_daemon==-1)
        {
            cout<<"Error in receinving time from daemon\n";
        }
        cout<<"Time received from daemon: "<<daemon_time<<endl;
        int difference = calculate_difference(daemon_time);
        int send_difference = htonl(difference);
        //send to server
        int send_resquest_to_server = write(csock, &send_difference, sizeof(send_difference));
        if(send_resquest_to_server==-1)
        {
            cout<<"Error in sending\n";
        }
        int synced_time=0; 
        int receive_synced_time_from_daemon = read(csock, &synced_time, sizeof(synced_time)); //receiving time from daemon
        if(receive_synced_time_from_daemon==-1)
        {
            cout<<"Error in receinving time from daemon\n";
        }
        cout<<"Difference received from daemon: "<<synced_time-local_time<<endl;
        local_time = synced_time; //local clock value adjusted
        cout<<"Synchronized time: "<<local_time<<endl;


    close(csock);
    
    return 0;
}
