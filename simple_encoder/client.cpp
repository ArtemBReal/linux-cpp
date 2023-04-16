#include <iostream>
#include <stdio.h>
#include <fstream>
#include <dirent.h>
#include <string.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

using namespace std;

int main (int argc, char *argv[])
{

    string data;
	string filename = argv[1];
    ifstream infile(filename);

    if (!infile.is_open()){
        cerr<< "Ошибка открытия файла "<< filename << "!" << endl;
        return 1;
    } else {
        while (getline (infile, data)){
            cout << data << "\n";
        }
        infile.close();
    }
	return 0;
}//main


