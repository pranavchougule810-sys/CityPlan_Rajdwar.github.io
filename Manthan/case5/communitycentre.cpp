#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
using namespace std;

/* ===================== CONFIG ===================== */
#define MAX_MEMBERS      2000
#define MAX_STAFF        1000
#define MAX_FACILITIES   500
#define MAX_EQUIPMENT    500
#define MAX_EVENTS       1500
#define MAX_BOOKINGS     5000
#define MAX_TXN          5000

/* ===================== HELPERS ===================== */
static int toInt(const string &s){ try{return stoi(s);}catch(...){return 0;} }
static double toDouble(const string &s){ try{return stod(s);}catch(...){return 0.0;} }

static int splitCSV(const string &line, string out[], int maxCols){
    int col = 0;
    string cur = "";
    bool inQ = false;
    for(char c : line){
        if(c == '"'){ inQ = !inQ; continue; }
        if(c == ',' && !inQ){
            if(col < maxCols) out[col++] = cur;
            cur.clear();
        } else cur.push_back(c);
    }
    if(col < maxCols) out[col++] = cur;
    return col;
}

static bool timeOverlap(const string &d1,const string &s1,const string &e1,
                        const string &d2,const string &s2,const string &e2){
    if(d1 != d2) return false;
    string a=d1+" "+s1, b=d1+" "+e1, c=d2+" "+s2, d=d2+" "+e2;
    return (a < d && c < b);
}

/* ===================== STRUCTS ===================== */
struct Member{ int id,age,active; string name,phone,email,mtype,join_date,address; };
struct Staff{ int id,active; double salary; string name,role,phone,email,join_date; };
struct Facility{ int id,capacity,active; double price; string name,type,location,from,to; };
struct Equipment{ int id,qty_total,qty_avail; string name,cond,last_maint; };
struct Event{ int id,org_member,facility,expected; double revenue; string title,date,start,end,status; };
struct Booking{ int id,event_id,member_id,facility_id; double total; string date,start,end,status; };
struct Revenue{ int id,src_id; double amount; string src,date,desc; };
struct Expense{ int id,related; double amount; string date,vendor,desc,type; };

/* ===================== GLOBAL ARRAYS ===================== */
static Member members[MAX_MEMBERS]; int memberCount=0;
static Staff staffs[MAX_STAFF]; int staffCount=0;
static Facility facilities[MAX_FACILITIES]; int facilityCount=0;
static Equipment equipmentArr[MAX_EQUIPMENT]; int equipmentCount=0;
static Event eventsArr[MAX_EVENTS]; int eventCount=0;
static Booking bookings[MAX_BOOKINGS]; int bookingCount=0;
static Revenue revenues[MAX_TXN]; int revenueCount=0;
static Expense expenses[MAX_TXN]; int expenseCount=0;

/* ===================== FORWARD DEFS ===================== */
bool communityCheckBookingOverlap(int fac,const string &d,const string &st,const string &en);
void communityBulkFindOverlaps();
void communityMainMenu();

/* ============================================================
   ===================== TABLE FORMATTER =======================
   ============================================================ */

static void printHeader(const string cols[], const int w[], int n){
    for(int i=0;i<n;i++){
        cout<<left<<setw(w[i])<<cols[i];
        if(i<n-1) cout<<" | ";
    }
    cout<<"\n";
    for(int i=0;i<n;i++){
        cout<<string(w[i],'-');
        if(i<n-1) cout<<"-+-";
    }
    cout<<"\n";
}

/* ============================================================
   ======================= LOADERS =============================
   (Each loader resets its own count)
   ============================================================ */

void communityLoadMembersCSV(const string &fn){
    memberCount = 0;
    ifstream in(fn);
    if(!in){ cout<<"Cannot open "<<fn<<"\n"; return; }
    string line; getline(in,line);
    while(getline(in,line)){
        string c[9]; int n = splitCSV(line,c,9);
        if(n<2) continue;
        Member &m = members[memberCount];
        m.id=toInt(c[0]); m.name=c[1]; m.age=(n>2?toInt(c[2]):0);
        m.phone=c[3]; m.email=c[4]; m.mtype=c[5];
        m.join_date=c[6]; m.address=c[7]; m.active=toInt(c[8]);
        if(m.id!=0) memberCount++;
    }
    cout<<"Loaded "<<memberCount<<" members\n";
}

void communityLoadStaffCSV(const string &fn){
    staffCount = 0;
    ifstream in(fn);
    if(!in){ cout<<"Cannot open "<<fn<<"\n"; return; }
    string line; getline(in,line);
    while(getline(in,line)){
        string c[8]; int n=splitCSV(line,c,8);
        if(n<2) continue;
        Staff &s = staffs[staffCount];
        s.id=toInt(c[0]); s.name=c[1]; s.role=c[2];
        s.phone=c[3]; s.email=c[4];
        s.salary=(n>5?toDouble(c[5]):0); s.join_date=c[6];
        s.active=(n>7?toInt(c[7]):1);
        if(s.id!=0) staffCount++;
    }
    cout<<"Loaded "<<staffCount<<" staff\n";
}

void communityLoadFacilitiesCSV(const string &fn){
    facilityCount = 0;
    ifstream in(fn);
    if(!in){ cout<<"Cannot open "<<fn<<"\n"; return; }
    string line; getline(in,line);
    while(getline(in,line)){
        string c[9]; int n=splitCSV(line,c,9);
        Facility &f=facilities[facilityCount];
        f.id=toInt(c[0]); f.name=c[1]; f.type=c[2];
        f.capacity=toInt(c[3]); f.price=toDouble(c[4]);
        f.location=c[5]; f.from=c[6]; f.to=c[7];
        f.active=(n>8?toInt(c[8]):1);
        if(f.id!=0) facilityCount++;
    }
    cout<<"Loaded "<<facilityCount<<" facilities\n";
}

void communityLoadEquipmentCSV(const string &fn){
    equipmentCount = 0;
    ifstream in(fn);
    if(!in){ cout<<"Cannot open "<<fn<<"\n"; return; }
    string line; getline(in,line);
    while(getline(in,line)){
        string c[6]; int n=splitCSV(line,c,6);
        Equipment &e = equipmentArr[equipmentCount];
        e.id=toInt(c[0]); e.name=c[1];
        e.qty_total=toInt(c[2]);
        e.qty_avail=toInt(c[3]);
        e.cond=c[4]; e.last_maint=c[5];
        if(e.id!=0) equipmentCount++;
    }
    cout<<"Loaded "<<equipmentCount<<" equipment\n";
}

void communityLoadEventsCSV(const string &fn){
    eventCount=0;
    ifstream in(fn);
    if(!in){ cout<<"Cannot open "<<fn<<"\n"; return; }
    string l; getline(in,l);
    while(getline(in,l)){
        string c[10]; splitCSV(l,c,10);
        Event &e = eventsArr[eventCount];
        e.id=toInt(c[0]); e.title=c[1];
        e.org_member=toInt(c[2]); e.facility=toInt(c[3]);
        e.date=c[4]; e.start=c[5]; e.end=c[6];
        e.expected=toInt(c[7]); e.revenue=toDouble(c[8]);
        e.status=c[9];
        if(e.id!=0) eventCount++;
    }
    cout<<"Loaded "<<eventCount<<" events\n";
}

bool communityCheckBookingOverlap(int fac,const string &d,const string &st,const string &en){
    for(int i=0;i<bookingCount;i++){
        Booking &b=bookings[i];
        if(b.facility_id==fac && b.date==d){
            if(timeOverlap(d,st,en,b.date,b.start,b.end))
                return true;
        }
    }
    return false;
}

void communityLoadBookingsCSV(const string &fn){
    bookingCount=0;
    ifstream in(fn);
    if(!in){ cout<<"Cannot open "<<fn<<"\n"; return; }
    string l; getline(in,l);
    while(getline(in,l)){
        string c[9]; splitCSV(l,c,9);
        Booking b;
        b.id=toInt(c[0]); b.event_id=toInt(c[1]);
        b.member_id=toInt(c[2]); b.facility_id=toInt(c[3]);
        b.date=c[4]; b.start=c[5]; b.end=c[6];
        b.total=toDouble(c[7]); b.status=c[8];
        if(b.id==0) continue;

        if(communityCheckBookingOverlap(b.facility_id,b.date,b.start,b.end)){
            cout<<"Skip overlap booking "<<b.id<<"\n"; 
            continue;
        }
        bookings[bookingCount++] = b;
    }
    cout<<"Loaded "<<bookingCount<<" bookings\n";
}

void communityLoadRevenueCSV(const string &fn){
    revenueCount=0;
    ifstream in(fn);
    if(!in){ cout<<"Cannot open "<<fn<<"\n"; return; }
    string l; getline(in,l);
    while(getline(in,l)){
        string c[6]; splitCSV(l,c,6);
        Revenue &r = revenues[revenueCount];
        r.id=toInt(c[0]); r.src=c[1]; r.src_id=toInt(c[2]);
        r.date=c[3]; r.amount=toDouble(c[4]); r.desc=c[5];
        if(r.id!=0) revenueCount++;
    }
    cout<<"Loaded "<<revenueCount<<" revenue rows\n";
}

void communityLoadExpensesCSV(const string &fn){
    expenseCount=0;
    ifstream in(fn);
    if(!in){ cout<<"Cannot open "<<fn<<"\n"; return; }
    string l; getline(in,l);
    while(getline(in,l)){
        string c[7]; splitCSV(l,c,7);
        Expense &e=expenses[expenseCount];
        e.id=toInt(c[0]); e.related=toInt(c[1]);
        e.date=c[2]; e.amount=toDouble(c[3]);
        e.vendor=c[4]; e.desc=c[5]; e.type=c[6];
        if(e.id!=0) expenseCount++;
    }
    cout<<"Loaded "<<expenseCount<<" expenses\n";
}

/* Load ALL */
void communityLoadAllCSVsFromFolder(const string &p){
    string x=p;
    if(!x.empty() && x.back()!='/' && x.back()!='\\') x.push_back('/');
    memberCount=staffCount=facilityCount=equipmentCount=eventCount=bookingCount=revenueCount=expenseCount=0;
    communityLoadMembersCSV(x+"communityMembers.csv");
    communityLoadStaffCSV(x+"communityStaff.csv");
    communityLoadFacilitiesCSV(x+"communityFacilities.csv");
    communityLoadEquipmentCSV(x+"communityEquipment.csv");
    communityLoadEventsCSV(x+"communityEvents.csv");
    communityLoadBookingsCSV(x+"communityBookings.csv");
    communityLoadRevenueCSV(x+"communityRevenue.csv");
    communityLoadExpensesCSV(x+"communityExpenses.csv");
}

/* ============================================================
   ====================== SORTING =============================
   ============================================================ */

static void swapB(Booking &a,Booking &b){ Booking t=a; a=b; b=t; }
static string bk(const Booking &b){ return b.date+" "+b.start; }

int partB(int l,int r){
    string p=bk(bookings[l]);
    int i=l,j=r+1;
    while(true){
        do{i++;} while(i<=r && bk(bookings[i])<p);
        do{j--;} while(j>=l && bk(bookings[j])>p);
        if(i>=j) break;
        swapB(bookings[i],bookings[j]);
    }
    swapB(bookings[l],bookings[j]);
    return j;
}
void qsortB(int l,int r){ if(l<r){ int s=partB(l,r); qsortB(l,s-1); qsortB(s+1,r);} }

void communitySortBookings(){
    if(bookingCount>1) qsortB(0,bookingCount-1);
    cout<<"Bookings sorted\n";
}

/* Facilities sorting */
static void swapF(Facility &a,Facility &b){ Facility t=a;a=b;b=t; }

int partFP(int l,int r){
    double p=facilities[l].price;
    int i=l,j=r+1;
    while(true){
        do{i++;} while(i<=r && facilities[i].price<p);
        do{j--;} while(j>=l && facilities[j].price>p);
        if(i>=j) break;
        swapF(facilities[i],facilities[j]);
    }
    swapF(facilities[l],facilities[j]);
    return j;
}
void qsortFP(int l,int r){ if(l<r){ int s=partFP(l,r); qsortFP(l,s-1); qsortFP(s+1,r);} }
void communitySortFacilitiesByPrice(){ if(facilityCount>1) qsortFP(0,facilityCount-1); }

int partFC(int l,int r){
    int p=facilities[l].capacity;
    int i=l,j=r+1;
    while(true){
        do{i++;} while(i<=r && facilities[i].capacity<p);
        do{j--;} while(j>=l && facilities[j].capacity>p);
        if(i>=j) break;
        swapF(facilities[i],facilities[j]);
    }
    swapF(facilities[l],facilities[j]);
    return j;
}
void qsortFC(int l,int r){ if(l<r){ int s=partFC(l,r); qsortFC(l,s-1); qsortFC(s+1,r);} }
void communitySortFacilitiesByCapacity(){ if(facilityCount>1) qsortFC(0,facilityCount-1); }

void communityBulkFindOverlaps(){
    communitySortBookings();
    for(int i=1;i<bookingCount;i++){
        Booking &a=bookings[i-1], &b=bookings[i];
        if(a.facility_id==b.facility_id && a.date==b.date){
            if(timeOverlap(a.date,a.start,a.end,b.date,b.start,b.end))
                cout<<"Overlap: "<<a.id<<" <-> "<<b.id<<" ("<<a.date<<")\n";
        }
    }
}

/* ============================================================
   ====================== MANUAL ADD ==========================
   ============================================================ */

void addMember(){
    if(memberCount>=MAX_MEMBERS){ cout<<"Full\n"; return; }
    Member &m=members[memberCount];
    cout<<"ID: "; cin>>m.id; cin.ignore();
    cout<<"Name: "; getline(cin,m.name);
    cout<<"Age: "; cin>>m.age; cin.ignore();
    cout<<"Phone: "; getline(cin,m.phone);
    cout<<"Email: "; getline(cin,m.email);
    cout<<"Type: "; getline(cin,m.mtype);
    cout<<"Join Date: "; getline(cin,m.join_date);
    cout<<"Address: "; getline(cin,m.address);
    m.active=1; memberCount++;
}

void addStaff(){
    if(staffCount>=MAX_STAFF){ cout<<"Full\n"; return; }
    Staff &s=staffs[staffCount];
    cout<<"ID: "; cin>>s.id; cin.ignore();
    cout<<"Name: "; getline(cin,s.name);
    cout<<"Role: "; getline(cin,s.role);
    cout<<"Phone: "; getline(cin,s.phone);
    cout<<"Email: "; getline(cin,s.email);
    cout<<"Salary: "; cin>>s.salary; cin.ignore();
    cout<<"Join Date: "; getline(cin,s.join_date);
    s.active=1; staffCount++;
}

void addFacility(){
    if(facilityCount>=MAX_FACILITIES){ cout<<"Full\n"; return; }
    Facility &f=facilities[facilityCount];
    cout<<"ID: "; cin>>f.id; cin.ignore();
    cout<<"Name: "; getline(cin,f.name);
    cout<<"Type: "; getline(cin,f.type);
    cout<<"Capacity: "; cin>>f.capacity; cin.ignore();
    cout<<"Price: "; cin>>f.price; cin.ignore();
    cout<<"Location: "; getline(cin,f.location);
    cout<<"From: "; getline(cin,f.from);
    cout<<"To: "; getline(cin,f.to);
    f.active=1; facilityCount++;
}

void addEquipment(){
    if(equipmentCount>=MAX_EQUIPMENT){ cout<<"Full\n"; return; }
    Equipment &e=equipmentArr[equipmentCount];
    cout<<"ID: "; cin>>e.id; cin.ignore();
    cout<<"Name: "; getline(cin,e.name);
    cout<<"Total Qty: "; cin>>e.qty_total; cin.ignore();
    cout<<"Available Qty: "; cin>>e.qty_avail; cin.ignore();
    cout<<"Cond: "; getline(cin,e.cond);
    cout<<"Last Maint: "; getline(cin,e.last_maint);
    equipmentCount++;
}

void addEvent(){
    if(eventCount>=MAX_EVENTS){ cout<<"Full\n"; return; }
    Event &ev=eventsArr[eventCount];
    cout<<"ID: "; cin>>ev.id; cin.ignore();
    cout<<"Title: "; getline(cin,ev.title);
    cout<<"Org Member: "; cin>>ev.org_member;
    cout<<"Facility: "; cin>>ev.facility; cin.ignore();
    cout<<"Date: "; getline(cin,ev.date);
    cout<<"Start: "; getline(cin,ev.start);
    cout<<"End: "; getline(cin,ev.end);
    cout<<"Expected: "; cin>>ev.expected;
    cout<<"Revenue Est: "; cin>>ev.revenue; cin.ignore();
    ev.status="scheduled";
    eventCount++;
}

void addBooking(){
    if(bookingCount>=MAX_BOOKINGS){ cout<<"Full\n"; return; }
    Booking b;
    cout<<"ID: "; cin>>b.id;
    cout<<"Event: "; cin>>b.event_id;
    cout<<"Member: "; cin>>b.member_id;
    cout<<"Facility: "; cin>>b.facility_id; cin.ignore();
    cout<<"Date: "; getline(cin,b.date);
    cout<<"Start: "; getline(cin,b.start);
    cout<<"End: "; getline(cin,b.end);

    if(communityCheckBookingOverlap(b.facility_id,b.date,b.start,b.end)){
        cout<<"Overlap rejected\n"; return;
    }

    cout<<"Total: "; cin>>b.total; cin.ignore();
    cout<<"Status: "; getline(cin,b.status);

    bookings[bookingCount++] = b;

if(b.status == "paid" && revenueCount < MAX_TXN){
    revenues[revenueCount].id = revenueCount;
    revenues[revenueCount].src = "booking";
    revenues[revenueCount].src_id = b.id;
    revenues[revenueCount].date = b.date;
    revenues[revenueCount].amount = b.total;
    revenues[revenueCount].desc = "auto";
    revenueCount++;
}

}

void addRevenue(){
    if(revenueCount>=MAX_TXN){ cout<<"Full\n"; return; }
    Revenue &r=revenues[revenueCount];
    cout<<"ID: "; cin>>r.id; cin.ignore();
    cout<<"Source: "; getline(cin,r.src);
    cout<<"Source ID: "; cin>>r.src_id; cin.ignore();
    cout<<"Date: "; getline(cin,r.date);
    cout<<"Amount: "; cin>>r.amount; cin.ignore();
    cout<<"Desc: "; getline(cin,r.desc);
    revenueCount++;
}

void addExpense(){
    if(expenseCount>=MAX_TXN){ cout<<"Full\n"; return; }
    Expense &e=expenses[expenseCount];
    cout<<"ID: "; cin>>e.id;
    cout<<"Related Event: "; cin>>e.related; cin.ignore();
    cout<<"Date: "; getline(cin,e.date);
    cout<<"Amount: "; cin>>e.amount; cin.ignore();
    cout<<"Vendor: "; getline(cin,e.vendor);
    cout<<"Desc: "; getline(cin,e.desc);
    cout<<"Type: "; getline(cin,e.type);
    expenseCount++;
}

/* ============================================================
   ======================== LISTING ============================
   ============================================================ */

void listMembers(){
    const string c[]={"ID","Name","Phone","Email","Type","Join"};
    int w[]={5,16,12,20,10,12};
    printHeader(c,w,6);
    for(int i=0;i<memberCount;i++){
        Member &m=members[i];
        cout<<setw(w[0])<<m.id<<" | "<<setw(w[1])<<m.name<<" | "<<setw(w[2])<<m.phone<<" | "<<setw(w[3])<<m.email<<" | "<<setw(w[4])<<m.mtype<<" | "<<setw(w[5])<<m.join_date<<"\n";
    }
}

void listStaff(){
    const string c[]={"ID","Name","Role","Phone","Salary","Join"};
    int w[]={5,16,12,12,10,12};
    printHeader(c,w,6);
    for(int i=0;i<staffCount;i++){
        Staff &s=staffs[i];
        cout<<setw(w[0])<<s.id<<" | "<<setw(w[1])<<s.name<<" | "<<setw(w[2])<<s.role<<" | "<<setw(w[3])<<s.phone<<" | "<<setw(w[4])<<s.salary<<" | "<<setw(w[5])<<s.join_date<<"\n";
    }
}

void listFacilities(){
    const string c[]={"ID","Name","Type","Cap","Price","Location"};
    int w[]={5,16,10,6,10,14};
    printHeader(c,w,6);
    for(int i=0;i<facilityCount;i++){
        Facility &f=facilities[i];
        cout<<setw(w[0])<<f.id<<" | "<<setw(w[1])<<f.name<<" | "<<setw(w[2])<<f.type<<" | "<<setw(w[3])<<f.capacity<<" | "<<setw(w[4])<<f.price<<" | "<<setw(w[5])<<f.location<<"\n";
    }
}

void listEquipment(){
    const string c[]={"ID","Name","Total","Avail","Cond","LastMaint"};
    int w[]={5,16,7,7,10,12};
    printHeader(c,w,6);
    for(int i=0;i<equipmentCount;i++){
        Equipment &e=equipmentArr[i];
        cout<<setw(w[0])<<e.id<<" | "<<setw(w[1])<<e.name<<" | "<<setw(w[2])<<e.qty_total<<" | "<<setw(w[3])<<e.qty_avail<<" | "<<setw(w[4])<<e.cond<<" | "<<setw(w[5])<<e.last_maint<<"\n";
    }
}

void listEvents(){
    const string c[]={"ID","Title","Fac","Date","Start","End","Exp","Rev","Status"};
    int w[]={5,16,5,12,7,7,6,10,10};
    printHeader(c,w,9);
    for(int i=0;i<eventCount;i++){
        Event &e=eventsArr[i];
        cout<<setw(w[0])<<e.id<<" | "<<setw(w[1])<<e.title<<" | "<<setw(w[2])<<e.facility<<" | "<<setw(w[3])<<e.date<<" | "
            <<setw(w[4])<<e.start<<" | "<<setw(w[5])<<e.end<<" | "<<setw(w[6])<<e.expected<<" | "<<setw(w[7])<<e.revenue<<" | "<<setw(w[8])<<e.status<<"\n";
    }
}

void listBookings(){
    const string c[]={"ID","Event","Member","Facility","Date","Start","End","Amt","Status"};
    int w[]={5,7,7,8,12,7,7,10,10};
    printHeader(c,w,9);
    for(int i=0;i<bookingCount;i++){
        Booking &b=bookings[i];
        cout<<setw(w[0])<<b.id<<" | "<<setw(w[1])<<b.event_id<<" | "<<setw(w[2])<<b.member_id<<" | "<<setw(w[3])<<b.facility_id<<" | "
            <<setw(w[4])<<b.date<<" | "<<setw(w[5])<<b.start<<" | "<<setw(w[6])<<b.end<<" | "<<setw(w[7])<<b.total<<" | "<<setw(w[8])<<b.status<<"\n";
    }
}

/* ============================================================
   ======================== FINANCE ============================
   ============================================================ */

void eventPnL(int eid){
    double R=0,E=0;
    for(int i=0;i<revenueCount;i++){
        Revenue &r=revenues[i];
        if(r.src=="booking"){
            for(int j=0;j<bookingCount;j++)
                if(bookings[j].id==r.src_id && bookings[j].event_id==eid)
                    R+=r.amount;
        }
        else if(r.src=="event" && r.src_id==eid)
            R+=r.amount;
    }
    for(int i=0;i<expenseCount;i++)
        if(expenses[i].related==eid) E+=expenses[i].amount;
    cout<<"P&L for event "<<eid<<" = "<<(R-E)<<" (Rev: "<<R<<"  Exp: "<<E<<")\n";
}

void monthlyRevenue(const string &m){
    double t=0; 
    for(int i=0;i<revenueCount;i++)
        if(revenues[i].date.rfind(m,0)==0) t+=revenues[i].amount;
    cout<<"Revenue for "<<m<<" = "<<t<<"\n";
}

void bookingsRange(const string &f,const string &t){
    const string c[]={"ID","Event","Mem","Fac","Date","Start","End","Amt"};
    int w[]={5,6,6,6,12,7,7,10};
    printHeader(c,w,8);
    for(int i=0;i<bookingCount;i++){
        Booking &b=bookings[i];
        if(b.date>=f && b.date<=t)
            cout<<setw(w[0])<<b.id<<" | "<<setw(w[1])<<b.event_id<<" | "<<setw(w[2])<<b.member_id<<" | "
                <<setw(w[3])<<b.facility_id<<" | "<<setw(w[4])<<b.date<<" | "<<setw(w[5])<<b.start<<" | "
                <<setw(w[6])<<b.end<<" | "<<setw(w[7])<<b.total<<"\n";
    }
}

/* ============================================================
   ========================= MAIN MENU =========================
   ============================================================ */

void communitySystem(){
    while(true){
        cout<<"\n==== COMMUNITY CENTRE ====\n"
            <<"1. Members\n"
            <<"2. Staff\n"
            <<"3. Facilities\n"
            <<"4. Equipment\n"
            <<"5. Events & Bookings\n"
            <<"6. Finance\n"
            <<"7. Reports\n"
            <<"8. Load ALL CSVs\n"
            <<"0. Return\nChoice: ";

        int c; cin>>c; cin.ignore();

        switch(c){

        case 1:{
            cout<<"1=Load 2=Add 3=List\nChoice: ";
            int s; cin>>s; cin.ignore();
            if(s==1) communityLoadMembersCSV("communityMembers.csv");
            else if(s==2) addMember();
            else if(s==3) listMembers();
            break;
        }

        case 2:{
            cout<<"1=Load 2=Add 3=List 4=Payroll\nChoice: ";
            int s; cin>>s; cin.ignore();
            if(s==1) communityLoadStaffCSV("communityStaff.csv");
            else if(s==2) addStaff();
            else if(s==3) listStaff();
            else if(s==4){
                double total=0;
                for(int i=0;i<staffCount;i++) total+=staffs[i].salary;
                cout<<"Total payroll = "<<total<<"\n";
            }
            break;
        }

        case 3:{
            cout<<"1=Load 2=Add 3=List 4=SortPrice 5=SortCap\nChoice: ";
            int s; cin>>s; cin.ignore();
            if(s==1) communityLoadFacilitiesCSV("communityFacilities.csv");
            else if(s==2) addFacility();
            else if(s==3) listFacilities();
            else if(s==4) communitySortFacilitiesByPrice();
            else if(s==5) communitySortFacilitiesByCapacity();
            break;
        }

        case 4:{
            cout<<"1=Load 2=Add 3=List\nChoice: ";
            int s; cin>>s; cin.ignore();
            if(s==1) communityLoadEquipmentCSV("communityEquipment.csv");
            else if(s==2) addEquipment();
            else if(s==3) listEquipment();
            break;
        }

        case 5:{
            cout<<"1=LoadEvts 2=LoadBkng 3=AddEvt 4=AddBkng 5=ListEvts 6=ListBkng 7=Sort 8=Overlap\nChoice: ";
            int s; cin>>s; cin.ignore();
            if(s==1) communityLoadEventsCSV("communityEvents.csv");
            else if(s==2) communityLoadBookingsCSV("communityBookings.csv");
            else if(s==3) addEvent();
            else if(s==4) addBooking();
            else if(s==5) listEvents();
            else if(s==6) listBookings();
            else if(s==7) communitySortBookings();
            else if(s==8) communityBulkFindOverlaps();
            break;
        }

        case 6:{
            cout<<"1=AddRevenue 2=AddExpense 3=EventPnL\nChoice: ";
            int s; cin>>s; cin.ignore();
            if(s==1) addRevenue();
            else if(s==2) addExpense();
            else if(s==3){ int id; cout<<"Event id: "; cin>>id; eventPnL(id); }
            break;
        }

        case 7:{
            cout<<"1=Range 2=Monthly\nChoice: ";
            int s; cin>>s; cin.ignore();
            if(s==1){ string f,t; cout<<"From: "; getline(cin,f); cout<<"To: "; getline(cin,t); bookingsRange(f,t); }
            else if(s==2){ string m; cout<<"YYYY-MM: "; getline(cin,m); monthlyRevenue(m); }
            break;
        }

        case 8:{
            cout<<"Loading ALL...\n";
            communityLoadAllCSVsFromFolder("");
            break;
        }

        case 0: return;
        default: cout<<"Invalid\n";
        }
    }
}
int main(){
    communitySystem();
}

