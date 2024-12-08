#include <iostream>
#include <cstring>
#include <string>
#include <sstream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <fstream>
#include "termcolor.hpp"

using namespace std;
using namespace termcolor;
#define AUTH "authentication.txt"
#define FILENAME "log.txt"

typedef struct Post{
    string topic;
    string body;
    string author;
}POST;

typedef struct User{
    string userName;
    unsigned long password;
}USER;

unsigned long PasswordHashing(string password){
    unsigned long hashValue = 5381;
    for(char c : password){
        hashValue = ((hashValue << 5)+hashValue) +c;
    }
    return hashValue;
}

bool SaveNewUser(vector<USER> usersList){
    ofstream fp;
    fp.open(AUTH);
    if(!fp.is_open()){
        cout << "Can't open file"<<endl;
        return false;
    }
    for(int i=0; i<usersList.size();i++){
        fp<<usersList[i].userName<<endl;
        fp<<usersList[i].password<<endl;
    }
    return true;
}

vector<USER> LoadUsers(){
    vector<USER> usersList;
    ifstream fp;
    fp.open(AUTH);
    
    if(!fp.is_open()){
        cout << "Can't open file"<<endl;
        return usersList;
    }
    string userName;
    string password;

    while(getline(fp, userName)){
        USER user;
        getline(fp,password);
        user.userName = userName;
        user.password = stoul(password);
        userName.clear();
        password.clear();
        usersList.push_back(user);
    }
    return usersList;
}

USER HandleLogin(string login){
    USER user;
    string password;
    istringstream iss(login);
    getline(iss, user.userName, '\n');
    getline(iss, password, '\n');
    user.password = PasswordHashing(password);
    return user;
}

bool Authenticate(USER user, vector<USER> usersList){
    for(int i=0; i<usersList.size();i++){
        if(user.password == usersList[i].password && user.userName == usersList[i].userName){
            return true;
        }
    }
    return false;
}

bool SaveToFile(vector<POST> posts){
    ofstream fp;
    fp.open(FILENAME);
    if(!fp.is_open()){
        cout << "Can't open file"<<endl;
        return false;
    }
    for(int i=0;i<posts.size();i++){
        fp << posts[i].topic<< endl<< posts[i].body<< endl << posts[i].author<<endl;
    }
    fp<<"/e";
    return true;
}

vector<POST> LoadFromFile(){
    ifstream fp;
    vector<POST> posts;
    fp.open(FILENAME);
    
    if(!fp.is_open()){
        cout << "Can't open file"<<endl;
        return posts;
    }
    string topic;
    string body;
    string author;

    while(true){
        getline(fp, topic);
        if(topic == "/e"){
            break;
        }
        getline(fp,body);
        getline(fp,author);
        POST post;
        post.topic = topic;
        post.body = body;
        post.author = author;
        posts.push_back(post);
    }
    return posts;
}

POST HandleClientMessage(string message){
    POST newPost;
    istringstream iss(message);
    string author;
    string topic;
    string body;
    getline(iss,newPost.author, '\n');
    getline(iss,newPost.topic, '\n');
    getline(iss,newPost.body, '\n');
    return newPost;
}

int main()
{
    vector<POST>postsList =LoadFromFile();
    vector<USER> usersList = LoadUsers();

    //Set up Socket for Server
	int ServerSocket;
    ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ServerSocket == -1){
        cout <<on_red<< "ERROR: Failed to create ServerSocket"<<reset<<endl;
        return 0;
    }
    cout << on_yellow<<"Server socket created"<<reset<<endl;

    //Set up Server information
    sockaddr_in SvrAddr;
    SvrAddr.sin_family = AF_INET; 
    SvrAddr.sin_addr.s_addr = INADDR_ANY; //Accept connection from any host
    SvrAddr.sin_port = htons(27000); //Port used

    //Bind socket to address
    if(bind(ServerSocket, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr)) == -1){
        close(ServerSocket);
        cout<< on_red << "ERROR: Failed to bind ServerSocket"<<reset << endl;
        return 0;
    }
    cout << on_yellow<<"Server binded successfully"<<reset<<endl;

    //Listen for connection
    if(listen(ServerSocket,1) == -1){
        close(ServerSocket);
        cout << on_red << "ERROR: Listen failed to configure ServerSocket"<<reset<<endl;
    }

        if(postsList.size() == 0){
        cout << on_yellow << "Empty, make some post!"<<reset<<endl;
    }
    else{
            cout << on_blue << "Posts history"<<reset<<endl;
    for(int i=0; i<postsList.size();i++){
        cout <<green<<"Post: #"<<i+1<<reset<<endl;
        cout<<"Author:" << postsList[i].author<<endl;
        cout<<"Topic:" << postsList[i].topic<<endl;
        cout<<"Message:" << postsList[i].body<<endl<<endl;
    }
    cout<<cyan << "Total: "<< postsList.size() << " posts"<<reset<<endl;
    }

    cout << on_yellow<< "Waiting for connection..." <<reset<<endl;
    //Accept connection (3-step handshake)
    int ConnectionSocket;
    bool newConnect = true;

    while(newConnect){
    int loginRequest = -1;
    if((ConnectionSocket = accept(ServerSocket, NULL, NULL)) == -1){
        close(ServerSocket);
        return 0;
    }
    cout << on_green <<"Client connected"<<reset<<endl;
    while(true){
    //     string userEmpty;
    // if(usersList.empty()){
    //     userEmpty = "1";
    // }
    // else{
    //     userEmpty = "0";
    // }
    // send(ConnectionSocket, userEmpty.c_str(), userEmpty.size(), 0);
        char mode[5];
    int clientMode = recv(ConnectionSocket, mode, sizeof(mode),0);
    if(clientMode <0){
        cout << on_red<<"ERROR: Can't receive login request from client, please try to reconnect"<<reset<<endl;
        close(ConnectionSocket);
    }
    else if(clientMode == 0){
        cout << on_yellow << "Client disconnected, waiting for new connection..."<<reset<<endl;
        close(ConnectionSocket);
    }
    else{
        if(mode[0] == 'c'){
            char newUser[1024];
            int createUser = recv(ConnectionSocket, newUser, sizeof(newUser), 0);
            newUser[createUser] = '\0';
            if(createUser <=0){
                cout <<on_red<<"ERROR: Failed to create user"<<reset<<endl;
            }
            else{
                USER user  = HandleLogin(newUser);
                usersList.push_back(user);
                cout<<on_cyan << "New user saved"<<reset<<endl;
            }
            mode[0]='\0';
            clientMode = recv(ConnectionSocket, mode, sizeof(mode),0);
            cout << on_cyan <<"Waiting for login..."<<reset<<endl;
        }
        if(mode[0] == 'l'){
            cout << on_yellow<<"Waiting for authentication..."<<reset<<endl;
            char login[1024];
            loginRequest = recv(ConnectionSocket, login, sizeof(login), 0);
            login[loginRequest]='\0';
            USER user;
            if(loginRequest <=0){
                cout << on_red<<"Login failed, disconnected with client, waiting for new connection..."<<reset<<endl;
                close(ConnectionSocket);
                loginRequest = -1;
            }
            else{
                user = HandleLogin(login);
                bool auth = Authenticate(user, usersList);
                cout << auth<<endl;
                if(auth){
                    cout << on_green<<"Authenticate successfully!, waiting message from client..."<<reset<<endl;
                    string loginMessage = "Authenticate successfully!";
                    send(ConnectionSocket, loginMessage.c_str(), loginMessage.size(), 0);
                    break;
                }
                else{
                    string loginMessage = "Username or password is incorrect";
                    send(ConnectionSocket, loginMessage.c_str(), loginMessage.size(), 0);
                    cout << on_red<<"Failed to authenticate user."<<reset<<endl;
                    loginRequest = -1;
                    mode[0]='\0';
                }
            }
        }
    }
    }

    while(loginRequest>0){
        char buffer[1024];
        // cout<<on_cyan << "Waiting response from client..."<<reset<<endl;
        int sizeOfData = recv(ConnectionSocket, buffer, sizeof(buffer), 0); //Get how many byte received from client
        if (sizeOfData == -1) {
            cout<<on_red << "ERROR: Failed to receive data" << reset<<endl;
            close(ConnectionSocket);
            break;
        } 
        else if(sizeOfData==0){
            close(ConnectionSocket);
            cout <<on_yellow<< "Client disconnected, waiting for new connection..."<<reset<<endl;
            break;
        }
        else {
            POST newPost;
            buffer[sizeOfData] = '\0'; //Put a termination character at the end of message 
            newPost = HandleClientMessage(buffer);
            postsList.push_back(newPost);
            cout << on_magenta << "New post received from client"<<reset<<endl;
            cout << "Author: "<<newPost.author<<endl<<"Header: "<<newPost.topic<<endl<<"Body: "<<newPost.body<<endl;
            string message;
        string input;
        cout << "Enter message (enter for stopping server): ";
        getline(cin,input);
        if(input.empty()){
            message = "\0";
            send(ConnectionSocket, message.c_str(), message.size(), 0);
            close(ConnectionSocket);
            cout<<on_yellow << "Disconnected, server stopped"<<reset<<endl;
            newConnect= false;
            break;
        }
        else{
            message = input;
            send(ConnectionSocket, message.c_str(), message.size(), 0); //Send message to client
        }
        }
    }
}
SaveNewUser(usersList);
SaveToFile(postsList);
    //Close socket (4-step handshake)
    close(ConnectionSocket);
    close(ServerSocket);

    return 1;
    
}
