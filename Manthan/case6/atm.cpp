#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <limits>
using namespace std;

#define ATM_MAX_ACCOUNTS 2000
#define ATM_MAX_TRANSACTIONS 20000
#define ATM_MAX_CASH 10
#define ATM_MAX_RATES 50
#define ATM_PIN_ATTEMPT_LIMIT 3
#define ATM_MAX_WITHDRAW_PER_TXN 20000
#define ATM_MAX_DEPOSIT_PER_TXN 200000
#define ATM_DAILY_LIMIT 50000
#define ATM_INTL_FEE 0.02

struct ATMAccount {
    string number, name, type, currency, cardCountry;
    int pin=0, locked=0, wrongPin=0;
    double balance=0, dayWithdraw=0, limit=0;
};

struct ATMTransaction {
    int id=0;
    string acc, type, currency, atmID, time, remark;
    double amt=0, balAfter=0;
};

struct ATMCash {
    string atmID, location;
    int n2000=0, n500=0, n200=0, n100=0, n50=0;
};

struct ATMRate {
    string code;
    double toINR=1.0;
};

ATMAccount ACC[ATM_MAX_ACCOUNTS];
ATMTransaction TX[ATM_MAX_TRANSACTIONS];
ATMCash CASH[ATM_MAX_CASH];
ATMRate RATE[ATM_MAX_RATES];

int accCnt = 0, txCnt = 0, cashCnt = 0, rateCnt = 0;
bool dataLoaded = false;

int findAcc(const string &a){
    for(int i=0;i<accCnt;i++) if(ACC[i].number==a) return i;
    return -1;
}
double getRate(const string &c){
    for(int i=0;i<rateCnt;i++) if(RATE[i].code==c) return RATE[i].toINR;
    return 1.0;
}
double toINR(double x,const string &c){ return x * getRate(c); }
double fromINR(double x,const string &c){ double r = getRate(c); return (r==0? x : x / r); }
double feeINR(double x){ return x * ATM_INTL_FEE; }

// Safe stoi/stod that return false on invalid input
bool safe_stoi(const string &s, int &out){
    if(s.empty()) return false;
    try { size_t pos; out = stoi(s,&pos); return pos==s.size(); }
    catch(...) { return false; }
}
bool safe_stod(const string &s, double &out){
    if(s.empty()) return false;
    try { size_t pos; out = stod(s,&pos); return pos==s.size(); }
    catch(...) { return false; }
}

// trim helper (remove leading/trailing spaces)
static inline string trim(const string &s){
    size_t i=0,j=s.size();
    while(i<j && isspace((unsigned char)s[i])) i++;
    while(j>i && isspace((unsigned char)s[j-1])) j--;
    return s.substr(i,j-i);
}

void loadAccounts(){
    ifstream f("atm_accounts.csv");
    if(!f){ cout<<"Warning: atm_accounts.csv not found -> continuing with empty accounts.\n"; return; }
    string line;
    accCnt = 0;
    while(getline(f,line)){
        if(line.empty()) continue;
        stringstream ss(line);
        ATMAccount a;
        string tmp;

        // read all fields defensively
        getline(ss,tmp,','); a.number = trim(tmp);
        getline(ss,tmp,','); a.name = trim(tmp);
        getline(ss,tmp,','); if(!safe_stoi(trim(tmp), a.pin)) { continue; }
        getline(ss,tmp,','); a.type = trim(tmp);
        getline(ss,tmp,','); a.currency = trim(tmp);
        getline(ss,tmp,','); if(!safe_stod(trim(tmp), a.balance)) { continue; }
        getline(ss,tmp,','); safe_stoi(trim(tmp), a.locked); // optional
        getline(ss,tmp,','); safe_stoi(trim(tmp), a.wrongPin);
        getline(ss,tmp,','); safe_stod(trim(tmp), a.dayWithdraw);
        getline(ss,tmp,','); safe_stod(trim(tmp), a.limit);
        getline(ss,tmp,','); a.cardCountry = trim(tmp);

        if(a.number.empty()) continue;
        ACC[accCnt++] = a;
        if(accCnt >= ATM_MAX_ACCOUNTS) break;
    }
    f.close();
}

void loadCash(){
    ifstream f("atm_cash.csv");
    if(!f){ cout<<"Warning: atm_cash.csv not found -> continuing with empty cash.\n"; return; }
    string line;
    cashCnt = 0;
    while(getline(f,line)){
        if(line.empty()) continue;
        stringstream ss(line);
        ATMCash c; string tmp;
        getline(ss,tmp,','); c.atmID = trim(tmp);
        getline(ss,tmp,','); c.location = trim(tmp);
        if(!getline(ss,tmp,',')) continue; if(!safe_stoi(trim(tmp), c.n2000)) c.n2000 = 0;
        if(!getline(ss,tmp,',')) continue; if(!safe_stoi(trim(tmp), c.n500)) c.n500 = 0;
        if(!getline(ss,tmp,',')) continue; if(!safe_stoi(trim(tmp), c.n200)) c.n200 = 0;
        if(!getline(ss,tmp,',')) continue; if(!safe_stoi(trim(tmp), c.n100)) c.n100 = 0;
        if(!getline(ss,tmp,',')) continue; if(!safe_stoi(trim(tmp), c.n50)) c.n50 = 0;
        if(c.atmID.empty()) c.atmID = "ATM001";
        CASH[cashCnt++] = c;
        if(cashCnt >= ATM_MAX_CASH) break;
    }
    f.close();
}

void loadRates(){
    ifstream f("atm_rates.csv");
    if(!f){ cout<<"Warning: atm_rates.csv not found -> continuing with default rates.\n"; return; }
    string line;
    rateCnt = 0;
    while(getline(f,line)){
        if(line.empty()) continue;
        stringstream ss(line);
        ATMRate r; string tmp;
        getline(ss,tmp,','); r.code = trim(tmp);
        getline(ss,tmp,',');
        if(!safe_stod(trim(tmp), r.toINR)) continue;
        if(r.code.empty()) continue;
        RATE[rateCnt++] = r;
        if(rateCnt >= ATM_MAX_RATES) break;
    }
    f.close();
}

void loadTx(){
    ifstream f("atm_transactions.csv");
    if(!f){ cout<<"Warning: atm_transactions.csv not found -> starting with empty transactions.\n"; return; }
    string line;
    txCnt = 0;
    while(getline(f,line)){
        if(line.empty()) continue;
        stringstream ss(line);
        ATMTransaction t; string tmp;

        // try to parse id; if fail, skip row (handles header or bad rows)
        if(!getline(ss,tmp,',')) continue;
        if(!safe_stoi(trim(tmp), t.id)) continue;
        getline(ss,t.acc,','); t.acc = trim(t.acc);
        getline(ss,t.type,','); t.type = trim(t.type);
        if(!getline(ss,tmp,',')) continue;
        if(!safe_stod(trim(tmp), t.amt)) continue;
        getline(ss,t.currency,','); t.currency = trim(t.currency);
        getline(ss,t.atmID,','); t.atmID = trim(t.atmID);
        getline(ss,t.time,','); t.time = trim(t.time);
        if(!getline(ss,tmp,',')) continue;
        if(!safe_stod(trim(tmp), t.balAfter)) continue;
        getline(ss,t.remark,','); t.remark = trim(t.remark);

        TX[txCnt++] = t;
        if(txCnt >= ATM_MAX_TRANSACTIONS) break;
    }
    f.close();
}

void loadAll(){
    loadAccounts();
    loadCash();
    loadRates();
    loadTx();
    dataLoaded = true;
    cout<<"CSV LOAD COMPLETE. (manual load done)\n";
}

void logTx(const string &acc,const string &type,double amt,const string &remark){
    if(txCnt >= ATM_MAX_TRANSACTIONS) return;
    ATMTransaction t;
    t.id = txCnt + 1;
    t.acc = acc;
    t.type = type;
    t.amt = amt;
    t.currency = "INR";
    t.atmID = (cashCnt>0 ? CASH[0].atmID : string("ATM001"));
    t.time = "TODAY";
    int idx = findAcc(acc);
    t.balAfter = (idx==-1? 0.0 : ACC[idx].balance);
    t.remark = remark;
    TX[txCnt++] = t;
}

bool dispense(int amt, ATMCash &c){
    long long total = (long long)c.n2000*2000 + (long long)c.n500*500 + (long long)c.n200*200 + (long long)c.n100*100 + (long long)c.n50*50;
    if(total < amt) return false;
    int r = amt;
    int a = min(c.n2000, r/2000); r -= a*2000;
    int b = min(c.n500, r/500); r -= b*500;
    int d = min(c.n200, r/200); r -= d*200;
    int e = min(c.n100, r/100); r -= e*100;
    int f = min(c.n50, r/50); r -= f*50;
    if(r != 0) return false;
    c.n2000 -= a; c.n500 -= b; c.n200 -= d; c.n100 -= e; c.n50 -= f;
    return true;
}

int login(){
    string a; int pin;
    cout<<"Enter Account Number: ";
    if(!(cin>>a)){ cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n'); return -1; }
    int i = findAcc(a);
    if(i == -1){ cout<<"Not found.\n"; return -1; }
    if(ACC[i].locked){ cout<<"Locked.\n"; return -1; }
    cout<<"PIN: ";
    if(!(cin>>pin)){ cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n'); return -1; }
    if(pin != ACC[i].pin){
        ACC[i].wrongPin++;
        if(ACC[i].wrongPin >= ATM_PIN_ATTEMPT_LIMIT) ACC[i].locked = 1;
        cout<<"Wrong PIN.\n";
        return -1;
    }
    ACC[i].wrongPin = 0;
    return i;
}

void checkBal(int i){
    cout<<"Balance: "<<ACC[i].balance<<" "<<ACC[i].currency<<"\n";
    logTx(ACC[i].number,"BAL_CHECK",0,"Balance Inquiry");
}

void deposit(int i){
    double amt;
    cout<<"Enter deposit amount: ";
    if(!(cin>>amt)){ cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n'); cout<<"Invalid input.\n"; return; }
    if(amt <= 0 || amt > ATM_MAX_DEPOSIT_PER_TXN){ cout<<"Invalid deposit amount.\n"; return; }
    ACC[i].balance += amt;
    logTx(ACC[i].number,"DEPOSIT",amt,"Cash Deposit");
    cout<<"Deposit successful. New balance: "<<ACC[i].balance<<"\n";
}

void withdraw(int i){
    double amt;
    cout<<"Enter withdrawal amount: ";
    if(!(cin>>amt)){ cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n'); cout<<"Invalid input.\n"; return; }
    if(amt <= 0 || amt > ATM_MAX_WITHDRAW_PER_TXN){ cout<<"Invalid withdrawal amount.\n"; return; }
    if(ACC[i].dayWithdraw + amt > ATM_DAILY_LIMIT){ cout<<"Daily withdrawal limit exceeded.\n"; return; }
    double required = amt;
    if(ACC[i].currency != "INR"){
        double conv = fromINR(amt, ACC[i].currency);
        double fee = feeINR(amt);
        double feeConv = fromINR(fee, ACC[i].currency);
        required = conv + feeConv;
        cout<<"International conversion: "<<conv<<" "<<ACC[i].currency<<", fee: "<<feeConv<<" "<<ACC[i].currency<<"\n";
    }
    if(required > ACC[i].balance){ cout<<"Insufficient funds after conversion.\n"; return; }
    if(cashCnt == 0){ cout<<"ATM cash not loaded.\n"; return; }
    if(!dispense((int)amt, CASH[0])){ cout<<"ATM cannot dispense this amount exactly.\n"; return; }
    ACC[i].balance -= required;
    ACC[i].dayWithdraw += amt;
    logTx(ACC[i].number,"WITHDRAW",amt,"Cash Withdrawal");
    cout<<"Withdrawal successful. New balance: "<<ACC[i].balance<<"\n";
}

bool adminLogin(){
    cout<<"Enter ADMIN PIN: ";
    int p; if(!(cin>>p)){ cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n'); return false; }
    return p == 9999;
}

void adminAddAcc(){
    if(accCnt >= ATM_MAX_ACCOUNTS){ cout<<"Account storage full.\n"; return; }
    ATMAccount a;
    cout<<"Enter Account Number: "; cin>>a.number;
    if(findAcc(a.number) != -1){ cout<<"Account already exists.\n"; return; }
    cout<<"Customer Name: "; cin.ignore(); getline(cin, a.name);
    cout<<"PIN (numeric): "; cin>>a.pin;
    cout<<"Account Type (SAVINGS/CURRENT): "; cin>>a.type;
    cout<<"Currency (INR/USD/EUR): "; cin>>a.currency;
    cout<<"Initial Balance: "; cin>>a.balance;
    if(a.balance < 0){ cout<<"Balance cannot be negative.\n"; return; }
    cout<<"Withdrawal Limit: "; cin>>a.limit;
    cout<<"Card Country Code: "; cin>>a.cardCountry;
    a.locked = 0; a.wrongPin = 0; a.dayWithdraw = 0;
    ACC[accCnt++] = a;
    cout<<"Account created successfully.\n";
}

void adminEditAcc(){
    string an; cout<<"Enter Account Number to edit: "; cin>>an;
    int i = findAcc(an);
    if(i == -1){ cout<<"Account not found.\n"; return; }
    int ch = -1;
    do{
        cout<<"1.Change Name 2.Change PIN 3.Change Type 4.Change Balance 5.Unlock 6.Change Limit 7.Change Currency 0.Done\nChoice: ";
        if(!(cin>>ch)){ cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n'); cout<<"Invalid input.\n"; continue; }
        switch(ch){
            case 1: cin.ignore(); cout<<"New Name: "; getline(cin, ACC[i].name); break;
            case 2: cout<<"New PIN: "; cin>>ACC[i].pin; break;
            case 3: cout<<"New Type: "; cin>>ACC[i].type; break;
            case 4: { double nb; cout<<"New Balance: "; cin>>nb; if(nb < 0) cout<<"Cannot set negative balance.\n"; else ACC[i].balance = nb; break; }
            case 5: ACC[i].locked = 0; ACC[i].wrongPin = 0; cout<<"Account unlocked.\n"; break;
            case 6: cout<<"New Withdrawal Limit: "; cin>>ACC[i].limit; break;
            case 7: cout<<"New Currency: "; cin>>ACC[i].currency; break;
            case 0: break;
            default: cout<<"Invalid option.\n";
        }
    } while(ch != 0);
}

void adminCash(){
    if(cashCnt == 0){
        CASH[0].atmID = "ATM001";
        CASH[0].location = "MAIN";
        CASH[0].n2000 = CASH[0].n500 = CASH[0].n200 = CASH[0].n100 = CASH[0].n50 = 0;
        cashCnt = 1;
    }
    int a,b,d,e,f;
    cout<<"Enter counts to ADD for 2000 500 200 100 50 (space separated): ";
    if(!(cin>>a>>b>>d>>e>>f)){ cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n'); cout<<"Invalid input.\n"; return; }
    if(a<0||b<0||d<0||e<0||f<0){ cout<<"Cannot add negative notes.\n"; return; }
    CASH[0].n2000 += a; CASH[0].n500 += b; CASH[0].n200 += d; CASH[0].n100 += e; CASH[0].n50 += f;
    cout<<"ATM cash updated.\n";
}

void adminAddRate(){
    ATMRate r;
    cout<<"Enter currency code (e.g. USD): "; cin>>r.code;
    cout<<"Enter rate to INR (e.g. 83): "; cin>>r.toINR;
    if(r.toINR <= 0){ cout<<"Invalid rate.\n"; return; }
    RATE[rateCnt++] = r;
    cout<<"Rate added.\n";
}

void userMenu(int idx){
    while(true){
        int c;
        cout<<"1.Check Balance 2.Withdraw 3.Deposit 0.Logout\nChoice: ";
        if(!(cin>>c)){ cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n'); cout<<"Invalid input.\n"; continue; }
        if(c==0) { cout<<"Logging out.\n"; return; }
        if(c==1) checkBal(idx);
        else if(c==2) withdraw(idx);
        else if(c==3) deposit(idx);
        else cout<<"Invalid option.\n";
    }
}

void adminMenu(){
    while(true){
        int c;
        cout<<"1.Add Account 2.Edit Account 3.Add/Refill Cash 4.Add Rate 5.List Accounts 0.Exit\nChoice: ";
        if(!(cin>>c)){ cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n'); cout<<"Invalid input.\n"; continue; }
        if(c==0){ cout<<"Exiting admin menu.\n"; return; }
        if(c==1) adminAddAcc();
        else if(c==2) adminEditAcc();
        else if(c==3) adminCash();
        else if(c==4) adminAddRate();
        else if(c==5){
            for(int i=0;i<accCnt;i++){
                cout<<ACC[i].number<<" | "<<ACC[i].name<<" | "<<ACC[i].balance<<" "<<ACC[i].currency<<" | "<<(ACC[i].locked?"LOCKED":"OK")<<"\n";
            }
        } else cout<<"Invalid option.\n";
    }
}

void atmSystem(){
    while(true){
        int c;
        cout<<"\nATM SYSTEM - Choose an option\n1.User Login 2.Admin Login 3.Load CSV (Manual) 0.Exit\nChoice: ";
        if(!(cin>>c)){ cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n'); cout<<"Invalid input.\n"; continue; }
        if(c==0){ cout<<"Exiting ATM.\n"; return; }
        if(c==3){ loadAll(); continue; }
        if(!dataLoaded){ cout<<"⚠ CSV FILES NOT LOADED. Please press 3 to load CSV files (manual load).\n"; continue; }
        if(c==1){
            int idx = login();
            if(idx != -1) userMenu(idx);
        } else if(c==2){
            if(adminLogin()) adminMenu();
        } else cout<<"Invalid choice.\n";
    }
}

int main(){
    cout<<"Starting ATM module. (CSV manual load available)\n";
    atmSystem();
    return 0;
}
