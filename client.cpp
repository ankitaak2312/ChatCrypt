#include <bits/stdc++.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

#define PORT 8080

// MODERATION
set<string> mutedUsers;
set<string> blockedUsers;
map<string, int> reputation;

//HEX 
string to_hex(const string &input) {
    static const char hex[] = "0123456789ABCDEF";
    string output;

    for(unsigned char c : input){

        output += hex[c >> 4];
        output += hex[c & 15];
    }

    return output;
}

string from_hex(const string &input) {
    string output;
    for (size_t i = 0; i < input.length(); i += 2) {
        string byte =input.substr(i, 2);
        char chr =(char) strtol(byte.c_str(),nullptr,16);
        output += chr;
    }

    return output;
}

//XOR
string xor_encrypt_decrypt(string data,char key){
    for (char &c : data) {
        c ^= key;
    }
    return data;
}

// HASH

string generate_hash(const string &msg){
    hash<string> hasher;

    return to_string(hasher(msg));
}

// ML API 
string call_ml_api(string message){
    string command =
        "curl -s -X POST http://127.0.0.1:5000/analyze "
        "-H \"Content-Type: application/json\" "
        "-d '{\"message\":\"" + message + "\"}'";

    string result;

    char buffer[256];
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe)
        return "ML ERROR";

    while (fgets(buffer,sizeof(buffer),pipe) != NULL){
        result += buffer;
    }
    pclose(pipe);
    return result;
}

int main() {

    int sock =socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET,"127.0.0.1",&serv_addr.sin_addr);
    connect(sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr));

    string username;
    string room;
    string key_input;
    cout << "Enter username: ";
    cin >> username;
    cout << "Enter room: ";
    cin >> room;
    cout << "Enter encryption key: ";
    cin >> key_input;
    cin.ignore();
    char key = key_input[0];

    // JOIN 

    string join_msg ="JOIN|" +username +"|" +room;
    send(sock,join_msg.c_str(),join_msg.size(),0);
    fd_set readfds;
    while (true) {
        FD_ZERO(&readfds);
        FD_SET(0, &readfds);
        FD_SET(sock, &readfds);
        select(sock + 1,&readfds,NULL,NULL,NULL);

        //SEND
        if (FD_ISSET(0, &readfds)) {
            string msg;
            getline(cin, msg);

            // PRIVATE MESSAGE
            if(msg.rfind("/pm ", 0)== 0){
                stringstream ss(msg);   
                string cmd;
                string receiver;
                ss >> cmd >> receiver;
                string private_msg;
                getline(ss, private_msg);
                if (!private_msg.empty() && private_msg[0] == ' '){
                    private_msg.erase(0, 1);
                }

                string encrypted_raw = xor_encrypt_decrypt(private_msg,key);
                string encrypted =to_hex(encrypted_raw);
                string msg_hash =generate_hash(encrypted);

                string send_msg ="PMSG|" +room + "|" +username + "|" +receiver + "|" +msg_hash + "|" +encrypted;

                send(sock,send_msg.c_str(),send_msg.size(),0);
            }

            // GROUP MESSAGE
            else {

                string encrypted_raw =xor_encrypt_decrypt(msg,key);
                string encrypted =to_hex(encrypted_raw);
                string msg_hash = generate_hash(encrypted);

                string send_msg ="MSG|" +room + "|" +username + "|" +msg_hash + "|" +encrypted;

                send(sock,send_msg.c_str(),send_msg.size(),0);
            }
        }

        // RECEIVE 

        if (FD_ISSET(sock, &readfds)) {

            char buffer[4096] = {0};

            int valread =read(sock,buffer,4096);
            if(valread <= 0){
                cout << "\nDisconnected from server.\n";
                break;
            }

            string received(buffer,valread);

            //MODERATION NOTICE

            if(received.rfind("[MODERATION]|",0) == 0){
                vector<string> mod_parts;
                stringstream ms(received);
                string temp;
                while(getline(ms, temp, '|')){
                    mod_parts.push_back(temp);
                }
                string reporter = mod_parts[1];
                string type = mod_parts[2];
                cout << "\n⚠️ Moderation Notice\n";
                cout << "Reported by: "<< reporter<< endl;

                cout << "Reason: "<< type<< endl;

                continue;
            }

            bool is_private = false;

            // PRIVATE
            if (received.rfind("[PRIVATE]",0) == 0){
                is_private = true;
                received = received.substr(9);
            }
            vector<string> parts;
            stringstream ss(received);
            string item;

            while(getline(ss, item, '|')){
                parts.push_back(item);
            }

            if (parts.size() >= 3) {

                string sender =parts[0];
                string received_hash =parts[1];
                string encrypted_msg =parts[2];

                // BLOCKED / MUTED
                if(blockedUsers.count(sender)|| mutedUsers.count(sender)){
                    continue;
                }

                // HASH CHECK
                string computed_hash =generate_hash(encrypted_msg);
                if(computed_hash!= received_hash){
                    cout << "\n⚠️ MESSAGE TAMPERING DETECTED\n";
                    continue;
                }

                // DECRYPT
                string encrypted_raw =from_hex(encrypted_msg);
                string decrypted =xor_encrypt_decrypt(encrypted_raw,key );

                // ML ANALYSIS
                string ml_result =call_ml_api(decrypted);
                cout << "\n";

                if (is_private) {
                    cout << "[PRIVATE] ";
                }

                cout << sender<< ": "<< decrypted<< endl;
                cout << "ML → "<< ml_result<< endl;

                //MODERATION 

                bool spam =(ml_result.find("\"spam\":1")!= string::npos);
                bool negative =(ml_result.find("NEGATIVE")!= string::npos);

                if (spam || negative) {

                    reputation[sender] -= 10;

                    cout << "\n⚠️ Dangerous message detected from "<< sender<< endl;

                    cout << "Reputation Score: "<< reputation[sender]<< endl;

                    // REPORT TO SERVER
                    string reason =spam ? "SPAM" : "NEGATIVE";

                    string report_msg ="REPORT|" +username + "|" +sender + "|" +reason;

                    send(sock,report_msg.c_str(),report_msg.size(),0);
                    // WARNING
                    if(reputation[sender]== -10){
                        cout<<sender<< " received warning.\n";
                    }

                    // MUTE
                    else if(reputation[sender]== -20){
                        cout << sender<< " muted.\n";
                        mutedUsers.insert(sender);
                    }

                    // PERMANENT BLOCK
                    else if (reputation[sender]<= -30){
                        cout << sender<< " permanently blocked.\n";
                        mutedUsers.erase(sender);
                        blockedUsers.insert(sender);
                    }
                }
            }
        }
    }

    close(sock);

    return 0;
}