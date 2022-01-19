#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#define number_of_clients 5
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
using namespace std;
int thread_ctr;
int daemon_time=0;
int client_fds[number_of_clients]={0};
int time_difference [number_of_clients] ={0};
int time_difference_ctr=0;
void communicate_with_clients(int x);
int sync_time();

int sync_time() //function to calculate the synchronized time
{   
    int sum=0;
    int average=0;
    if(thread_ctr==number_of_clients)
    {
        for(int i=0;i<number_of_clients;i++)
        {
            sum = sum+time_difference[i];
        }
    average=sum/(number_of_clients+1);
    daemon_time = daemon_time +average;
    cout<<"\nDaemon's adjusted clock after synchronization: "<<daemon_time<<endl;    
    }
    int updated_time= daemon_time;
    for (int i = 0; i < number_of_clients; i++)
    {
          
            int send_synced_time_to_client = write(client_fds[i], &updated_time, sizeof(updated_time)); //sending adjusted clock value to all the clients 
            if(send_synced_time_to_client==-1)
            {
                cout<<"Error in sending time to clients\n";
            }
    }
     cout<<"Clients informed.....!!"<<endl;
   
    return 0;
}


void *operations(void *arg) {
    int client_socket = *((int *)arg);
    char buff[4096];
    string msg_to_client;
    cout << "Client connected"
         << " " << client_socket << " "
         << "Thread ID"
         << " " << pthread_self() << endl;
    while (thread_ctr!=(number_of_clients)) //checking if all the clients are connected or not
    {
        continue;
    }
    
    communicate_with_clients(client_socket);
    pthread_exit(NULL);
}

void communicate_with_clients (int x)
{  
    int send_time_to_client = write(x, &daemon_time, sizeof(daemon_time)); //sending local time to clients
        if(send_time_to_client==-1)
        {
            cout<<"Error in sending time to clients\n";
        }
    cout<<"Time " <<daemon_time<< " sent to client with file descriptor: "<<x<<endl;
    sleep(0.3);
    //wait for response from clients, they will send the difference
    
        int temp_time_diff=0;
        int y;
        int receive_diff_from_client = read(x, &y, sizeof(y)); //receiving time differences from clients
        temp_time_diff= ntohl(y);
        if(receive_diff_from_client==-1)
        {
            cout<<"Error in receiving time difference from client with file descriptor: "<<x<<endl;
        }
        int time_diff = temp_time_diff;
        cout<<"Time differece "<<time_diff<<" recevied from client with desciptor:"<<x<<endl; 
        time_difference[time_difference_ctr]=time_diff;        
        time_difference_ctr++;
        //cout<<"Time difference counter: "<<time_difference_ctr<<endl;
        

        if(time_difference_ctr==number_of_clients)
        {
            sync_time(); //function which calculates average of all the time differences            

        }

        
}

int main() {  
    srand(time(NULL));
    daemon_time = rand() % 24;
    cout<<"Daemon's local time: "<<daemon_time<<endl;
    //For socket part I have referred below video
    //https://www.youtube.com/watch?v=cNdlrbZSkyQ
    pthread_t newthread[number_of_clients];
    //Create a server socket

    int listening = 0;
    listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening == -1) {
        cerr << "socket not created\n";
    }

    else {
        cout << "Socket created with FD: " << listening << "\n";
    }

    int reuse_address = 1;
    //Below code is referred from: https://pubs.opengroup.org/onlinepubs/000095399/functions/setsockopt.html
    //To reuse the port
    if (setsockopt(listening, SOL_SOCKET, SO_REUSEPORT, &reuse_address, sizeof(reuse_address)) != 0) {
        cout << "Failed to reuse the port" << endl;
    }

    //Bind socket on ip & port
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(54004);
    inet_pton(AF_INET, "127.0.0.1", &hint.sin_addr);

    if (bind(listening, (sockaddr *)&hint, sizeof(hint)) == -1) {
        cerr << "Binding failed\n";
    }

    //Make the socket listen
    if (listen(listening, 5) == -1) {
        cerr << "Listening failed\n";
    }

    //accpet the connection
    sockaddr_in client;
    socklen_t clientsize = sizeof(client);
    char host[NI_MAXHOST];
    char svc[NI_MAXSERV];
    int clientsocket[number_of_clients];
    for (int j =0; j < number_of_clients; j++) {
        clientsocket[j] = 0;
    }
    thread_ctr=0;
    while (true) {
        while (clientsocket[thread_ctr] = accept(listening, (struct sockaddr *)&client, (socklen_t *)&clientsize)) {
            if (clientsocket[thread_ctr] == -1) {
                cerr << "Unable to connect with client\n";
                continue;
            } else {
                pthread_create(&newthread[thread_ctr], NULL, operations, &clientsocket[thread_ctr]);
                client_fds[thread_ctr]=clientsocket[thread_ctr];
                thread_ctr++;
            }
        }
        for(int p = 0; p < number_of_clients; p++)
                {
                    pthread_join(newthread[p], NULL);       //waiting for all threads to finish                    
                }
            cout << "closing " << clientsocket << endl;
    }
    
    return 0;
}