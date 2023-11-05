// Project Identifier: 292F24D17A4455C1B5133EDD8C7CEAA0C9570A98
#include <iostream>
#include <string>
#include <math.h>
#include <algorithm>
#include <queue>
#include <getopt.h>
#include <fstream>
#include <sstream>
#include <map>

using namespace std;

/* priority_queue <Transaction*, vector<Transaction*>, Comparator> gq; //track transactions

struct Transaction{
    public:
        int32_t timestamp;
        string ip;
        User sender;
        User recipient;
        int amount;
        int32_t exec;
        bool o; //true -> o, false -> s
}; */

struct User{
    public:
        int64_t timestamp;
        string id;
        int pin;
        int balance;
        vector<string> ip;
};

unordered_map<string, User> usermap;


/* struct Comparator{
    bool operator()(const Transaction* t1, const Transaction* t2) const{
        if(t1->timestamp > t2->timestamp){
            return t2;
        }
        return t1;
    }
}; */

bool f, v = false;
string filename;


void cl(int argc, char** argv){
    int gotopt;
    int option_index = 0;
    option long_opts[]={
        {"verbose", no_argument, nullptr, 'v'},
        {"file", required_argument, nullptr, 'f'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, 0, nullptr, '\0'}
    };

    while((gotopt = getopt_long(argc, argv, "vf:h", long_opts, &option_index)) != -1){
        switch(gotopt){
            case 'h':
                cout << "help";
                exit(0);
            case 'f':
                f = true;
                filename = optarg;
                break;
            case 'v':
                v = true;
                break;
            default:
                cout << "default error\n";
                exit(1);
        }
    }
}

uint64_t convertTimestamp(string line){
    std::string cleanedInput;
    for (char c : line) {
        if (c != ':') {
            cleanedInput += c;
        }
    }
    return static_cast<uint64_t>(std::stoul(cleanedInput));
}


void regfill(){
    ifstream infile(filename);
    string line;
    while(getline(infile, line)){
        istringstream iss(line); // Use istringstream to split the line
        string timestampPart, id, pinPart, balancePart;
        
        // Split the line based on '|'
        if (getline(iss, timestampPart, '|') &&
            getline(iss, id, '|') &&
            getline(iss, pinPart, '|') &&
            getline(iss, balancePart, '|')) {
            User temp;
            temp.timestamp = convertTimestamp(timestampPart); // Convert the timestamp part
            temp.id = id;
            temp.pin = stoi(pinPart);
            temp.balance = stoi(balancePart);
            usermap[temp.id] = temp;
        }
    }
}

void transactionfill(){
}

int main(int argc, char **argv){
    ios_base::sync_with_stdio(false);
    cl(argc, argv);
    regfill();
    transactionfill();
};