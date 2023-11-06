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

uint64_t convertTimestamp(string line);


struct User{
    public:
        int64_t timestamp;
        string id;
        int pin;
        int balance;
        vector<string> ip;
        
        User() : timestamp(0), id(""), pin(0), balance(0), ip() {
        }

        // Constructor with integer timestamp
        User(int64_t ts, string idi, int pini, int balancei)
            : timestamp(ts), id(idi), pin(pini), balance(balancei), ip() {
        }

        // Constructor with string timestamp
        User(string ts, string idi, int pini, int balancei)
            : timestamp(convertTimestamp(ts)), id(idi), pin(pini), balance(balancei), ip() {
        }
};
unordered_map<string, User> usermap;

struct Transaction{
    public:
        int64_t timestamp;
        string ip;
        User sender;
        User recipient;
        int amount;
        int64_t exec;
        bool o; //true -> o, false -> s

        Transaction(int64_t ts, string ipAddress, User sndr, User rcpt, int amnt, int64_t e, bool isO)
        : timestamp(ts), ip(ipAddress), sender(sndr), recipient(rcpt), amount(amnt), exec(e), o(isO) {
    }
};
struct Comparator{
    bool operator()(Transaction t1, Transaction t2) const{
        if(t1.exec > t2.exec){
            return true;
        }
        return false;
    }
};
priority_queue <Transaction, vector<Transaction>, Comparator> transpq; //track transactions



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
    string cleanedInput;
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
            User temp(timestampPart, id, stoi(pinPart), stoi(balancePart));
            
            usermap[temp.id] = temp;
        }
    }
}

bool validTransaction(Transaction){
    
}

void transactionfill(){
    string line;
    //user ip must be updated
    while(getline(cin, line)){
        if(cin.peek() == '#'){
            while(cin.peek() != '\n'){
                cin >> line;
            }
            continue;
        }
        cin >> line;
        if(line == "login"){
            string id;
            string pin;
            string ip;
            cin >> id >> pin >> ip;
            cout << "login: " << id << endl;
            cout << "pin: " << pin << endl;
            cout << "ip: " << ip << endl;

        }
        else if(line == "place"){
            string timestamp;
            string ip;
            string sender;
            string recipient;
            string amount;
            string exec;
            string os;
            cin >> timestamp >> ip >> sender >> recipient >> amount >> exec >> os;
            cout << "Transaction: " << ip << sender << recipient << endl;
            bool isO = (os=="o");
            Transaction temp(convertTimestamp(timestamp), ip, usermap[sender], usermap[recipient], stoi(amount), convertTimestamp(exec), isO);
            transpq.push(temp);
        }
    }
}

int main(int argc, char **argv){
    ios_base::sync_with_stdio(false);
    cl(argc, argv);
    regfill();
    transactionfill();
};
