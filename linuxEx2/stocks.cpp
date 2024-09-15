#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>
#include <dirent.h>
#include <filesystem>
#include <stdio.h>
#include <string.h>

#include <zip.h>
#include <json-c/json.h>

#include "stockData.h"
#include "csvExport.h"
#include "utils.h"

#include <fcntl.h>
#include <errno.h>
#include <sys/syscall.h>
#include <signal.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <iostream>
#include "stockData.h"
#include <zip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <dirent.h>
#include <limits.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <zip.h>
#include <vector>
#include <set>

using namespace std;

static void display_menu();

string option_4();

int get_input();

void plumb_pipes_and_remove_files();

void sigint_handler(int signum);

/***************globals***************/
int slave_pid = 0;
int parent_pid = 0;

bool option1_done_flag = false;

int pipe_parent_to_child[2]; // Used to store two ends of first pipe
int pipe_child_to_parent[2]; // Used to store two ends of second pipe

#define READ 0
#define WRITE 1
#define SIZE_OF_DATA 2000
/*************************************/

class zipUtil   //used for zip extraction later on
{
public:
    const char *name = "stocks_db.zip";
    set<string> extract_zip();
};


int main(void)
{
    system("chmod 755 get_stocks_data.sh"); // first thing, enabling the option to execute the script

    zipUtil ext;
    ext.extract_zip();   //extracting previous db

    int bufSize = 8000;
    char slave_buffer[bufSize];

    char option_choice[100];
    string stock_for_option_1_or_3;

    pid_t pid;

    //checking pipe creation errors
    if (pipe(pipe_parent_to_child) == -1)
    {
        fprintf(stderr, "Pipe Failed");
        return 1;
    }
    if (pipe(pipe_child_to_parent) == -1)
    {
        fprintf(stderr, "Pipe Failed");
        return 1;
    }

    pid = fork();
    while (true)
    {

        if (pid < 0) // should not fail
        {
            fprintf(stderr, "fork Failed");
            return 1;
        }

        //PARENT PROCESS
        else if (pid > 0) //only writes to pipe_parent_to_child and reads from pipe_child_to_parent
        {
            parent_pid = getpid();  //updating the global variable

            cout << endl
                 << endl
                 << "================================" << endl;

            signal(SIGINT, sigint_handler); // for catching ^C

            //closing unused pipe ends
            close(pipe_parent_to_child[READ]);
            close(pipe_child_to_parent[WRITE]);

            //menu 1-4 display
            display_menu();

            //recieving input from the user
            int choice = get_input();

            string temp = to_string(choice);    //later on, we will concatenate the chosen option to the input stock name, in order to write together to the pipe.
                                                // for example: 1GOOG
            switch (choice) //specifically, in option 1 and 3 we need to get the stock name
            {
            case 1:
                cout << "Type stock name: ";
                cin >> stock_for_option_1_or_3;
                temp += stock_for_option_1_or_3;
                break;
            case 3:
                cout << "Type stock name: ";
                cin >> stock_for_option_1_or_3;
                temp += stock_for_option_1_or_3;
                break;
            }

            char input_str[100];
            strcpy(input_str, &temp[0]);
            char stringFromWorker[bufSize];

            write(pipe_parent_to_child[WRITE], input_str, strlen(input_str) + 1);   // writing to child

            read(pipe_child_to_parent[READ], slave_buffer, bufSize);    // read from child

            cout << slave_buffer; // display buffer;
        }
        //CHILD PROCESS
        if (pid == 0)   //only writes to pipe_child_to_parent and reads from pipe_parent_to_child
        {
            slave_pid = getpid();

            //closing unused pipe ends
            close(pipe_child_to_parent[READ]);
            close(pipe_parent_to_child[WRITE]);
            
            //reading from parent process 
            read(pipe_parent_to_child[READ], slave_buffer, bufSize);

            //seperating the stock name and the user choice
            string temp(slave_buffer);
            string to_pipe_buffer = "";

            int user_choice = temp[0] - '0';

            temp = temp.substr(1, temp.length());

            //getting the output of each function, and doing the operations
            switch (user_choice)
            {
            case 1:
                to_pipe_buffer += option_1or3(1, option1_done_flag, temp);
                option1_done_flag = true;
                break;
            case 2:
                to_pipe_buffer += option_2();
                break;
            case 3:
                to_pipe_buffer += option_1or3(3, option1_done_flag, temp);
                option1_done_flag = true;
                break;
            case 4:
                to_pipe_buffer += option_4();
                plumb_pipes_and_remove_files();
                exit(1);
                break;
            }

            //writing back to pipe
            write(pipe_child_to_parent[WRITE], to_pipe_buffer.c_str(), bufSize);
        }
    }
}
string option_4()
{
    vector<string> names = fetch_stock_names_in_dir();

    filesEndingWithStockToCSV(names); // all data- into csv format
    string buffer = "";

    remove("stocks_db.zip");
    int errorp;
    
    zip_t *zipper;
    zip_source_t *zip_source;

    ifstream fs;          // used to open files, READ them into buffer, into new files inside the zip
    string name_with_csv; // used to add ".csv"

    for (int i = 0; i < names.size(); i++) // each csv file, is being opened, READ, and putted inside the zip
    {
        zipper = zip_open("stocks_db.zip", ZIP_CREATE, &errorp);

        name_with_csv = names[i] + ".csv";

        fs.open(name_with_csv, std::ios::binary);

        std::string content((std::istreambuf_iterator<char>(fs)), (std::istreambuf_iterator<char>()));
        fs.close();

        zip_source = zip_source_buffer(zipper, content.c_str(), content.length(), 0);
        zip_file_add(zipper, name_with_csv.c_str(), zip_source, ZIP_FL_OVERWRITE);
        zip_close(zipper);

        remove(name_with_csv.c_str());
    }
    string tmp1;

    char tmp[2048];
    getcwd(tmp, 2048);
    tmp1 = tmp;
    buffer += "Data saved in " + tmp1 + "/stocks_db.zip\n";
    return buffer;
}

static void display_menu()
{
    cout << "1 - Fetch stock data      " << endl
         << "2 - List fetched sotcks   " << endl
         << "3 - Print stock data      " << endl
         << "4 - Save all stocks data  " << endl
         << "Make your choice(1/2/3/4): ";
}

void sigint_handler(int signum) // catch CTRL C
{
    if (getpid() == slave_pid || getpid() == parent_pid) /// child process will do option 4 and exit
    {
        cout << endl
             << "Crushing gently :-)..." << endl;
        option_4();
    }

    plumb_pipes_and_remove_files();
    exit(1);
}

void plumb_pipes_and_remove_files() /// close all the pipe
{
    close(pipe_parent_to_child[READ]);
    close(pipe_parent_to_child[WRITE]);

    close(pipe_child_to_parent[READ]);
    close(pipe_child_to_parent[WRITE]);

    system("rm -f *.stock");
    system("rm -f *.eps");
    system("rm -f *.csv");
}

int get_input()
{
    int input = 0, preInput = 0;
    bool printed = false;
    cin >> input;
    if (preInput != input)
        printed = false;
    if (input < 1 || input > 4)
    {
        if (!printed)
        {
            cout << "Must be options 1-4 " << endl;
            printed = true;
        }
    }
    return input;
}

set<string> zipUtil::extract_zip()
{
    set<string> stockNameSet;
    int err = 0;
    zip *zip = zip_open(name, 0, &err);
    struct zip_stat st;

    for (int i = 0; i < zip_get_num_entries(zip, 0); i++)
    {
        zip_stat_index(zip, i, 0, &st);
        char *inside = new char[st.size];
        zip_file *file = zip_fopen(zip, st.name, 0);
        zip_fread(file, inside, st.size);
        zip_fclose(file);

        ofstream outfile(st.name);
        outfile << inside;
        outfile.flush();
        outfile.close();

        delete[] inside;
        string s(st.name);
        stockNameSet.insert(s.substr(0, s.find(".")));
    }
    zip_close(zip);
    return stockNameSet;
}