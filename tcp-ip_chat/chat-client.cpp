#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <future>
#include <thread>
#include <mutex>
#include <chrono>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

const int BUFFER_SIZE = 1024;
std::mutex mtx;

void receive_messages(int sockfd) {
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0);
        if (bytes_received == -1) {
            cerr << "\nОшибка! Не удалось получить сообщение с сервера" << endl;
        } else if (bytes_received == 0) {
            cerr << "\nСервер ограничил подключение" << endl;
            close(sockfd);
            break;
        } else {
            //buffer[bytes_received] = '\0';
            cout << "\33[2K\r";
            
            string message(buffer);
            //message += "Введите сообщение: ";
            //message += " Введите сообщение: "; 
        //cout << message;
        printf("%s", message.c_str());
        //printf("\nВведите сообщение: ");
            //cout << "Введите сообщение: ";
        }
    }
}


int main(int argc, char *argv[]) {
    
    if (argc != 4) {
        cerr << "\nОшибка! Вызов клиента должен быть произведён следующим образом: " << argv[0] << "[IP-адрес сервера] [Порт сервера] [Никнейм]" << endl;
        exit(EXIT_FAILURE);
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &optval, sizeof(optval));
    //fcntl(sockfd, F_SETFL, O_NONBLOCK);
    if (sockfd == -1) {
        cerr << "\nОшибка! Не удалось создать сокет" << endl;
        exit(EXIT_FAILURE);
    }

    //int flags = fcntl(sockfd, F_GETFL,0);
    //fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    sockaddr_in server_address;
    memset((struct sockaddr *)&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(argv[1]);
    server_address.sin_port = htons(atoi(argv[2]));

     if (connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        cerr << "\nОшибка соединения с сервером" << endl;
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    cout << "Успешное подключение к серверу" << endl;


    future<void> receive_future = async(launch::async, receive_messages, sockfd);

    string message;
    string part_message;
    string nickname(argv[3]);
    mtx.lock();
    while (true){
        cin.clear();
        message.clear();
        cout << "\nВведите сообщение: ";
        message += nickname;
        message += ": ";
        getline(cin, part_message);
        message += part_message;
        message += "\n";
        //cout << message;
        int bytes_sent = send(sockfd, message.c_str(), message.length(), 0);
       
        if (bytes_sent == -1) {
            cerr << "Ошибка! не удалось отправить сообщение на сервер" << endl;
            break;
        }
        
    }
    mtx.unlock();
    receive_future.wait();
    close(sockfd);

    return 0;
}