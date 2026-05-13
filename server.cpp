#include <bits/stdc++.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

#define PORT 8080

map<string, vector<int>> rooms; // room no. -> collection of all client's socks 
map<int, string> clientRoom; // socks -> room no.
map<int, string> clientName; // socks -> user name
map<string, int> userSocket; // username -> socks // for private messaging 

//JOIN
void handle_join(int sock,string user,string room){
    rooms[room].push_back(sock);
    clientRoom[sock] = room;
    clientName[sock] = user;
    userSocket[user] = sock;
    cout<< user<< " joined room "<< room<< endl;
}
//GROUP
void handle_group_message(int sock,string room,string sender,string msg_hash,string encrypted_msg){
    cout << "[ROOM "<< room<< "] "<< sender<< " -> GROUP"<< endl;
    string full_msg =sender + "|" +msg_hash + "|" +encrypted_msg;
    for (int client : rooms[room]) {
        if (client != sock) {
            send(client,full_msg.c_str(),full_msg.size(),0);
        }
    }
}

//PRIVATE
void handle_private_message(string sender,string receiver,string msg_hash,string encrypted_msg){
    if (!userSocket.count(receiver)) {
        return;
    }

    int target_sock = userSocket[receiver];
    cout<<sender<< " -> "<< receiver<< " [PRIVATE]"<< endl;
    string full_msg ="[PRIVATE]" +sender + "|" +msg_hash + "|" + encrypted_msg;
    send(target_sock,full_msg.c_str(),full_msg.size(),0);
}

//REPORT 
void handle_report(string reporter,string target,string type){
    if (!userSocket.count(target)) {
        return;
    }
    int target_sock =userSocket[target];

    string warning_msg ="[MODERATION]|" +reporter + "|" +type;

    send(target_sock,warning_msg.c_str(),warning_msg.size(),0);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    bind(server_fd,(struct sockaddr*)&address,sizeof(address));
    listen(server_fd, 5);
    cout << "Server running...\n";
    vector<int> clients;
    fd_set readfds;

    while (true) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        int max_sd = server_fd;
        for (int sock : clients) {
            FD_SET(sock, &readfds);
            max_sd = max(max_sd, sock);
        }
        select(max_sd + 1,&readfds,NULL,NULL,NULL);

        // NEW CLIENT
        if (FD_ISSET(server_fd, &readfds)) {
            new_socket =accept(server_fd,NULL,NULL);
            clients.push_back(new_socket);
            cout << "New client connected\n";
        }

        // EXISTING CLIENTS
        for (int i = 0; i < clients.size(); i++) {
            int sock = clients[i];
            if (FD_ISSET(sock, &readfds)) {
                char buffer[4096] = {0};
                int valread = read(sock, buffer, 4096);

                if (valread <= 0) {

                    close(sock);
                    clients.erase(clients.begin() + i);
                    i--;

                    continue;
                }

                string msg(buffer,valread);
                vector<string> parts;
                stringstream ss(msg);
                string item;
                while(getline(ss, item, '|')){
                    parts.push_back(item);
                }
                if(parts.empty()){
                    continue;
                }
                // JOIN
                if(parts[0] == "JOIN" && parts.size() >= 3){
                    handle_join(sock,parts[1],parts[2]);
                }

                // GROUP
                else if(parts[0] == "MSG" && parts.size() >= 5){
                    handle_group_message(sock,parts[1],parts[2],parts[3],parts[4]);
                }

                // PRIVATE
                else if(parts[0] == "PMSG" && parts.size() >= 6){
                    handle_private_message(parts[2],parts[3],parts[4],parts[5]);
                }

                // REPORT
                else if(parts[0] == "REPORT" && parts.size() >= 4){
                    handle_report(parts[1],parts[2],parts[3]);
                }
            }
        }
    }

    return 0;
}