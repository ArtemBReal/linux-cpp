#include <iostream>
#include <string>
#include <cstdlib>
#include <stdio.h>
#include <fstream>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <algorithm>

using namespace std;

const int NumberOfFiles = 2;

int main()
{
    int fd[2];  //дескриптор файла (0 - чтение канала, 1 - запись в канал)

    pid_t cpid;
    char* files_array[NumberOfFiles];
    char buffer[NumberOfFiles][1024];
    char xor_res[1024];

    int length[NumberOfFiles];

    string filename;

    for (int i = 0; i < NumberOfFiles; i++){

        cout << "Введите имя файла, открываемого дочерним процессом: ";
        getline(cin, filename);
        cin.clear();
        cout.clear();
        files_array[i] = const_cast<char*>(filename.c_str());
        // Создание канала pipe()
        if (pipe(fd) == -1){
            cerr << "Ошибка в создании канала!" << endl;
            return 1;
        }

        // Создание дочернего процесса
        cpid = fork();

        if (cpid == -1){
            cerr << "Ошибка в создании дочерднего процесса!" << endl;
            return 2;
        }

        if (cpid == 0) {    // Дочерний процесс
            close(fd[0]);   //Закрытие потока чтения канала

            //Перенаправление потока вывода на канал
            dup2(fd[1], STDOUT_FILENO);

            //Запуск внешней программы
            execlp("./client", "./client", files_array[i], NULL);

            exit(0);

        } else {    //Родительский процесс
            close(fd[1]);

            //Ожидание завершения дочернего процесса
            int status;
            waitpid(cpid, &status, 0);

            //Проверка, был ли дочерний процесс завершён успешно
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                cout << "\nДочерний процесс завершён успешно" << endl;
            } else {
                cerr << "Ошибка выполнения дочернего процесса" << endl;
                return 1;
            }

            //Чтение данных из канала
            read(fd[0], buffer[i], sizeof(buffer[i]));
            close(fd[0]);   //закрытие потока чтения канала

            length[i] = strlen(buffer[i]);
            cout << "Полученные из дочернего процесса данные: \n" << buffer[i] << endl;
            
        }
    }
    
    //Поиск максимального количества действительных символов, считанных дочерней программой из файлов
    int* maxLength = max_element(length, length + (sizeof(length) / sizeof(length[0])));

    //Открытие файла для записи результата
    ofstream fout ("outputfile.txt");

    if (fout.is_open()){    //Если файл открыт удачно
        for (int j = 0; j < *maxLength; j++) {
            xor_res[j] = buffer[0][j] xor buffer[1][j]; //Побитовая операция xor
            fout << xor_res[j];
        }//for j
        fout << endl;
        cout << "\nДанные успешно записаны в файл" << endl;
        fout.close();
    } else {    //Если не удалось открыть файл
        cout << "Ошибка! Не удалось открыть файл для записи!" << endl;
    }
    
    cout.clear();
    cin.clear();

    return 0;
}
