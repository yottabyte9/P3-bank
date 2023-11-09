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
uint64_t currentTime;

struct User{
    public:
        uint64_t timestamp;
        string id;
        int pin;
        int balance;
        vector<uint64_t> ip;
        
        User() : timestamp(0), id(""), pin(0), balance(0), ip() {
        }

        // Constructor with integer timestamp
        User(uint64_t ts, string idi, int pini, int balancei)
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
        uint64_t timestamp;
        uint64_t ip;
        User& sender;
        User& recipient;
        int amount;
        uint64_t exec;
        bool o; //true -> o, false -> s
        int fee;

        Transaction(uint64_t ts, uint64_t ipAddress, User& sndr, User& rcpt, int amnt, uint64_t e, bool isO)
        : timestamp(ts), ip(ipAddress), sender(sndr), recipient(rcpt), amount(amnt), exec(e), o(isO), fee() {
        }
        Transaction(uint64_t ts, uint64_t ipAddress, string sndrid, string rcptid, int amnt, uint64_t e, bool isO)
        : timestamp(ts), ip(ipAddress), sender(usermap[sndrid]), recipient(usermap[rcptid]), amount(amnt), exec(e), o(isO), fee() {
        }
};
struct Comparator{
    bool operator()(Transaction* t1, Transaction* t2) const{
        if(t1->exec > t2->exec){
            return true;
        }
        return false;
    }
};
priority_queue <Transaction*, vector<Transaction*>, Comparator> transpq; //track transactions



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

uint64_t convertTimestamp(string line){ //converts ips as well
    string cleanedInput;
    for (char c : line) {
        if (c != ':' && c != '.') {
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

bool validLogin(string id, int pin){ //make IP into number
    auto it = usermap.find(id);
    if(it==usermap.end()){
        return false; //no user there
    }
    else if(usermap[id].pin != pin){
        return false; //wrong pin
    } //already logged in???
    return true;
}

bool validTransaction(uint64_t ts, uint64_t ipAddress, string sndrid, string rcptid, string amt, uint64_t e){
    //cout << e << " checking validity \n";
    if(ts + 3000000 < e){
        if(v) cout << "Select a time less than three days in the future.\n";
        return false;
    }
    if(usermap.find(sndrid) == usermap.end()){ //sender does not exist
        if(v) cout << "Sender " << sndrid << " does not exist.\n";
        return false;
    }
    if(usermap.find(rcptid) == usermap.end()){ //recipient does not exist
        if(v) cout << "Recipient " << rcptid << " does not exist.\n";
        return false;
    }
    User& sndr = usermap[sndrid];
    User& rcpt = usermap[rcptid];
    if(usermap[sndr.id].timestamp > ts || usermap[rcpt.id].timestamp > ts){
        if(v) cout << "At the time of execution, sender and/or recipient have not registered.\n";
        return false;
    }
    if(sndr.ip.size() == 0){
        if(v) cout << "Sender " << sndrid << " is not logged in.\n";
        return false;
    }
    auto it = find(sndr.ip.begin(), sndr.ip.end(), ipAddress);
    if(it == sndr.ip.end()){
        if(v) cout << "Fraudulent transaction detected, aborting request.\n";
        return false;
    }
    if(v) cout << "Transaction placed at " << ts << ": $" << amt <<" from " << sndrid << " to " << rcptid << " at " << e << ".\n";
    return true;
}

bool checkamt(Transaction* place){
    int fee = (int)(place->amount *0.01);
    if(fee < 10){
        fee = 10;
    }
    if(fee > 450){
        fee = 450;
    }
    if(place->sender.timestamp > place->timestamp + 50000000000){
        fee = (fee*3)/4;
    }
    //place.amount += (int)fee;
    if(place->o){ //sender cover
        if(place->sender.balance < place->amount + fee){
            if(v){
                cout << "Insufficient funds to process transaction " << place->ip << ".\n"; //is this IP or ID?
            }
            return false;
        } 
    }
    else if(!place->o){
        if(place->sender.balance < place->amount) return false;
        if(place->recipient.balance + place->amount < fee) return false;
    }
    place->fee = fee;
    //cout << place->sender.balance << "\n";
    return true;
} 

vector<Transaction*> transdone;

void transactionfill(){
    //cout << "begin transaction fill \n";
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
            //cout << "started login\n";
            string id;
            string pin;
            string ip;
            cin >> id >> pin >> ip;
            if(validLogin(id, stoi(pin))){
                usermap[id].ip.push_back(convertTimestamp(ip));
                if(v){
                    cout << "User " << id << " logged in.\n";
                }
            }
            else{
                if(v){
                    cout << "Failed to log in " << id << ".\n";
                }
            }
        }
        else if(line == "out"){
            string id;
            string ip;
            cin >> id >> ip;
            auto userIt = usermap.find(id);
            if (userIt != usermap.end()) {
                User& user = userIt->second;
                // Use std::remove_if to remove the specified ip
                user.ip.erase(std::remove_if(user.ip.begin(), user.ip.end(),
                    [ip](const uint64_t& userIp) { return userIp == convertTimestamp(ip); }),
                    user.ip.end());

                cout << "User " << id << " logged out.\n";
            }
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
            //cout << "Transaction: " << ip << sender << recipient << "  ";
            bool isO = (os=="o");
            uint64_t convertedIP = convertTimestamp(ip);
            while(transpq.size() > 0 && transpq.top()->exec < convertTimestamp(timestamp)){
                Transaction* temp = transpq.top();
                if(checkamt(temp)){
                    //cout << temp->sender.id << ", " << temp->recipient.id << ", " << temp->amount << endl;
                    if(temp->o){ //seller covers fee
                        temp->sender.balance -= temp->fee;
                        temp->sender.balance -= temp->amount;
                        temp->recipient.balance += temp->amount;
                    }
                    else{
                        temp->sender.balance -= (int)(temp->fee/2);
                        temp->sender.balance -= temp->amount;
                        temp->recipient.balance += temp->amount - (int)(temp->fee/2);
                    }
                    transdone.push_back(temp);
                    if(v){
                        cout << "Transaction executed at " << temp->exec << ": $" << temp->amount << " from " << temp->sender.id << " to " << temp->recipient.id << ".\n";
                    }
                }
                transpq.pop();
            }
            if(validTransaction(convertTimestamp(timestamp), convertedIP, sender, recipient, amount, convertTimestamp(exec))){
                Transaction *temp = new Transaction(convertTimestamp(timestamp), convertedIP, sender, recipient, stoi(amount), convertTimestamp(exec), isO);
                transpq.push(temp);
            }
            //cout << "Transaction done \n";
        }
        else if(line == "$$$"){
            break;
        }
    }
}

void place(){
    //cout << transpq.size();
    while(!transpq.empty()){
        Transaction* temp = transpq.top();
        if(checkamt(temp)){
            //cout << temp->sender.id << ", " << temp->recipient.id << ", " << temp->amount << endl;
            if(temp->o){ //seller covers fee
                temp->sender.balance -= temp->fee;
                temp->sender.balance -= temp->amount;
                temp->recipient.balance += temp->amount;
            }
            else{
                temp->sender.balance -= (int)(temp->fee/2);
                temp->sender.balance -= temp->amount;
                temp->recipient.balance += temp->amount - (int)(temp->fee/2);
            }
            transdone.push_back(temp);
            if(v){
                cout << "Transaction executed at " << temp->exec << ": $" << temp->amount << " from " << temp->sender.id << " to " << temp->recipient.id << ".\n";
            }
        }
        else{
            delete temp;
        }
        transpq.pop();
    }
}

void querylist(){
    string indicator;
    string in1;
    string in2;
    cin >> indicator;
    if(indicator == "l"){
        cin >> in1 >> in2;
        uint64_t ts1 = convertTimestamp(in1);
        uint64_t ts2 = convertTimestamp(in2);
        for(int i=0; i<transdone.size(); i++){
            if(transdone[i]->exec < ts2 && transdone[i]->exec >= ts1){
                cout << i << "";
            }
        }
    }
    else if(indicator == "r"){
        cin >> in1 >> in2;
        uint64_t ts1 = convertTimestamp(in1);
        uint64_t ts2 = convertTimestamp(in2);
    }
    else if(indicator == "h"){
        cin >> in1;
        string id = in1;
    }
    else if(indicator == "s"){
        cin >> in1;
        uint64_t tsDay = convertTimestamp(in1);
    }
}

void removeall(){
    for(auto i: transdone){
        delete i;
    }
    if(transpq.size() != 0){
        cout << "FAILED TO DELETE TRANSACTION PQ OBJECTS";
    }
}

int main(int argc, char **argv){
    ios_base::sync_with_stdio(false);
    cl(argc, argv);
    regfill();
    transactionfill();
    place();
    querylist();
    removeall();
};
