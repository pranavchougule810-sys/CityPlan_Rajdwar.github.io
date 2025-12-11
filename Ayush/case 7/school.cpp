// school_management.cpp
// Standalone School / College Management module (CSV-backed)
// Extracted-style standalone module (helpers included so it runs independently)

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

// small utility to safely get column string
string col(const vector<string> &r, size_t idx) {
    if (idx < r.size()) return r[idx];
    return "";
}

// ----------------- SCHOOL MODULE FILE NAMES -----------------
const string S_STUDENT_FILE = "school_students.csv";
const string S_TEACHER_FILE = "school_teachers.csv";
const string S_CLASS_FILE   = "school_classes.csv";
const string S_ATTEND_FILE  = "school_attendance.csv";
const string S_FEE_FILE     = "school_fees.csv";
const string S_GRADE_FILE   = "school_grades.csv";

// ----------------- STUDENT MANAGEMENT -----------------
void school_addStudent() {
    string id,name,age,gender,grade,phone,address;
    cout << "Student ID: "; getline(cin,id);
    cout << "Name: "; getline(cin,name);
    cout << "Age: "; getline(cin,age);
    cout << "Gender: "; getline(cin,gender);
    cout << "Class/Grade (e.g. 10A): "; getline(cin,grade);
    cout << "Phone: "; getline(cin,phone);
    cout << "Address: "; getline(cin,address);

    appendCSV(S_STUDENT_FILE, {id,name,age,gender,grade,phone,address});
    cout << "Student added.\n";
}

void school_viewStudents() {
    auto rows = readCSV(S_STUDENT_FILE);
    cout << "\n=== STUDENTS ===\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        cout << col(rows[i],0) << " | " << col(rows[i],1)
             << " | Age:" << col(rows[i],2)
             << " | Class:" << col(rows[i],4) << "\n";
    }
}

// Search student by name substring
void school_findStudent() {
    cout << "Enter name or part of name to search: ";
    string term; getline(cin, term);
    string low = term; transform(low.begin(), low.end(), low.begin(), ::tolower);

    auto rows = readCSV(S_STUDENT_FILE);
    bool found = false;
    for (size_t i = 1; i < rows.size(); ++i) {
        string nm = col(rows[i],1); transform(nm.begin(), nm.end(), nm.begin(), ::tolower);
        if (nm.find(low) != string::npos) {
            cout << "Found: " << col(rows[i],0) << " | " << col(rows[i],1) << " | Class:" << col(rows[i],4) << "\n";
            found = true;
        }
    }
    if (!found) cout << "No student found.\n";
}

// ----------------- TEACHER MANAGEMENT -----------------
void school_addTeacher() {
    string id,name,subject,phone,exp;
    cout << "Teacher ID: "; getline(cin,id);
    cout << "Name: "; getline(cin,name);
    cout << "Subject: "; getline(cin,subject);
    cout << "Phone: "; getline(cin,phone);
    cout << "Experience (years): "; getline(cin,exp);

    appendCSV(S_TEACHER_FILE, {id,name,subject,phone,exp});
    cout << "Teacher added.\n";
}

void school_viewTeachers() {
    auto rows = readCSV(S_TEACHER_FILE);
    cout << "\n=== TEACHERS ===\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        cout << col(rows[i],0) << " | " << col(rows[i],1) << " | " << col(rows[i],2)
             << " | Exp:" << col(rows[i],4) << "\n";
    }
}

// ----------------- CLASS & TIMETABLE -----------------
void school_addClass() {
    string cid, name, teacherId, room, notes;
    cout << "Class ID: "; getline(cin,cid);
    cout << "Class Name (e.g. 10A): "; getline(cin,name);
    cout << "Class Teacher ID: "; getline(cin,teacherId);
    cout << "Room No: "; getline(cin,room);
    cout << "Notes: "; getline(cin,notes);

    appendCSV(S_CLASS_FILE, {cid,name,teacherId,room,notes});
    cout << "Class added.\n";
}

void school_viewClasses() {
    auto rows = readCSV(S_CLASS_FILE);
    cout << "\n=== CLASSES ===\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        cout << col(rows[i],0) << " | " << col(rows[i],1) << " | Teacher:" << col(rows[i],2)
             << " | Room:" << col(rows[i],3) << "\n";
    }
}

// ----------------- ATTENDANCE -----------------
// attendance CSV columns: date, student_id, status (P/A)
void school_markAttendance() {
    cout << "Enter date (YYYY-MM-DD) or leave blank for today: ";
    string date; getline(cin,date);
    if (date.empty()) date = nowDate();

    cout << "Enter Student ID: ";
    string sid; getline(cin,sid);
    cout << "Status (P/A): ";
    string st; getline(cin,st);

    appendCSV(S_ATTEND_FILE, {date,sid,st});
    cout << "Attendance recorded.\n";
}

void school_viewAttendanceForDate() {
    cout << "Enter date (YYYY-MM-DD): ";
    string date; getline(cin,date);
    auto rows = readCSV(S_ATTEND_FILE);
    cout << "\nAttendance on " << date << ":\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        if (col(rows[i],0) == date) {
            cout << col(rows[i],1) << " | " << col(rows[i],2) << "\n";
        }
    }
}

void school_attendanceSummary() {
    cout << "Enter Student ID for summary: ";
    string sid; getline(cin,sid);
    auto rows = readCSV(S_ATTEND_FILE);
    int present = 0, total = 0;
    for (size_t i = 1; i < rows.size(); ++i) {
        if (col(rows[i],1) == sid) {
            total++;
            if (col(rows[i],2) == "P" || col(rows[i],2) == "p") present++;
        }
    }
    if (total == 0) cout << "No attendance records for " << sid << ".\n";
    else cout << "Present " << present << "/" << total << " (" << (present*100.0/total) << "%)\n";
}

// ----------------- FEES -----------------
// fees CSV columns: fee_id, student_id, amount, date, status
void school_recordFeePayment() {
    string fid, sid, amount, date, status;
    cout << "Fee Payment ID: "; getline(cin,fid);
    cout << "Student ID: "; getline(cin,sid);
    cout << "Amount: "; getline(cin,amount);
    cout << "Date (YYYY-MM-DD) or blank for today: "; getline(cin,date);
    if (date.empty()) date = nowDate();
    cout << "Status (Paid/Unpaid): "; getline(cin,status);

    appendCSV(S_FEE_FILE, {fid,sid,amount,date,status});
    cout << "Fee recorded.\n";
}

void school_viewFees() {
    auto rows = readCSV(S_FEE_FILE);
    cout << "\n=== FEE RECORDS ===\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        cout << col(rows[i],0) << " | Student:" << col(rows[i],1)
             << " | Amt:" << col(rows[i],2) << " | " << col(rows[i],4) << "\n";
    }
}

void school_totalFeesCollected() {
    cout << "Enter date (YYYY-MM-DD) or blank for all time: ";
    string date; getline(cin,date);
    auto rows = readCSV(S_FEE_FILE);
    double sum = 0;
    for (size_t i = 1; i < rows.size(); ++i) {
        if (!date.empty() && col(rows[i],3) != date) continue;
        if (col(rows[i],4) == "Paid" || col(rows[i],4) == "paid")
            sum += atof(col(rows[i],2).c_str());
    }
    if (date.empty()) cout << "Total collected (all time) = " << sum << "\n";
    else cout << "Total collected on " << date << " = " << sum << "\n";
}

// ----------------- GRADES -----------------
// grades CSV: grade_id, student_id, subject, term, marks
void school_recordGrade() {
    string gid,sid,subject,term,marks;
    cout << "Grade ID: "; getline(cin,gid);
    cout << "Student ID: "; getline(cin,sid);
    cout << "Subject: "; getline(cin,subject);
    cout << "Term (e.g. Midterm, Final): "; getline(cin,term);
    cout << "Marks: "; getline(cin,marks);

    appendCSV(S_GRADE_FILE, {gid,sid,subject,term,marks});
    cout << "Grade recorded.\n";
}

void school_viewGradesForStudent() {
    cout << "Enter Student ID: ";
    string sid; getline(cin,sid);
    auto rows = readCSV(S_GRADE_FILE);
    cout << "\nGrades for " << sid << ":\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        if (col(rows[i],1) == sid) {
            cout << col(rows[i],2) << " | " << col(rows[i],3) << " | " << col(rows[i],4) << "\n";
        }
    }
}

// ----------------- SAMPLE DATA LOADER -----------------
void school_loadSampleData() {
    vector<vector<string>> students = {
        {"id","name","age","gender","class","phone","address"},
        {"ST01","Ramesh","15","M","10A","9876543210","Near Park"},
        {"ST02","Sita","14","F","9B","9123456780","Sector 5"},
        {"ST03","Amit","16","M","11C","9988776655","Hill View"},
        {"ST04","Priya","15","F","10A","9001122334","Green Lane"},
        {"ST05","Karan","13","M","8A","9332211001","River Road"}
    };
    overwriteCSV(S_STUDENT_FILE, students);

    vector<vector<string>> teachers = {
        {"id","name","subject","phone","experience"},
        {"T01","Mr. Sharma","Mathematics","9811122233","8"},
        {"T02","Ms. Iyer","English","9888712345","6"},
        {"T03","Mr. Khan","Science","9900112233","10"}
    };
    overwriteCSV(S_TEACHER_FILE, teachers);

    vector<vector<string>> classes = {
        {"id","name","teacher_id","room","notes"},
        {"C10A","10A","T01","101","Section A of class 10"},
        {"C09B","9B","T02","102","Section B of class 9"},
        {"C11C","11C","T03","103","Section C of class 11"}
    };
    overwriteCSV(S_CLASS_FILE, classes);

    vector<vector<string>> attendance = {
        {"date","student_id","status"},
        {nowDate(),"ST01","P"},
        {nowDate(),"ST02","A"},
        {nowDate(),"ST03","P"}
    };
    overwriteCSV(S_ATTEND_FILE, attendance);

    vector<vector<string>> fees = {
        {"fee_id","student_id","amount","date","status"},
        {"F01","ST01","5000","2025-01-10","Paid"},
        {"F02","ST02","5000","2025-01-10","Unpaid"}
    };
    overwriteCSV(S_FEE_FILE, fees);

    vector<vector<string>> grades = {
        {"grade_id","student_id","subject","term","marks"},
        {"G01","ST01","Math","Midterm","78"},
        {"G02","ST01","Science","Midterm","82"},
        {"G03","ST02","English","Midterm","69"}
    };
    overwriteCSV(S_GRADE_FILE, grades);

    cout << "School sample data loaded.\n";
}

// ----------------- MAIN MENU -----------------
void school_mainMenu() {
    while (true) {
        cout << "\n=== SCHOOL / COLLEGE MANAGEMENT ===\n";
        cout << "1. Students\n";
        cout << "2. Teachers\n";
        cout << "3. Classes\n";
        cout << "4. Attendance\n";
        cout << "5. Fees\n";
        cout << "6. Grades\n";
        cout << "7. Load Sample Data\n";
        cout << "8. Back / Exit\n";
        cout << "Choice: ";

        string c; getline(cin,c);

        if (c == "1") {
            while (true) {
                cout << "\nStudents Menu\n1.Add Student\n2.View Students\n3.Search Student\n4.Back\nChoice: ";
                string s; getline(cin,s);
                if (s == "1") school_addStudent();
                else if (s == "2") school_viewStudents();
                else if (s == "3") school_findStudent();
                else break;
            }
        } else if (c == "2") {
            while (true) {
                cout << "\nTeachers Menu\n1.Add Teacher\n2.View Teachers\n3.Back\nChoice: ";
                string s; getline(cin,s);
                if (s == "1") school_addTeacher();
                else if (s == "2") school_viewTeachers();
                else break;
            }
        } else if (c == "3") {
            while (true) {
                cout << "\nClasses Menu\n1.Add Class\n2.View Classes\n3.Back\nChoice: ";
                string s; getline(cin,s);
                if (s == "1") school_addClass();
                else if (s == "2") school_viewClasses();
                else break;
            }
        } else if (c == "4") {
            while (true) {
                cout << "\nAttendance Menu\n1.Mark Attendance\n2.View Attendance by Date\n3.Attendance Summary (Student)\n4.Back\nChoice: ";
                string s; getline(cin,s);
                if (s == "1") school_markAttendance();
                else if (s == "2") school_viewAttendanceForDate();
                else if (s == "3") school_attendanceSummary();
                else break;
            }
        } else if (c == "5") {
            while (true) {
                cout << "\nFees Menu\n1.Record Payment\n2.View Fees\n3.Total Collected\n4.Back\nChoice: ";
                string s; getline(cin,s);
                if (s == "1") school_recordFeePayment();
                else if (s == "2") school_viewFees();
                else if (s == "3") school_totalFeesCollected();
                else break;
            }
        } else if (c == "6") {
            while (true) {
                cout << "\nGrades Menu\n1.Record Grade\n2.View Grades (Student)\n3.Back\nChoice: ";
                string s; getline(cin,s);
                if (s == "1") school_recordGrade();
                else if (s == "2") school_viewGradesForStudent();
                else break;
            }
        } else if (c == "7") school_loadSampleData();
        else if (c == "8") break;
        else cout << "Invalid choice.\n";
    }
}

// Standalone main for this module
int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);
    school_mainMenu();
    cout << "Exiting School module. Goodbye!\n";
    return 0;
}
