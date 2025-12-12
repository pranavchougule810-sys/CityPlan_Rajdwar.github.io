// hospital_module_reduced.cpp
// Reduced Hospital Management module — appointments-only approach.
// Removed: ER processing, separate surgeries, ambulance, Dijkstra, interactive lab result input.
// Lab results are only loaded via CSV (hospitalLoadLabsCSV).
// Minimal helper functions included: toInt, splitCSV.

#include <bits/stdc++.h>
using namespace std;

#define HOSPITAL_MAX_PATIENTS 2000
#define HOSPITAL_MAX_STAFF 500
#define HOSPITAL_MAX_ROOMS 500
#define HOSPITAL_MAX_BEDS 2000
#define HOSPITAL_MAX_APPOINTS 2000
#define HOSPITAL_HASH_SIZE 32749
#define HOSPITAL_INF 999999

// ---------- Helper utilities (simple) ----------
int toInt(const string &s)
{
    if (s.empty()) return 0;
    try { return stoi(s); } catch (...) { return 0; }
}

// Very simple CSV splitter
// Handles quoted fields with commas (basic)
int splitCSV(const string &line, string out[], int maxCols)
{
    int col = 0;
    string cur;
    bool inq = false;
    for (size_t i = 0; i < line.size(); ++i)
    {
        char c = line[i];
        if (c == '"' )
        {
            inq = !inq;
            continue;
        }
        if (c == ',' && !inq)
        {
            if (col < maxCols) out[col++] = cur;
            cur.clear();
        }
        else
        {
            cur.push_back(c);
        }
    }
    if (col < maxCols) out[col++] = cur;
    return col;
}

// ---------- Entities ----------
struct HospitalPatient
{
    int patient_id;
    char name[128];
    int age;
    char gender[16];
    char contact[32];
    char address[256];
    int status; // 0 registered, 1 admitted, 3 discharged
    char notes[512];
    // minimal: more fields can be added
};

struct HospitalStaff
{
    int id;
    char name[128];
    char role[64];
    char department[64];
    int shift;
    int salary;
    char contact[32];
    char specialty[64];
};

struct HospitalRoom
{
    int roomID;
    char type[32];
    int capacity;
    int occupied;
    int bedStartIdx;
};

struct HospitalBed
{
    int bedID;
    int roomID;
    bool occupied;
    int patientID; // -1 if empty
};

// Appointment is now unified: general / surgery / lab test
struct HospitalAppointment
{
    int apptID;
    int type; // 1 = general, 2 = surgery, 3 = lab test
    int patientID;
    int doctorID;
    char date[16]; // YYYY-MM-DD
    char time[6];  // HH:MM
    int duration;  // minutes
    int status;    // 0 booked,1 done,2 cancelled

    // surgery-specific (optional, set when type==2)
    int OTroomID;
    int durationMins; // surgery duration
    char anesthesiaType[32];

    // lab-specific (optional, set when type==3)
    char testType[64];
    char resultDate[16];
    char resultSummary[256];

    char remarks[256];
};

// ---------- Module globals ----------
static HospitalPatient hospitalPatients[HOSPITAL_MAX_PATIENTS];
static int hospitalPatientCount = 0;
static int hospitalNextPatientID = 9000;

static HospitalStaff hospitalStaff[HOSPITAL_MAX_STAFF];
static int hospitalStaffCount = 0;
static int hospitalNextStaffID = 7000;

static HospitalRoom hospitalRooms[HOSPITAL_MAX_ROOMS];
static int hospitalRoomCount = 0;
static HospitalBed hospitalBeds[HOSPITAL_MAX_BEDS];
static int hospitalBedCount = 0;

static HospitalAppointment hospitalAppts[HOSPITAL_MAX_APPOINTS];
static int hospitalApptCount = 0;
static int hospitalNextApptID = 21000;

// ---------- Hash table for patients (by patient_id) ----------
struct HospitalPatientHashEntry
{
    int key;
    HospitalPatient val;
    bool used;
    bool deleted;
};
static HospitalPatientHashEntry hospitalPatientHash[HOSPITAL_HASH_SIZE];
static int hospitalPatientHashCount = 0;

unsigned int hospital_hash_key_int(int key)
{
    return (unsigned int)(key * 2654435761u) % HOSPITAL_HASH_SIZE;
}
void hospital_init_patient_hash()
{
    for (int i = 0; i < HOSPITAL_HASH_SIZE; ++i)
    {
        hospitalPatientHash[i].used = false;
        hospitalPatientHash[i].deleted = false;
        hospitalPatientHash[i].key = -1;
    }
    hospitalPatientHashCount = 0;
}
bool hospital_patient_hash_insert(const HospitalPatient &p)
{
    if (hospitalPatientHashCount >= HOSPITAL_MAX_PATIENTS) return false;
    unsigned int idx = hospital_hash_key_int(p.patient_id);
    unsigned int start = idx;
    while (hospitalPatientHash[idx].used && !hospitalPatientHash[idx].deleted && hospitalPatientHash[idx].key != p.patient_id)
    {
        idx = (idx + 1) % HOSPITAL_HASH_SIZE;
        if (idx == start) return false;
    }
    hospitalPatientHash[idx].used = true;
    hospitalPatientHash[idx].deleted = false;
    hospitalPatientHash[idx].key = p.patient_id;
    hospitalPatientHash[idx].val = p;
    hospitalPatientHashCount++;
    return true;
}
bool hospital_patient_hash_get(int key, HospitalPatient &out)
{
    unsigned int idx = hospital_hash_key_int(key);
    unsigned int start = idx;
    while (hospitalPatientHash[idx].used)
    {
        if (!hospitalPatientHash[idx].deleted && hospitalPatientHash[idx].key == key)
        {
            out = hospitalPatientHash[idx].val;
            return true;
        }
        idx = (idx + 1) % HOSPITAL_HASH_SIZE;
        if (idx == start) break;
    }
    return false;
}
bool hospital_patient_hash_remove(int key)
{
    unsigned int idx = hospital_hash_key_int(key);
    unsigned int start = idx;
    while (hospitalPatientHash[idx].used)
    {
        if (!hospitalPatientHash[idx].deleted && hospitalPatientHash[idx].key == key)
        {
            hospitalPatientHash[idx].deleted = true;
            hospitalPatientHashCount--;
            return true;
        }
        idx = (idx + 1) % HOSPITAL_HASH_SIZE;
        if (idx == start) break;
    }
    return false;
}

// ---------- Date/time helpers ----------
static string hospital_now_date()
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char buf[32];
    sprintf(buf, "%04d-%02d-%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
    return string(buf);
}
static string hospital_now_time()
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char buf[16];
    sprintf(buf, "%02d:%02d", tm->tm_hour, tm->tm_min);
    return string(buf);
}

// ---------- CSV loaders & generators ----------
void hospitalLoadPatientsCSV(const string &fn)
{
    ifstream in(fn.c_str());
    if (!in.is_open()) { cout << "Cannot open " << fn << "\n"; return; }
    string line;
    getline(in, line); // header
    int loaded = 0;
    while (getline(in, line))
    {
        if (line.size() < 2) continue;
        string cols[8];
        int n = splitCSV(line, cols, 8);
        if (n < 2) continue;
        if (hospitalPatientCount >= HOSPITAL_MAX_PATIENTS) break;
        HospitalPatient p;
        p.patient_id = toInt(cols[0]);
        if (p.patient_id == 0) p.patient_id = ++hospitalNextPatientID;
        strncpy(p.name, cols[1].c_str(), sizeof(p.name) - 1); p.name[sizeof(p.name)-1]=0;
        p.age = (n>=3?toInt(cols[2]):0);
        strncpy(p.gender,(n>=4?cols[3].c_str():""),sizeof(p.gender)-1);
        strncpy(p.contact,(n>=5?cols[4].c_str():""),sizeof(p.contact)-1);
        strncpy(p.address,(n>=6?cols[5].c_str():""),sizeof(p.address)-1);
        p.status = (n>=7?toInt(cols[6]):0);
        strncpy(p.notes,(n>=8?cols[7].c_str():""),sizeof(p.notes)-1);
        hospitalPatients[hospitalPatientCount] = p;
        hospitalPatientCount++;
        hospital_patient_hash_insert(p);
        loaded++;
    }
    cout << "Loaded " << loaded << " patients from " << fn << "\n";
}

void hospitalLoadStaffCSV(const string &fn)
{
    ifstream in(fn.c_str());
    if (!in.is_open()) { cout << "Cannot open " << fn << "\n"; return; }
    string line;
    getline(in, line);
    int loaded = 0;
    while (getline(in, line))
    {
        if (line.size() < 2) continue;
        string cols[8];
        int n = splitCSV(line, cols, 8);
        if (n < 2) continue;
        if (hospitalStaffCount >= HOSPITAL_MAX_STAFF) break;
        HospitalStaff s;
        s.id = toInt(cols[0]); if (s.id == 0) s.id = ++hospitalNextStaffID;
        strncpy(s.name, cols[1].c_str(), sizeof(s.name)-1); s.name[sizeof(s.name)-1]=0;
        strncpy(s.role, (n>=3?cols[2].c_str():""), sizeof(s.role)-1);
        strncpy(s.department, (n>=4?cols[3].c_str():""), sizeof(s.department)-1);
        s.shift = (n>=5?toInt(cols[4]):0);
        s.salary = (n>=6?toInt(cols[5]):0);
        strncpy(s.contact, (n>=7?cols[6].c_str():""), sizeof(s.contact)-1);
        strncpy(s.specialty, (n>=8?cols[7].c_str():""), sizeof(s.specialty)-1);
        hospitalStaff[hospitalStaffCount++] = s;
        loaded++;
    }
    cout << "Loaded " << loaded << " staff from " << fn << "\n";
}

void hospitalLoadRoomsCSV(const string &fn)
{
    ifstream in(fn.c_str());
    if (!in.is_open()) { cout << "Cannot open " << fn << "\n"; return; }
    string line;
    getline(in, line);
    int loaded = 0;
    while (getline(in, line))
    {
        if (line.size() < 2) continue;
        string cols[6];
        int n = splitCSV(line, cols, 6);
        if (n < 3) continue;
        if (hospitalRoomCount >= HOSPITAL_MAX_ROOMS) break;
        HospitalRoom r;
        r.roomID = toInt(cols[0]);
        strncpy(r.type, cols[1].c_str(), sizeof(r.type)-1); r.type[sizeof(r.type)-1]=0;
        r.capacity = toInt(cols[2]);
        r.occupied = 0;
        r.bedStartIdx = hospitalBedCount;
        for (int b = 0; b < r.capacity && hospitalBedCount < HOSPITAL_MAX_BEDS; ++b)
        {
            HospitalBed bd;
            bd.bedID = hospitalBedCount + 10000;
            bd.roomID = r.roomID;
            bd.occupied = false;
            bd.patientID = -1;
            hospitalBeds[hospitalBedCount++] = bd;
        }
        hospitalRooms[hospitalRoomCount++] = r;
        loaded++;
    }
    cout << "Loaded " << loaded << " rooms from " << fn << "\n";
}

// Appointments CSV loader: columns suggested:
// apptID, type(1=gen,2=surg,3=lab), patientID, doctorID, date, time, duration, status, remarks, (for surgery: OTroomID,durationMins,anesthesia),(for lab: testType,resultDate,resultSummary)
void hospitalLoadAppointmentsCSV(const string &fn)
{
    ifstream in(fn.c_str());
    if (!in.is_open()) { cout << "Cannot open " << fn << "\n"; return; }
    string line;
    getline(in, line);
    int loaded = 0;
    while (getline(in, line))
    {
        if (line.size() < 2) continue;
        string cols[20];
        int n = splitCSV(line, cols, 20);
        if (n < 6) continue;
        if (hospitalApptCount >= HOSPITAL_MAX_APPOINTS) break;
        HospitalAppointment a;
        a.apptID = toInt(cols[0]); if (a.apptID == 0) a.apptID = ++hospitalNextApptID;
        a.type = toInt(cols[1]);
        a.patientID = toInt(cols[2]);
        a.doctorID = toInt(cols[3]);
        strncpy(a.date, cols[4].c_str(), sizeof(a.date)-1); a.date[sizeof(a.date)-1]=0;
        strncpy(a.time, cols[5].c_str(), sizeof(a.time)-1); a.time[sizeof(a.time)-1]=0;
        a.duration = (n>=7?toInt(cols[6]):15);
        a.status = (n>=8?toInt(cols[7]):0);
        if (n >= 9) strncpy(a.remarks, cols[8].c_str(), sizeof(a.remarks)-1); else strncpy(a.remarks,"",sizeof(a.remarks)-1);
        // surgery fields
        a.OTroomID = (n>=10?toInt(cols[9]):-1);
        a.durationMins = (n>=11?toInt(cols[10]):0);
        if (n >= 12) strncpy(a.anesthesiaType, cols[11].c_str(), sizeof(a.anesthesiaType)-1); else strncpy(a.anesthesiaType,"",sizeof(a.anesthesiaType)-1);
        // lab fields
        if (n >= 13) strncpy(a.testType, cols[12].c_str(), sizeof(a.testType)-1); else strncpy(a.testType,"",sizeof(a.testType)-1);
        if (n >= 14) strncpy(a.resultDate, cols[13].c_str(), sizeof(a.resultDate)-1); else strncpy(a.resultDate,"",sizeof(a.resultDate)-1);
        if (n >= 15) strncpy(a.resultSummary, cols[14].c_str(), sizeof(a.resultSummary)-1); else strncpy(a.resultSummary,"",sizeof(a.resultSummary)-1);
        hospitalAppts[hospitalApptCount++] = a;
        loaded++;
    }
    cout << "Loaded " << loaded << " appointments from " << fn << "\n";
}

// Lab CSV loader ONLY — used to update lab results (no interactive updating).
// Expected columns (recommended): apptID, patientID, testType, resultDate, resultSummary
void hospitalLoadLabsCSV(const string &fn)
{
    ifstream in(fn.c_str());
    if (!in.is_open()) { cout << "Cannot open " << fn << "\n"; return; }
    string line;
    getline(in, line);
    int updated = 0;
    while (getline(in, line))
    {
        if (line.size() < 2) continue;
        string cols[8];
        int n = splitCSV(line, cols, 8);
        if (n < 3) continue;
        int apptID = toInt(cols[0]);
        int pid = toInt(cols[1]);
        string testType = (n>=3?cols[2]:"");
        string resDate = (n>=4?cols[3]:"");
        string resSummary = (n>=5?cols[4]:"");
        // Prefer matching by apptID if provided (>0), else match by patientID + testType (first not-done)
        bool found = false;
        if (apptID > 0)
        {
            for (int i = 0; i < hospitalApptCount; ++i)
            {
                if (hospitalAppts[i].apptID == apptID && hospitalAppts[i].type == 3)
                {
                    if (!testType.empty()) strncpy(hospitalAppts[i].testType, testType.c_str(), sizeof(hospitalAppts[i].testType)-1);
                    if (!resDate.empty()) strncpy(hospitalAppts[i].resultDate, resDate.c_str(), sizeof(hospitalAppts[i].resultDate)-1);
                    if (!resSummary.empty()) strncpy(hospitalAppts[i].resultSummary, resSummary.c_str(), sizeof(hospitalAppts[i].resultSummary)-1);
                    updated++; found = true; break;
                }
            }
        }
        if (!found && pid > 0)
        {
            for (int i = 0; i < hospitalApptCount; ++i)
            {
                if (hospitalAppts[i].patientID == pid && hospitalAppts[i].type == 3)
                {
                    // Use additional matching: testType if specified
                    if (!testType.empty() && strlen(hospitalAppts[i].testType) > 0 && string(hospitalAppts[i].testType) != testType) continue;
                    if (!testType.empty()) strncpy(hospitalAppts[i].testType, testType.c_str(), sizeof(hospitalAppts[i].testType)-1);
                    if (!resDate.empty()) strncpy(hospitalAppts[i].resultDate, resDate.c_str(), sizeof(hospitalAppts[i].resultDate)-1);
                    if (!resSummary.empty()) strncpy(hospitalAppts[i].resultSummary, resSummary.c_str(), sizeof(hospitalAppts[i].resultSummary)-1);
                    updated++; found = true; break;
                }
            }
        }
    }
    cout << "Updated lab results from " << fn << ".\n";
}

// ---------- Interactive functions (menus) ----------
int hospital_createPatientID() { return ++hospitalNextPatientID; }
int hospital_createStaffID() { return ++hospitalNextStaffID; }
int hospital_createApptID() { return ++hospitalNextApptID; }

// Add a new patient (ER or OPD)
void hospitalAddPatient()
{
    if (hospitalPatientCount >= HOSPITAL_MAX_PATIENTS)
    {
        cout << "Overflow: patients capacity reached!\n";
        return;
    }
    HospitalPatient p;
    p.patient_id = hospital_createPatientID();
    cout << "Enter name: ";
    string tmp;
    getline(cin, tmp);
    strncpy(p.name, tmp.c_str(), sizeof(p.name) - 1); p.name[sizeof(p.name)-1]=0;
    cout << "Enter age: ";
    getline(cin, tmp);
    p.age = toInt(tmp);
    cout << "Enter gender: ";
    getline(cin, tmp);
    strncpy(p.gender, tmp.c_str(), sizeof(p.gender) - 1); p.gender[sizeof(p.gender)-1]=0;
    cout << "Enter contact: ";
    getline(cin, tmp);
    strncpy(p.contact, tmp.c_str(), sizeof(p.contact) - 1); p.contact[sizeof(p.contact)-1]=0;
    cout << "Enter address: ";
    getline(cin, tmp);
    strncpy(p.address, tmp.c_str(), sizeof(p.address) - 1); p.address[sizeof(p.address)-1]=0;
    p.status = 0;
    strncpy(p.notes, "", sizeof(p.notes)-1);
    hospitalPatients[hospitalPatientCount] = p;
    hospitalPatientCount++;
    hospital_patient_hash_insert(p);
    cout << "Patient registered. ID: " << p.patient_id << "\n";
}

// Admit patient by ID to specified room/bed
void hospitalAdmitPatient()
{
    cout << "Enter patient ID: ";
    int pid;
    if (!(cin >> pid)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout<<"Invalid.\n"; return; }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    HospitalPatient p;
    if (!hospital_patient_hash_get(pid, p)) { cout << "Patient not found.\n"; return; }
    cout << "Enter roomID to admit into: ";
    int rid;
    if (!(cin >> rid)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout<<"Invalid.\n"; return; }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    int bedIdx = -1;
    for (int i = 0; i < hospitalBedCount; i++)
        if (hospitalBeds[i].roomID == rid && !hospitalBeds[i].occupied)
        {
            bedIdx = i; break;
        }
    if (bedIdx == -1) { cout << "No free bed in room " << rid << "\n"; return; }
    hospitalBeds[bedIdx].occupied = true;
    hospitalBeds[bedIdx].patientID = pid;
    for (int i = 0; i < hospitalPatientCount; i++)
        if (hospitalPatients[i].patient_id == pid)
        {
            hospitalPatients[i].status = 1; // admitted
            hospital_patient_hash_remove(pid);
            hospital_patient_hash_insert(hospitalPatients[i]);
            break;
        }
    cout << "Admitted patient " << pid << " to bed " << hospitalBeds[bedIdx].bedID << "\n";
}

// Discharge patient
void hospitalDischargePatient()
{
    cout << "Enter patient ID to discharge: ";
    int pid;
    if (!(cin >> pid)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout<<"Invalid.\n"; return; }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    for (int i = 0; i < hospitalPatientCount; i++)
    {
        if (hospitalPatients[i].patient_id == pid)
        {
            for (int b = 0; b < hospitalBedCount; b++)
            {
                if (hospitalBeds[b].patientID == pid)
                {
                    hospitalBeds[b].patientID = -1;
                    hospitalBeds[b].occupied = false;
                    break;
                }
            }
            hospitalPatients[i].status = 3;
            hospital_patient_hash_remove(pid);
            hospital_patient_hash_insert(hospitalPatients[i]);
            cout << "Patient " << pid << " discharged.\n";
            return;
        }
    }
    cout << "Patient not found.\n";
}

// Add staff
void hospitalAddStaffInteractive()
{
    if (hospitalStaffCount >= HOSPITAL_MAX_STAFF) { cout << "Overflow: staff capacity reached!\n"; return; }
    HospitalStaff s;
    s.id = hospital_createStaffID();
    cout << "Enter name: ";
    string tmp; getline(cin, tmp);
    strncpy(s.name, tmp.c_str(), sizeof(s.name)-1); s.name[sizeof(s.name)-1]=0;
    cout << "Enter role: "; getline(cin, tmp);
    strncpy(s.role, tmp.c_str(), sizeof(s.role)-1); s.role[sizeof(s.role)-1]=0;
    cout << "Enter department: "; getline(cin, tmp);
    strncpy(s.department, tmp.c_str(), sizeof(s.department)-1); s.department[sizeof(s.department)-1]=0;
    cout << "Enter shift (0 none,1 morning,2 evening,3 night): "; getline(cin, tmp);
    s.shift = toInt(tmp);
    cout << "Enter salary: "; getline(cin, tmp); s.salary = toInt(tmp);
    cout << "Enter contact: "; getline(cin, tmp); strncpy(s.contact, tmp.c_str(), sizeof(s.contact)-1);
    cout << "Enter specialty (if doctor): "; getline(cin, tmp); strncpy(s.specialty, tmp.c_str(), sizeof(s.specialty)-1);
    hospitalStaff[hospitalStaffCount++] = s;
    cout << "Staff added ID " << s.id << "\n";
}

void hospitalListStaff()
{
    if (hospitalStaffCount == 0) { cout << "No staff.\n"; return; }
    for (int i = 0; i < hospitalStaffCount; i++)
    {
        cout << hospitalStaff[i].id << " | " << hospitalStaff[i].name << " | " << hospitalStaff[i].role << " | Dept:" << hospitalStaff[i].department << " | Sal:" << hospitalStaff[i].salary << "\n";
    }
}

// Search patient by name substring (simple case-insensitive)
void hospitalSearchPatientByName()
{
    cout << "Enter name pattern: ";
    string pat;
    getline(cin, pat);
    if (pat.empty()) { cout << "Empty.\n"; return; }
    string patL = pat; for (size_t i = 0; i < patL.size(); ++i) patL[i] = tolower((unsigned char)patL[i]);
    bool found = false;
    for (int i = 0; i < hospitalPatientCount; i++)
    {
        string name = hospitalPatients[i].name;
        string nameL = name; for (size_t k = 0; k < nameL.size(); ++k) nameL[k] = tolower((unsigned char)nameL[k]);
        if (nameL.find(patL) != string::npos)
        {
            cout << "Found ID:" << hospitalPatients[i].patient_id << " | " << hospitalPatients[i].name << " | Age:" << hospitalPatients[i].age << " | Status:" << hospitalPatients[i].status << "\n";
            found = true;
        }
    }
    if (!found) cout << "No patients found.\n";
}

// Search by ID (hash)
void hospitalSearchPatientByIDInteractive()
{
    cout << "Enter patient ID: ";
    int pid;
    if (!(cin >> pid)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout<<"Invalid.\n"; return; }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    HospitalPatient p;
    if (!hospital_patient_hash_get(pid, p)) { cout << "Patient not found.\n"; return; }
    cout << "Patient: " << p.patient_id << " | " << p.name << " | Age:" << p.age << " | Gender:" << p.gender << " | Contact:" << p.contact << " | Status:" << p.status << "\n";
}

// ---------- Merged Booking: Appointment / Surgery / Lab ----------
void hospitalBookAppointmentInteractive()
{
    if (hospitalApptCount >= HOSPITAL_MAX_APPOINTS) { cout << "Overflow: appointments limit\n"; return; }
    HospitalAppointment a;
    a.apptID = hospital_createApptID();
    cout << "Select booking type: (1) General Checkup  (2) Surgery  (3) Lab Test\n";
    int t;
    string tmp;
    getline(cin, tmp);
    t = toInt(tmp);
    if (t < 1 || t > 3) { cout << "Invalid type.\n"; return; }
    a.type = t;
    cout << "Enter patient ID: "; getline(cin, tmp); a.patientID = toInt(tmp);
    cout << "Enter doctor ID: "; getline(cin, tmp); a.doctorID = toInt(tmp);
    cout << "Enter date (YYYY-MM-DD): "; getline(cin, tmp); strncpy(a.date, tmp.c_str(), sizeof(a.date)-1); a.date[sizeof(a.date)-1]=0;
    cout << "Enter time (HH:MM): "; getline(cin, tmp); strncpy(a.time, tmp.c_str(), sizeof(a.time)-1); a.time[sizeof(a.time)-1]=0;
    cout << "Enter duration minutes (typical): "; getline(cin, tmp); a.duration = toInt(tmp);
    a.status = 0;
    strncpy(a.remarks, "", sizeof(a.remarks)-1);
    // If surgery, collect optional surgery details
    if (a.type == 2)
    {
        cout << "Enter OT room ID (optional, -1 if none): "; getline(cin, tmp); a.OTroomID = toInt(tmp);
        cout << "Enter expected surgery duration (minutes): "; getline(cin, tmp); a.durationMins = toInt(tmp);
        cout << "Enter anesthesia type: "; getline(cin, tmp); strncpy(a.anesthesiaType, tmp.c_str(), sizeof(a.anesthesiaType)-1); a.anesthesiaType[sizeof(a.anesthesiaType)-1]=0;
    }
    else
    {
        a.OTroomID = -1; a.durationMins = 0; strncpy(a.anesthesiaType,"",sizeof(a.anesthesiaType)-1);
    }
    // If lab, collect test type (results are loaded from CSV only later)
    if (a.type == 3)
    {
        cout << "Enter test type: "; getline(cin, tmp); strncpy(a.testType, tmp.c_str(), sizeof(a.testType)-1); a.testType[sizeof(a.testType)-1]=0;
        strncpy(a.resultDate, "", sizeof(a.resultDate)-1);
        strncpy(a.resultSummary, "", sizeof(a.resultSummary)-1);
    }
    else
    {
        strncpy(a.testType,"",sizeof(a.testType)-1);
        strncpy(a.resultDate,"",sizeof(a.resultDate)-1);
        strncpy(a.resultSummary,"",sizeof(a.resultSummary)-1);
    }
    hospitalAppts[hospitalApptCount++] = a;
    cout << "Appointment booked ID " << a.apptID << " (type " << a.type << ")\n";
}

// List appointments
void hospitalListAppointments()
{
    if (hospitalApptCount == 0) { cout << "No appointments.\n"; return; }
    cout << "Appointments:\n";
    for (int i = 0; i < hospitalApptCount; ++i)
    {
        HospitalAppointment &a = hospitalAppts[i];
        string tstr = (a.type==1?"General":(a.type==2?"Surgery":"Lab"));
        cout << a.apptID << " | " << tstr << " | Patient:" << a.patientID << " | Doctor:" << a.doctorID << " | " << a.date << " " << a.time << " | Dur:" << a.duration << " | Status:" << a.status;
        if (a.type == 2)
            cout << " | OT:" << a.OTroomID << " | SurgDur:" << a.durationMins << " | Anesth:" << a.anesthesiaType;
        if (a.type == 3)
            cout << " | Test:" << a.testType << " | ResultDate:" << a.resultDate << " | ResultSummary:" << a.resultSummary;
        if (strlen(a.remarks) > 0) cout << " | Remarks:" << a.remarks;
        cout << "\n";
    }
}

// ---------- Module menu ----------
void hospitalShowMenu()
{
    cout << "\n====================================\n";
    cout << "           HOSPITAL MANAGEMENT\n";
    cout << "====================================\n";
    cout << " 1. Add Patient (ER/OPD)\n";
    cout << " 2. Search Patient by ID\n";
    cout << " 3. Search Patient by Name\n";
    cout << " 4. Admit Patient (assign bed)\n";
    cout << " 5. Discharge Patient\n";
    cout << " 6. Add Staff\n";
    cout << " 7. List Staff\n";
    cout << " 8. Load patients CSV (patients.csv)\n";
    cout << " 9. Load staff CSV (hospital_staff.csv)\n";
    cout << "10. Load rooms CSV (hospital_rooms.csv)\n";
    cout << "11. Book Appointment / Surgery / Lab Test\n";
    cout << "12. List Appointments\n";
    cout << "13. Load appointments CSV (appointments.csv)\n";
    cout << "14. Load labs CSV (labs.csv)  (updates lab results only)\n";
    cout << " 0. Return to MAIN MENU\n";
    cout << "====================================\n";
    cout << "Enter choice: ";
}

void hospitalInitModule()
{
    hospital_init_patient_hash();
    // other arrays are default-initialized by static storage
}

// hospitalSystem (entry)
void hospitalSystem()
{
    hospitalInitModule();
    int choice;
    while (true)
    {
        hospitalShowMenu();
        if (!(cin >> choice))
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input.\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        switch (choice)
        {
        case 1: hospitalAddPatient(); break;
        case 2: hospitalSearchPatientByIDInteractive(); break;
        case 3: hospitalSearchPatientByName(); break;
        case 4: hospitalAdmitPatient(); break;
        case 5: hospitalDischargePatient(); break;
        case 6: hospitalAddStaffInteractive(); break;
        case 7: hospitalListStaff(); break;
        case 8: hospitalLoadPatientsCSV("patients.csv"); break;
        case 9: hospitalLoadStaffCSV("hospital_staff.csv"); break;
        case 10: hospitalLoadRoomsCSV("hospital_rooms.csv"); break;
        case 11: hospitalBookAppointmentInteractive(); break;
        case 12: hospitalListAppointments(); break;
        case 13: hospitalLoadAppointmentsCSV("appointments.csv"); break;
        case 14: hospitalLoadLabsCSV("labs.csv"); break;
        case 0: cout << "Returning to main menu...\n"; return;
        default: cout << "Invalid choice.\n";
        }
    }
}
int main()
{
    hospitalSystem();
}
