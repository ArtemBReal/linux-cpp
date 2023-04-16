#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <future>
#include <thread>
#include <mutex>
#include <chrono>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

#include <sstream>
#include <poll.h>

using namespace std;

constexpr int MAX_CLIENTS = 10;
constexpr int BUFFER_SIZE = 1024;

std::mutex mtx;

void send_messages(int client_socket, vector<int> &clients, int index, string message){

        mtx.lock();
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] != client_socket && clients[i] != -1) {
                if(send(clients[i], message.c_str(), message.size(),0) == -1) {
                    cerr << "Ошибка! Не удалось отправить сообщение клиенту " << i << endl;
                    perror("Ошибка");
                }
                else cout << "\nСообщение от клиента " << index << " отправлено клиенту " << i << endl;
            }
        }
       mtx.unlock();

}

void handle_client(int client_socket, vector<int> &clients, int index, int& soc_count) {
    char buffer [BUFFER_SIZE];
    string part_message;
    string message;
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received  = recv(client_socket, buffer, BUFFER_SIZE, 0);

        if (bytes_received <= 0) {
            ostringstream err;
            cerr << "\nКлиент " << index <<" отключился" << endl;
            //err << "\nКлиент " << index <<" отключился" << endl;
            message = err.str();
            //send_messages(client_socket, clients, index, message);
            close(client_socket);
            clients[index] = -1;
            soc_count--;
            index = -1;
            break;
        }

        part_message.clear();
        part_message = string(buffer, bytes_received);
        message.clear();

        //ostringstream oss;
        //oss << "\nКлиент " << index << " пишет: ";
        //message = oss.str();
        message+=part_message;

        //cout << "\nПолучено сообщение от клиента " << index << endl;
        //cout << endl;

        send_messages(client_socket, clients, index, message);

       cout << endl;
    }
    close(client_socket);
}



int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_RCVBUF, &optval, sizeof(optval));
    if (server_socket == -1) {
        cerr << "Ошибка! Не удалось создать сокет сервера" << endl;;
        return -1;
    }

    struct timeval timeout;
    //timeout.tv_sec = 500;
    //timeout.tv_usec = 0;
    int opt = 1;
    //if (setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1) {
    if (setsockopt(server_socket, SOL_SOCKET, SO_RCVBUF, &optval, sizeof(optval)) == -1){
        cerr << "Ошибка! Не удалось установить настройки сокета" << endl;;
        return -1; 
    }


    //Связываем сокет с адресом и портом
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(8080);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        cerr << "Ошибка! Не удалось связать сокет" << endl;
        perror("Ошибка");
        return -1;
    }

    //Слушаем входящие соединения
    if (listen(server_socket, MAX_CLIENTS) == -1) {
        cerr << "Ошибка! Не удалось прослушать входящие соединения" << endl;
        return -1;
    }

 
    vector<int> clients(MAX_CLIENTS, -1);
    vector<future<void>> futures(MAX_CLIENTS);
    void* ip_address = &server_address.sin_addr;
    char ip_str[INET6_ADDRSTRLEN];
    inet_ntop( server_address.sin_family, ip_address, ip_str, sizeof ip_str);

    cout << "IP-адрес сервера: " << ip_str << endl;

    cout << "Сервер запущен. Ожидание подключения клиентов..." << endl;



    int index = -1;
    int soc_count = 0;
    while (true) {
        int client_socket = accept(server_socket, NULL, NULL);
        cout << "\n client socket from while = " << client_socket << endl;
        //fcntl(client_socket, F_SETFL, O_NONBLOCK);
        if (client_socket == -1) {
            cerr << "Ошибка! Не удалось подтвердить подключение клиента" << endl;
            continue;
        }

        
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] == -1) {
                index = i;
                soc_count++;
                //clients[i] = client_socket;
                break;
            }
            
        }

        if (index == -1) {
            cerr << "Ошибка! Достигнуто максимальное число клиентов" << endl;
            close(client_socket);
            continue;
        }

        string info_message;

        ostringstream inf;
        
        cout << "Подключен клиент " << index << " с сокетом = " << client_socket << endl;
        inf << "Вам присвоен номер " << index << endl;
        info_message = inf.str();
        
        if (send(client_socket, info_message.c_str(), info_message.size(),0) == -1) {
            cerr << "Ошибка! Уведомление не дошло до клиента" << endl;
            continue;
        } 

        //fcntl(client_socket, F_SETFL, O_NONBLOCK);
        clients[index] = client_socket;
        for (int j = 0; j < MAX_CLIENTS; j++){
            cout << "\nclient " << j << " socket " << clients[j];
        }

       
        //------------------------------------------------------------------------------------------------------------
        futures[index] = async(launch::async, handle_client, client_socket, ref(clients), cref(index),ref(soc_count));
        //------------------------------------------------------------------------------------------------------------

        inf.str("");
        info_message.clear();
        inf <<"Подключился клиент " << index << endl;
        info_message = inf.str();
        //send_messages(client_socket, clients, index, info_message);
    
    }

    close(server_socket);

    return 0;
}