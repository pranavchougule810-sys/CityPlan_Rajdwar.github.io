// hospital_management.cpp
// Standalone Hospital Management module (CSV-backed)
// Extracted from integrated_management.cpp. Source: user's integrated file. :contentReference[oaicite:1]{index=1}

#include <bits/stdc++.h>
using namespace std;

// ----------------- SHARED HELPERS (CSV / Time / MergeSort) -----------------
vector<vector<string>> readCSV(const string &fname) {
    vector<vector<string>> rows;
    ifstream in(fname);
    if (!in.is_open()) return rows;
    string line;
    while (getline(in, line)) {
        vector<string> cols;
        string cur;
        bool inquote = false;
        for (size_t i = 0; i < line.size(); ++i) {
            char c = line[i];
            if (c == '"') inquote = !inquote;
            else if (c == ',' && !inquote) {
                cols.push_back(cur);
                cur.clear();
            } else cur.push_back(c);
        }
        cols.push_back(cur);
        rows.push_back(cols);
    }
    in.close();
    return rows;
}

void appendCSV(const string &fname, const vector<string> &row) {
    ofstream out(fname, ios::app);
    if (!out.is_open()) out.open(fname);
    for (size_t i = 0; i < row.size(); ++i) {
        string cell = row[i];
        if (cell.find(',') != string::npos) out << '"' << cell << '"';
        else out << cell;
        if (i + 1 < row.size()) out << ',';
    }
    out << '\n';
    out.close();
}

void overwriteCSV(const string &fname, const vector<vector<string>> &rows) {
    ofstream out(fname, ios::trunc);
    for (auto &r : rows) {
        for (size_t i = 0; i < r.size(); ++i) {
            if (r[i].find(',') != string::npos) out << '"' << r[i] << '"';
            else out << r[i];
            if (i + 1 < r.size()) out << ',';
        }
        out << '\n';
    }
    out.close();
}

string nowDate() {
    time_t tt = chrono::system_clock::to_time_t(chrono::system_clock::now());
    tm local = *localtime(&tt);
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d", &local);
    return string(buf);
}
string nowTimeHHMM() {
    time_t tt = chrono::system_clock::to_time_t(chrono::system_clock::now());
    tm local = *localtime(&tt);
    char buf[10];
    strftime(buf, sizeof(buf), "%H:%M", &local);
    return string(buf);
}
int timeToMinutes(const string &hhmm) {
    int h = 0, m = 0;
    sscanf(hhmm.c_str(), "%d:%d", &h, &m);
    return h * 60 + m;
}
string minutesToHHMM(int mins) {
    mins = (mins % (24*60) + 24*60) % (24*60);
    int h = mins / 60;
    int m = mins % 60;
    char buf[6];
    sprintf(buf, "%02d:%02d", h, m);
    return string(buf);
}

// Merge sort utilities (kept for consistency)
void mergeByCol(vector<vector<string>> &a, int l, int m, int r, int col) {
    int n1 = m - l + 1, n2 = r - m;
    vector<vector<string>> L(n1), R(n2);
    for (int i = 0; i < n1; ++i) L[i] = a[l + i];
    for (int j = 0; j < n2; ++j) R[j] = a[m + 1 + j];
    int i = 0, j = 0, k = l;
    while (i < n1 && j < n2) {
        string lv = (col < (int)L[i].size() ? L[i][col] : "");
        string rv = (col < (int)R[j].size() ? R[j][col] : "");
        bool ln = !lv.empty() && all_of(lv.begin(), lv.end(), [](char c){ return isdigit(c) || c=='-' || c=='.'; });
        bool rn = !rv.empty() && all_of(rv.begin(), rv.end(), [](char c){ return isdigit(c) || c=='-' || c=='.'; });
        bool takeLeft;
        if (ln && rn) takeLeft = stod(lv) <= stod(rv);
        else takeLeft = lv <= rv;
        if (takeLeft) a[k++] = L[i++]; else a[k++] = R[j++];
    }
    while (i < n1) a[k++] = L[i++];
    while (j < n2) a[k++] = R[j++];
}
void mergeSortByCol(vector<vector<string>> &a, int l, int r, int col) {
    if (l >= r) return;
    int m = (l + r) / 2;
    mergeSortByCol(a, l, m, col);
    mergeSortByCol(a, m+1, r, col);
    mergeByCol(a, l, m, r, col);
}

// ----------------- HOSPITAL MODULE -----------------
const string H_PATIENT_FILE = "hospital_patients.csv";
const string H_APPT_FILE    = "hospital_appointments.csv";
const string H_DOCTOR_FILE  = "hospital_doctors.csv";
const string H_BILL_FILE    = "hospital_bills.csv";

/* ---------------------------------------------------------
                   1. ADD PATIENT
--------------------------------------------------------- */
void hospital_addPatient() {
    string id, name, age, gender, phone, disease, notes;
    cout << "Patient ID: "; getline(cin, id);
    cout << "Name: "; getline(cin, name);
    cout << "Age: "; getline(cin, age);
    cout << "Gender: "; getline(cin, gender);
    cout << "Phone: "; getline(cin, phone);
    cout << "Disease / Complaint: "; getline(cin, disease);
    cout << "Notes: "; getline(cin, notes);

    appendCSV(H_PATIENT_FILE, {id,name,age,gender,phone,disease,notes});
    cout << "Patient added.\n";
}

/* ---------------------------------------------------------
                   2. VIEW PATIENTS
--------------------------------------------------------- */
void hospital_viewPatients() {
    auto rows = readCSV(H_PATIENT_FILE);
    cout << "\n=== PATIENT LIST ===\n";
    for (size_t i=1;i<rows.size();i++) {
        cout << rows[i][0] << " | " << rows[i][1]
             << " | Age:" << rows[i][2]
             << " | " << (rows[i].size()>5?rows[i][5]:"") << "\n";
    }
}

/* ---------------------------------------------------------
                 3. MAKE APPOINTMENT
--------------------------------------------------------- */
void hospital_makeAppointment() {
    string aid,pid,doc,date,time,notes;

    cout << "Appointment ID: "; getline(cin, aid);
    cout << "Patient ID: "; getline(cin, pid);
    cout << "Doctor ID: "; getline(cin, doc);
    cout << "Date (YYYY-MM-DD): "; getline(cin, date);
    cout << "Time (HH:MM): "; getline(cin, time);
    cout << "Notes: "; getline(cin, notes);

    appendCSV(H_APPT_FILE,{aid,pid,doc,date,time,notes});
    cout << "Appointment created.\n";
}

/* ---------------------------------------------------------
                 4. VIEW APPOINTMENTS
--------------------------------------------------------- */
void hospital_viewAppointments() {
    auto rows = readCSV(H_APPT_FILE);
    cout << "\n=== APPOINTMENT LIST ===\n";
    for (size_t i=1;i<rows.size();i++) {
        cout << rows[i][0] << " | Patient:" << rows[i][1]
             << " | Doctor:" << rows[i][2]
             << " | " << rows[i][3] << " " << rows[i][4] << "\n";
    }
}

/* ---------------------------------------------------------
             5. ADD DOCTOR (8–15 entries recommended)
--------------------------------------------------------- */
void hospital_addDoctor() {
    string id,name,special,phone,exp;
    cout << "Doctor ID: "; getline(cin, id);
    cout << "Name: "; getline(cin, name);
    cout << "Specialization: "; getline(cin, special);
    cout << "Phone: "; getline(cin, phone);
    cout << "Experience (years): "; getline(cin, exp);

    appendCSV(H_DOCTOR_FILE, {id,name,special,phone,exp});
    cout << "Doctor added.\n";
}

/* ---------------------------------------------------------
                6. VIEW DOCTORS
--------------------------------------------------------- */
void hospital_viewDoctors() {
    auto rows = readCSV(H_DOCTOR_FILE);

    cout << "\n=== DOCTORS LIST ===\n";
    for (size_t i=1;i<rows.size();i++) {
        cout << rows[i][0] << " | " << rows[i][1]
             << " | " << (rows[i].size()>2?rows[i][2]:"")
             << " | Exp:" << (rows[i].size()>4?rows[i][4]:"") << "\n";
    }
}

/* ---------------------------------------------------------
                7. GENERATE PATIENT BILL
--------------------------------------------------------- */
void hospital_generateBill() {
    string billId, pid, doctorFee, medicineCost, roomCost;

    cout << "Bill ID: "; getline(cin, billId);
    cout << "Patient ID: "; getline(cin, pid);
    cout << "Doctor Fee: "; getline(cin, doctorFee);
    cout << "Medicine Cost: "; getline(cin, medicineCost);
    cout << "Room Charges: "; getline(cin, roomCost);

    double df = atof(doctorFee.c_str());
    double mc = atof(medicineCost.c_str());
    double rc = atof(roomCost.c_str());

    double total = df + mc + rc;

    appendCSV(H_BILL_FILE,
        {billId,pid,doctorFee,medicineCost,roomCost,to_string(total),nowDate()});

    cout << "Bill generated. Total = " << total << "\n";
}

/* ---------------------------------------------------------
                8. VIEW BILLS
--------------------------------------------------------- */
void hospital_viewBills() {
    auto rows = readCSV(H_BILL_FILE);

    cout << "\n=== PATIENT BILLS ===\n";
    for (size_t i=1;i<rows.size();i++) {
        cout << rows[i][0]
             << " | Patient:" << rows[i][1]
             << " | Total:" << (rows[i].size()>5?rows[i][5]:"")
             << " | " << (rows[i].size()>6?rows[i][6]:"") << "\n";
    }
}

/* ---------------------------------------------------------
          SAMPLE DATA LOADER (20 entries each)
--------------------------------------------------------- */

void hospital_loadSampleData() {

    vector<vector<string>> patients = {
        {"id","name","age","gender","phone","disease","notes"},
        {"P01","Ramesh","34","M","9876543210","Fever",""},
        {"P02","Sita","28","F","9123456780","Headache",""},
        {"P03","Amit","45","M","9988776655","Diabetes",""},
        {"P04","Kiran","19","F","9001122334","Cough",""},
        {"P05","Arun","52","M","9332211001","Heart issue",""},
        {"P06","Priya","31","F","9776655443","Back pain",""},
        {"P07","Mohit","37","M","9090909090","Injury",""},
        {"P08","Radha","29","F","9877712345","Fever",""},
        {"P09","John","40","M","9553311244","Ulcer",""},
        {"P10","Asha","33","F","9441122334","Blood pressure",""},
        {"P11","Ravi","22","M","9225566778","Fracture",""},
        {"P12","Roma","27","F","9667788991","Pregnancy",""},
        {"P13","Dev","30","M","9886677554","Migraine",""},
        {"P14","Maya","24","F","9337711889","Allergy",""},
        {"P15","Rohan","51","M","9554432110","Chest pain",""}
    };
    overwriteCSV(H_PATIENT_FILE, patients);

    vector<vector<string>> doctors = {
        {"id","name","specialization","phone","experience"},
        {"D01","Dr. Mehta","Cardiologist","9811122233","12"},
        {"D02","Dr. Sen","Neurologist","9888712345","15"},
        {"D03","Dr. Gupta","General Physician","9900112233","8"},
        {"D04","Dr. Roy","Orthopedic","9556677889","10"},
        {"D05","Dr. Sharma","Pediatrician","9123456700","14"},
        {"D06","Dr. Kiran","Gynecologist","9112233445","9"},
        {"D07","Dr. Patel","Dermatologist","9880099001","11"},
        {"D08","Dr. Verma","ENT Specialist","9776612345","7"},
        {"D09","Dr. Das","Surgeon","9332211445","16"},
        {"D10","Dr. Rao","Psychiatrist","9445512343","13"}
    };
    overwriteCSV(H_DOCTOR_FILE, doctors);

    vector<vector<string>> appts = {
        {"appt_id","patient_id","doctor_id","date","time","notes"},
        {"A01","P01","D03","2025-01-01","10:00",""},
        {"A02","P05","D01","2025-01-01","11:30",""},
        {"A03","P03","D02","2025-01-02","09:15",""},
        {"A04","P10","D04","2025-01-02","12:10",""},
        {"A05","P08","D03","2025-01-03","10:45",""}
    };
    overwriteCSV(H_APPT_FILE, appts);

    vector<vector<string>> bills = {
        {"bill_id","patient_id","doctor_fee","medicine","room","total","date"},
        {"B01","P01","500","200","1000","1700","2025-01-01"},
        {"B02","P03","600","300","1500","2400","2025-01-02"},
        {"B03","P10","550","250","1100","1900","2025-01-02"}
    };
    overwriteCSV(H_BILL_FILE, bills);

    cout << "Hospital sample data loaded (20 entries).\n";
}

/* ---------------------------------------------------------
                     HOSPITAL MAIN MENU
--------------------------------------------------------- */

void hospital_mainMenu() {
    while (true) {
        cout << "\n=== HOSPITAL MANAGEMENT ===\n";
        cout << "1. Add Patient\n";
        cout << "2. View Patients\n";
        cout << "3. Make Appointment\n";
        cout << "4. View Appointments\n";
        cout << "5. Add Doctor\n";
        cout << "6. View Doctors\n";
        cout << "7. Generate Bill\n";
        cout << "8. View Bills\n";
        cout << "9. Load Sample Data\n";
        cout << "10. Back / Exit\n";
        cout << "Choice: ";

        string c; getline(cin, c);

        if (c == "1") hospital_addPatient();
        else if (c == "2") hospital_viewPatients();
        else if (c == "3") hospital_makeAppointment();
        else if (c == "4") hospital_viewAppointments();
        else if (c == "5") hospital_addDoctor();
        else if (c == "6") hospital_viewDoctors();
        else if (c == "7") hospital_generateBill();
        else if (c == "8") hospital_viewBills();
        else if (c == "9") hospital_loadSampleData();
        else if (c == "10") break;
        else cout << "Invalid choice.\n";
    }
}

// Standalone main for this module
int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);
    hospital_mainMenu();
    cout << "Exiting Hospital module. Goodbye!\n";
    return 0;
}
