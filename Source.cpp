#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include<string>

using namespace std;

const int DAILY_HOURS = 8;
const int WEEKLY_HOURS = DAILY_HOURS * 5;
const int MONTHLY_HOURS = WEEKLY_HOURS * 4;

class Leave;
class System;
void (*mainMenuPtr)(System&);
class Attendance {
    string date;
    string start;
    string end;
    bool isRecorded;

public:
    Attendance() : date(""), start(""), end(""), isRecorded(true) {}

    Attendance(string d, string s, string e, bool isRecorded = true) : date(d), start(s), end(e), isRecorded(isRecorded) {}


    int computeHours() const {
        int startHour = stoi(start.substr(0, 2));
        int endHour = stoi(end.substr(0, 2));
        return endHour - startHour;
    }


    bool checkvalidity() {
        int totaltime = computeHours();
        return totaltime >= 8;
    }

    string getDate() const { return date; }
    string getStartTime() const { return start; }
    string getEndTime() const { return end; }
    string getStatus() const {
        return isRecorded ? "Present" : "Absent";
    }

    // Mark as absent
    void markAsAbsent() {
        isRecorded = false;
    }
};
class LeaveBalance {
    int casual, earned;
    std::vector<Leave*> leaves;

public:
    LeaveBalance() : casual(15), earned(21) {}
    void addLeave(Leave* leave) {
        leaves.push_back(leave);  // Add leave to the vector
    }
    std::vector<Leave*>& getLeaves() {
        return leaves;
    }

    bool deductCasual(const string& start, const string& end) {
        int days = calculateLeaveDays(start, end); // Helper function to calculate leave days
        if (casual >= days) {
            casual -= days;
            return true;
        }
        return false;
    }

    bool deductEarned(const string& start, const string& end) {
        int days = calculateLeaveDays(start, end); // Helper function to calculate leave days
        if (earned >= days) {
            earned -= days;
            return true;
        }
        return false;
    }

    int calculateLeaveDays(const string& start, const string& end) {
        int startDay = stoi(start.substr(8, 2)); // Extract day from YYYY-MM-DD
        int endDay = stoi(end.substr(8, 2));   // Extract day from YYYY-MM-DD
        return endDay - startDay + 1;          // Add 1 to include the start day
    }

    void viewBalance() const {
        cout << "Casual Leaves: " << casual << ", Earned Leaves: " << earned << endl;
    }
};


class Leave {
public:
    string type, startDate, endDate, reason, status;
    string leaveAddress, approvalDate;


public:
    Leave(string t, string start, string end, string r, string address = "", string s = "Pending")
        : type(t), startDate(start), endDate(end), reason(r), leaveAddress(address), status(s) {}

    string toString() const {
        return type + "," + startDate + "," + endDate + "," + reason + "," + status;
    }

    void approve() {
        status = "Approved";
        string approvalDateInput;
        cout << "enter todays date for the record:" << endl;
        cin >> approvalDateInput;
        approvalDate = approvalDateInput;
    }
    void disapprove() {
        status = "Disapproved";
        string disapprovalDateInput;
        cout << "enter todays date for the record:" << endl;
        cin >> disapprovalDateInput;
        approvalDate = disapprovalDateInput; // now that data member will store the date of the dissapproval
    }
    bool isApproved() const {
        if (status == "Approved")
            return true;
        else return false;
    }
    bool isDisapproved() const {
        if (status == "Disapproved")
            return true;
        else return false;
    }
    string getType() const { return type; }
    virtual bool applyForLeave() = 0; // Abstract method to be implemented by subclasses

    bool coversDate(const string& date) const {
        // Parse dates into integers (YYYYMMDD format) for comparison
        int leaveStart = stoi(startDate.substr(0, 4) + startDate.substr(5, 2) + startDate.substr(8, 2));
        int leaveEnd = stoi(endDate.substr(0, 4) + endDate.substr(5, 2) + endDate.substr(8, 2));
        int checkDate = stoi(date.substr(0, 4) + date.substr(5, 2) + date.substr(8, 2));

        return checkDate >= leaveStart && checkDate <= leaveEnd;
    }


};


// Casual Leave
class CasualLeave : public Leave {
    static const int maxLeavesPerYear = 15;
    static const int maxLeavesAtATime = 4;
    static int leavesTaken; // Tracks leaves taken in the current year

public:
    CasualLeave(string start, string end, string reason, string leaveAddress)
        : Leave("Casual Leave", start, end, reason, leaveAddress) {}

    bool applyForLeave() override {
        int days = stoi(endDate.substr(8, 2)) - stoi(startDate.substr(8, 2)) + 1; //total number of casual leaves calculated
        if (days > maxLeavesAtATime) {
            cout << "Cannot apply for more than " << maxLeavesAtATime << " casual leaves at a time.\n";
            return false;
        }
        if (leavesTaken + days > maxLeavesPerYear) {
            cout << "Insufficient casual leave balance.\n";
            return false;
        }

        leavesTaken += days;
        cout << "Casual leave applied successfully for " << days << " days.\n";
        status = "Approved";

        return true;
    }
};
int CasualLeave::leavesTaken = 0;

// Earned Leave
class EarnedLeave : public Leave {
    static const int maxLeavesPerYear = 21;
    static int leavesTaken;

public:
    EarnedLeave(string start, string end, string reason, string leaveAddress)
        : Leave("Earned Leave", start, end, reason, leaveAddress) {}

    bool applyForLeave() override {
        int days = stoi(endDate.substr(8, 2)) - stoi(startDate.substr(8, 2)) + 1;
        if (leavesTaken + days > maxLeavesPerYear) {
            cout << "Insufficient earned leave balance.\n";
            return false;
        }
        cout << "Earned leave applied successfully for " << days << " days. Awaiting approval.\n";
        leavesTaken += days;
        return true;
    }
};
int EarnedLeave::leavesTaken = 0;

// Official Leave
class OfficialLeave : public Leave {
public:
    OfficialLeave(string reason, string leaveAddress)
        : Leave("Official Leave", "", "", reason, leaveAddress) {}

    bool applyForLeave() override {
        cout << "Official leave applied successfully. Full credit awarded.\n";
        return true;
    }
};

// Unpaid Leave
class UnpaidLeave : public Leave {
public:
    UnpaidLeave(string start, string end, string reason, string leaveAddress)
        : Leave("Unpaid Leave", start, end, reason, leaveAddress) {}

    bool applyForLeave() override {
        cout << "Unpaid leave applied successfully. Awaiting approval.\n";
        return true;
    }
};

class Employee {
public:
    int empID;
    string name, phone, role, supervisorID;
    vector<Attendance> attendanceRecords;
    vector<Leave*> leaveApplications;
    LeaveBalance leaveBalance;
    int attendanceCount;
    int leaveCount;

public:
    Employee() : empID(0), name(""), phone(""), role(""), supervisorID(""), attendanceCount(0), leaveCount(0) {}
    Employee(int id, string n, string p, string r, string supID = "") : empID(id), name(n), phone(p), role(r), supervisorID(supID), attendanceCount(0), leaveCount(0) {}

    virtual void markAttendance(Employee& emp, const string& date, const string& start, const string& end) {
        cout << "Only guards can mark attendance.\n";
    }

    void viewAttendance() const {
        if (attendanceRecords.empty()) {
            cout << "No attendance records available for " << name << ".\n";
            return;
        }

        cout << "Attendance Records for " << name << ":\n";
        for (const auto& record : attendanceRecords) {
            cout << "Date: " << record.getDate()
                << ", Start Time: " << record.getStartTime()
                << ", End Time: " << record.getEndTime() << "\n";
        }
    }

    void calculateAttendance() {
        if (attendanceRecords.empty()) {
            cout << "No attendance records found.\n";
            return;
        }

        unordered_map<string, int> hoursPerWeek;
        unordered_map<string, int> hoursPerMonth;

        for (const auto& record : attendanceRecords) {
            string date = record.getDate();

            //per week 40 hours are required 
            string week = date.substr(0, 7) + "-W" + to_string(stoi(date.substr(8, 2)) / 7 + 1);
            string month = date.substr(0, 7);

            hoursPerWeek[week] += record.computeHours();
            hoursPerMonth[month] += record.computeHours();
        }
        //prompts 
        for (const auto& week : hoursPerWeek) {
            cout << "Week " << week.first << " - Hours Worked: " << week.second
                << (week.second < WEEKLY_HOURS ? " (Insufficient)" : "") << endl;
        }

        for (const auto& month : hoursPerMonth) {
            cout << "Month " << month.first << " - Hours Worked: " << month.second
                << (month.second < MONTHLY_HOURS ? " (Insufficient)" : "") << endl;
        }

        cout << "Attendance Percentage: " << calculateAttendancePercentage() << endl;
    }

    // calculate the attendance percentage
    float calculateAttendancePercentage() const {
        int totalHours = 0;
        for (const auto& record : attendanceRecords) {
            totalHours += record.computeHours();
        }
        if (MONTHLY_HOURS == 0) {
            cout << "Error: MONTHLY_HOURS is zero. Cannot calculate attendance percentage.\n";
            return 0.0f;
        }
        return (static_cast<float>(totalHours) / MONTHLY_HOURS) * 100;
    }

    vector<string> getAbsentDays() const {
        vector<string> absentDays; // Vector to store dates of absence

        cout << "Absent Days for " << name << ":\n";

        for (const auto& record : attendanceRecords) {
            if (record.getStatus() == "Absent") {
                absentDays.push_back(record.getDate()); // Add absent date to vector
                cout << record.getDate() << endl;      // Print the date
            }
        }

        return absentDays; // Return the vector of absent days
    }

    int getID() const { return empID; }
    string getName() const { return name; }


    int getAttendanceCount() const { return attendanceCount; }
    size_t getLeaveCount() const { return leaveApplications.size(); }


    void applyForLeave(Leave* leave) {
        leave->applyForLeave();
        leaveApplications.push_back(leave);
        cout << "Leave added for " << name << ": " << leave->startDate << " - " << leave->endDate << endl;
    }

    void viewPendingLeaves() const {
        cout << "Pending Leave Applications for " << name << ":\n";

        // Debugging the size of the vector
        cout << "Number of leave applications: " << leaveApplications.size() << endl;

        if (leaveApplications.empty()) {
            cout << "No leave applications found.\n";
            return;
        }

        // Iterate through the vector
        for (const auto& leave : leaveApplications) {
            if (leave->isApproved() == false && leave->isDisapproved() == false) {
                cout << leave->startDate << " - " << leave->endDate << " | TYPE: "
                    << leave->getType() << " | Status: "
                    << "Pending" << endl;
            }
        }
    }


    void viewLeaves() const {
        cout << "Leave Applications for " << name << ":\n";

        // Debugging the size of the vector
        cout << "Number of leave applications: " << leaveApplications.size() << endl;

        if (leaveApplications.empty()) {
            cout << "No leave applications found.\n";
            return;
        }

        // Iterate through the vector
        for (const auto& leave : leaveApplications) {
            cout << leave->startDate << " - " << leave->endDate << " | TYPE: "
                << leave->getType() << " | Status: ";
            if (leave->isApproved() == true) cout << "Approved" << endl;
            else if (leave->isDisapproved() == true) cout << "Disapproved" << endl;
            else cout << "Pending" << endl;
            /* << (leave->isApproved() ? "Approved" : leave->isDisapproved() ? "Disapproved" : "Pending") << endl;*/
        }
    }

    LeaveBalance& getLeaveBalance() { return leaveBalance; }


    string toString() const {
        stringstream ss;
        ss << empID << "," << name << "," << role << "," << supervisorID << ",";
        for (const auto& leave : leaveApplications) {
            ss << leave->toString() << ";";
        }
        return ss.str();
    }
    void addLeave(Leave* leave) {
        if (leave != nullptr) {
            leaveApplications.push_back(leave);  // Add the leave to the vector
            //cout << "Leave added for " << name << ": " << leave->startDate << " - " << leave->endDate << endl;
        }
    }

};
class Guard : public Employee {
public:
    Guard() {}
    Guard(int id, string n, string p, string r, string supID) : Employee(id, n, p, r, supID = "") {}
    void markAttendance(Employee& employee, const string& date, const string& start, const string& end) {
        // Update the employee's attendance records
        employee.attendanceRecords.emplace_back(date, start, end);
        cout << "Attendance marked for " << employee.getName() << " on " << date
            << " by Guard " << getName() << ".\n";

        // Log attendance to a file
        ofstream file("attendance.txt", ios::app);
        if (file.is_open()) {
            file << employee.getID() << "," << date << "," << start << "," << end << "\n";
            file.close();
        }
        else {
            cout << "Unable to open attendance file for logging.\n";
        }
    }
};


class Supervisor :public Employee {
public:
    void approveLeave(Leave& leave) {
        leave.approve();
    }
    void disapproveLeave(Leave& leave) {
        leave.disapprove();
    }

};

class Director : public Employee {
public:
    void approveLeave(Leave& leave) { leave.approve(); }
    void disapproveLeave(Leave& leave) { leave.disapprove(); }
};

class System {
public:
    Employee* arr[100];
    int employeeCount;
public:

    System() : employeeCount(0) {
        for (int i = 0; i < 100; ++i) {
            arr[i] = nullptr;  // Initialize all pointers to nullptr
        }
    }

    void loadAttendanceFromFile() {
        ifstream attendanceFile("attendance.txt");
        if (!attendanceFile.is_open()) {
            cout << "Attendance file not found or unable to open. Starting with empty records.\n";
            return;
        }

        string line;
        while (getline(attendanceFile, line)) {
            if (line.empty()) continue;

            stringstream ss(line);
            string empIDStr, date, start, end;
            getline(ss, empIDStr, ',');
            getline(ss, date, ',');
            getline(ss, start, ',');
            getline(ss, end, ',');

            int empID = stoi(empIDStr);
            Employee* emp = getEmployee(empID);

            if (emp) {
                emp->attendanceRecords.emplace_back(date, start, end);
            }
            else {
                cout << "Warning: Employee ID " << empID << " not found. Skipping attendance record.\n";
            }
        }

        attendanceFile.close();
        cout << "Attendance data loaded successfully.\n";
    }


    Employee* findEmployeeByID(int id) {
        for (auto& e : arr) {
            if (e->getID() == id) {
                return e;
            }
        }
        return nullptr;
    }

    void addEmployee(const Employee& e) {
        if (employeeCount >= 100) {  // Ensure we don't exceed array bounds
            cout << "Error: Employee list is full. Cannot add more employees.\n";
            return;
        }

        arr[employeeCount] = new Employee(e);  // Dynamically allocate memory for a new Employee
        employeeCount++;
    }


    void viewOutstandingLeaves() const {
        for (int i = 0; i < employeeCount; ++i) {
            if (arr[i] != nullptr) {  // Check for null pointers
                arr[i]->viewLeaves();
            }
        }
    }
    void viewPendinggLeaves() const {
        for (int i = 0; i < employeeCount; ++i) {
            if (arr[i] != nullptr) {  // Check for null pointers
                arr[i]->viewPendingLeaves();
            }
        }
    }


    Employee* getEmployee(int empID) {
        for (int i = 0; i < employeeCount; i++) {
            if (arr[i]->empID == empID) {
                return arr[i];
            }
        }
        return nullptr;
    }

    void saveDataToFile() const {
        ofstream file("employee.txt");
        if (!file.is_open()) {
            cout << "Error: Unable to open file for saving.\n";
            return;
        }

        for (int i = 0; i < employeeCount; ++i) {
            Employee* emp = arr[i];
            file << emp->empID << "," << emp->name << "," << emp->phone << "," << emp->role << "," << emp->supervisorID << ',';
            for (const Leave* leave : emp->leaveApplications) {
                file << leave->getType() << "," << leave->startDate << "," << leave->endDate << "," << leave->reason << "," << leave->status << "," << leave->leaveAddress << ";";
            }
            file << endl;  // Add a newline after all the leave applications for an employee
        }

        file.close();
        cout << "Data saved successfully.\n";
    }

    void saveAttendanceToFile() const {
        ofstream attendanceFile("attendance.txt", ios::trunc); // Overwrite attendance file
        if (!attendanceFile.is_open()) {
            cout << "Error: Unable to open attendance file for saving.\n";
            return;
        }

        for (int i = 0; i < employeeCount; ++i) {
            Employee* emp = arr[i];
            for (const auto& record : emp->attendanceRecords) {
                attendanceFile << emp->getID() << "," << record.getDate() << ","
                    << record.getStartTime() << "," << record.getEndTime() << "\n";
            }
        }

        attendanceFile.close();
        cout << "Attendance data saved successfully.\n";
    }

    void loadDataFromFile() {
        ifstream file("employee.txt");
        if (!file.is_open()) {
            cout << "Error: Unable to open file for loading.\n";
            return;
        }

        string line;
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string empIDStr;

            string name, role, phone, supID, leaveData;
            getline(ss, empIDStr, ',');
            getline(ss, name, ',');
            getline(ss, phone, ',');
            getline(ss, role, ',');
            getline(ss, supID, ',');
            if (empIDStr.empty() || name.empty()) {
                cout << "Skipping malformed employee entry: " << line << endl;
                continue;
            }
            int empID = stoi(empIDStr);
            Employee* emp = new Employee(empID, name, phone, role, supID);

            while (getline(ss, leaveData, ';')) {
                stringstream leaveStream(leaveData);
                string type, start, end, reason, status, leaveAddress;

                getline(leaveStream, type, ',');
                getline(leaveStream, start, ',');
                getline(leaveStream, end, ',');
                getline(leaveStream, reason, ',');
                getline(leaveStream, status, ',');
                getline(leaveStream, leaveAddress, ',');

                Leave* leave = nullptr;
                if (type == "Casual Leave") {
                    leave = new CasualLeave(start, end, reason, leaveAddress);
                    if (status == "Approved") {
                        emp->leaveBalance.deductCasual(start, end);
                    }
                }
                else if (type == "Earned Leave") {
                    leave = new EarnedLeave(start, end, reason, leaveAddress);
                    if (status == "Approved") {
                        emp->leaveBalance.deductEarned(start, end);
                    }
                }
                else if (type == "Official Leave") {
                    leave = new OfficialLeave(reason, leaveAddress);
                }
                else if (type == "Unpaid Leave") {
                    leave = new UnpaidLeave(start, end, reason, leaveAddress);
                }

                /* if (leave) {
                     leave->status = status;
                     emp->leaveApplications.push_back(leave);
                 }*/
                if (leave) {
                    leave->status = status;
                    emp->addLeave(leave);  // Use addLeave to populate leaveApplications
                }
            }

            if (role == "Guard") {
                arr[employeeCount++] = new Guard(empID, name, phone, role, supID);
            }
            else {
                arr[employeeCount++] = emp;
            }
        }

        file.close();
        cout << "Data loaded successfully.\n";
    }

    void generateAttendanceReport() {


        cout << "\n--- Attendance Report (Less than 80%) ---\n";
        for (const auto& employee : arr) {
            if (!employee) {
                continue;
            }

            float attendancePercentage = employee->calculateAttendancePercentage();
            if (attendancePercentage < 80.0) {
                cout << employee->getName() << " (" << employee->getID() << ") - Attendance: "
                    << attendancePercentage << "%\n";
            }
        }
    }
    void viewUnfiledCasualLeaves() {
        cout << "--- Unfiled Casual Leaves Report ---\n";
        for (int i = 0; i < employeeCount; ++i) {
            Employee* emp = arr[i];
            vector<string> absentDays = emp->getAbsentDays();
            vector<Leave*> leaves = emp->leaveApplications;

            for (const auto& day : absentDays) {
                bool isCovered = false;
                for (const auto& leave : leaves) {
                    if (leave->getType() == "Casual Leave" && leave->coversDate(day)) {
                        isCovered = true;
                        break;
                    }
                }

                if (!isCovered) {
                    cout << "Employee: " << emp->getName()
                        << " was absent on " << day
                        << " without filing a casual leave.\n";
                }
            }
        }
    }



};

void employeeMenu(Employee& emp, System& system) {
    int choice;
    do {
        cout << "\n--- Employee Menu ---\n";
        cout << "1. Apply for Leave\n";
        cout << "2. View Attendance\n";
        cout << "3. View Leave Applications\n";
        cout << "4. Check Attendance\n";
        cout << "5. View Leave Balance\n";
        cout << "6. Back to Main Menu\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
        case 1: {
            cout << "Enter leave type (1. Casual, 2. Earned, 3. Official, 4. Unpaid): ";
            int leaveType;
            cin >> leaveType;
            string start, end, reason, address;
            if (leaveType != 3) {
                cout << "Enter start date (YYYY-MM-DD): ";
                cin >> start;
                cout << "Enter end date (YYYY-MM-DD): ";
                cin >> end;
            }
            cout << "Enter reason for leave: ";
            cin.ignore();
            getline(cin, reason);
            cout << "enter you adress:";
            getline(cin, address);
            Leave* leave = nullptr;

            switch (leaveType) {
            case 1: leave = new CasualLeave(start, end, reason, address); break;
            case 2: leave = new EarnedLeave(start, end, reason, address); break;
            case 3: leave = new OfficialLeave(reason, address); break;
            case 4: leave = new UnpaidLeave(start, end, reason, address); break;
            default:
                cout << "Invalid leave type.\n";
                continue;
            }
            if (leave) {
                emp.applyForLeave(leave);  // Add the leave to the employee's list
                cout << "Leave application submitted.\n";

            }
            else {
                cout << "Failed to create leave application.\n";
                delete leave;
            }
            break;
        }
        case 2:
            emp.viewAttendance();
            break;
        case 3:
            emp.viewLeaves();
            break;
        case 4:
            emp.calculateAttendance();
            break;
        case 5:
            cout << "Leave Balance for " << emp.getName() << ":\n";
            emp.getLeaveBalance().viewBalance();
            break;
        case 6:
            cout << "Returning to main menu.\n";
            mainMenuPtr(system);
            break;
        default:
            cout << "Invalid choice. Try again.\n";
        }
    } while (choice != 6);
}
void supervisorMenu(System& system, int id) {
    int choice;
    do {
        cout << "\n--- Supervisor Menu ---\n";
        cout << "1. Approve Leave\n";
        cout << "2. Outstanding Casual Leaves\n";
        cout << "3. View Pending Leaves\n";
        cout << "4. View All Leaves\n";
        cout << "5. View Short Attendance Report\n";
        cout << "6. Back to Main Menu\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
        case 1: {
            cout << "Enter Employee ID: ";
            int empID;
            cin >> empID;

            Employee* emp = system.getEmployee(empID);
            string supID = std::to_string(id);
            if (emp->supervisorID != supID) {
                cout << "You are not this Employee's supervisor";
                return;
            }
            if (emp) {
                cout << "Employee ID " << empID << " found.\n";
                cout << "Pending Leaves:\n";

                vector<Leave*>& leaves = emp->leaveApplications;
                cout << "Number of leave applications: " << leaves.size() << endl;

                for (size_t i = 0; i < leaves.size(); ++i) {
                    cout << "Leave " << i + 1 << " status: " << leaves[i]->status << endl; // Debugging status
                    if (leaves[i]->status == "Pending") {
                        cout << i + 1 << ". " << leaves[i]->toString() << endl;
                    }
                }

                if (leaves.empty()) {
                    cout << "No leave applications found for this employee.\n";
                }

                cout << "Enter leave number to process (0 to cancel): ";
                int leaveChoice;
                cin >> leaveChoice;

                if (leaveChoice > 0 && leaveChoice <= leaves.size()) {
                    Leave* leave = leaves[leaveChoice - 1];

                    if (leave->getType() == "Earned Leave") {
                        cout << "Do you want to forward this leave to the Director for approval? (1 = Yes, 2 = Disapprove): ";
                        int action;
                        cin >> action;

                        if (action == 1) {
                            leave->status = "Forwarded to Director";
                            cout << "Leave forwarded to Director.\n";
                        }
                        else if (action == 2) {
                            leave->disapprove();
                            system.saveDataToFile();
                            cout << "Leave disapproved by Supervisor.\n";
                        }
                        else {
                            cout << "Invalid action.\n";
                        }
                    }
                    else {
                        cout << "Approve (1) or Disapprove (2): ";
                        int action;
                        cin >> action;

                        if (action == 1) {
                            leave->approve();
                            cout << "Leave approved successfully.\n";
                            system.saveDataToFile();
                        }
                        else if (action == 2) {
                            leave->disapprove();
                            cout << "Leave disapproved successfully.\n";
                            system.saveDataToFile();
                        }
                        else {
                            cout << "Invalid action.\n";
                        }
                    }
                }
            }
            else {
                cout << "Invalid Employee ID.\n";
            }
            break;
        }

        case 2: { // New case for viewing outstanding casual leaves
            cout << "\n--- Outstanding Casual Leaves ---\n";
            for (int i = 0; i < system.employeeCount; ++i) {
                Employee* emp = system.arr[i];
                if (!emp) continue; // Skip null pointers

                vector<Leave*>& leaves = emp->leaveApplications;
                for (const auto& leave : leaves) {
                    if (leave->getType() == "Casual Leave" && leave->status == "Pending") {
                        cout << emp->getName() << " | Leave: " << leave->toString() << endl;

                        // Deduct the casual leave days
                        int days = emp->getLeaveBalance().calculateLeaveDays(leave->startDate, leave->endDate);
                        if (emp->getLeaveBalance().deductCasual(leave->startDate, leave->endDate)) {
                            cout << "Deducted " << days << " casual leave(s) from balance.\n";
                        }
                        else {
                            cout << "Insufficient casual leave balance for " << emp->getName() << ".\n";
                        }
                    }
                }
            }
            break;
        }


        case 3: {
            system.viewPendinggLeaves();
            break;
        }
        case 4:
            system.viewOutstandingLeaves();
            break;
        case 5:
            system.generateAttendanceReport();
            break;
        case 6:
            cout << "Returning to main menu.\n";
            mainMenuPtr(system);
            break;
        default:
            cout << "Invalid choice. Try again.\n";
        }
    } while (choice != 6);
}

void directorMenu(System& system) {
    int choice;
    do {
        cout << "\n--- Director Menu ---\n";
        cout << "1. Approve Leave\n";
        cout << "2. View All Leave Applications\n";
        cout << "3. Back to Main Menu\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
        case 1: {
            cout << "Enter Employee ID: ";
            int empID;
            cin >> empID;

            Employee* emp = system.getEmployee(empID);
            if (emp) {
                cout << "Pending Earned Leaves:\n";
                vector<Leave*>& leaves = emp->leaveApplications;
                for (size_t i = 0; i < leaves.size(); ++i) {
                    if (leaves[i]->getType() == "Earned Leave" && leaves[i]->status == "Forwarded to Director") {
                        cout << i + 1 << ". " << leaves[i]->toString() << endl;
                    }
                }

                cout << "Enter leave number to approve/disapprove (0 to cancel): ";
                int leaveChoice;
                cin >> leaveChoice;

                if (leaveChoice > 0 && leaveChoice <= leaves.size()) {
                    Leave* leave = leaves[leaveChoice - 1];
                    cout << "Approve (1) or Disapprove (2): ";
                    int action;
                    cin >> action;

                    if (action == 1) {
                        leave->approve();
                        cout << "Leave approved successfully.\n";
                        system.saveDataToFile();

                    }
                    else if (action == 2) {
                        leave->disapprove();
                        cout << "Leave disapproved successfully.\n";
                        system.saveDataToFile();

                    }
                    else {
                        cout << "Invalid action.\n";
                    }
                }
            }
            else {
                cout << "Invalid Employee ID.\n";
            }
            break;
        }
        case 2:
            system.viewOutstandingLeaves();
            break;
        case 3:
            cout << "Returning to main menu.\n";
            mainMenuPtr(system);
            break;
        default:
            cout << "Invalid choice. Try again.\n";
        }
    } while (choice != 3);
}

void mainMenu(System& system) {
    int choice;
    do {
        cout << "\n--- Employee Management System ---\n";
        cout << "1. Employee Login\n";
        cout << "2. Guard Login\n";
        cout << "3. Supervisor Login\n";
        cout << "4. Director Login\n";
        cout << "5. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
        case 1: {
            cout << "Enter Employee ID: ";
            int empID;
            cin >> empID;
            Employee* emp = system.getEmployee(empID);

            if (emp) {
                employeeMenu(*emp, system);//access to employee menu
                system.saveDataToFile();
            }
            else {
                cout << "Invalid Employee ID.\n";
            }
            break;
        }
        case 2: {
            cout << "Enter Guard ID: ";
            int guardID;
            cin >> guardID;

            Employee* employee = system.getEmployee(guardID);

            // Check if the employee is a guard
            Guard* guard = dynamic_cast<Guard*>(employee);
            if (guard) {
                cout << "Guard " << guard->getName() << " logged in.\n";

                // Proceed to mark attendance for an employee
                cout << "Enter Employee ID to mark attendance: ";
                int empID;
                cin >> empID;

                Employee* emp = system.getEmployee(empID);

                if (emp) {
                    cout << "Enter date (YYYY-MM-DD), start time (HH:mm), and end time (HH:mm): ";
                    string date, start, end;
                    cin >> date >> start >> end;

                    guard->markAttendance(*emp, date, start, end);
                    system.saveDataToFile(); //saving data
                }
                else {
                    cout << "Invalid Employee ID.\n";
                }
            }
            else {
                cout << "Invalid Guard ID or not authorized to mark attendance.\n";
            }
            break;
        }
        case 3: {
            cout << "Enter Supervisor ID: ";
            int id;
            cin >> id;

            Employee* employee = system.getEmployee(id);

            if (employee && employee->role == "Supervisor") {
                cout << "Supervisor " << employee->getName() << " logged in.\n";
                supervisorMenu(system, id); // Access the supervisor menu
            }
            else {
                cout << "Invalid Supervisor ID or not authorized.\n";
            }
            break;
        }
        case 4: {
            cout << "Enter Director ID: ";
            int id;
            cin >> id;

            Employee* employee = system.getEmployee(id);

            if (employee && employee->role == "Director") {
                cout << "Director " << employee->getName() << " logged in.\n";
                directorMenu(system); // Access the director menu
            }
            else {
                cout << "Invalid Director ID or not authorized.\n";
            }
            break;
        }
        case 5: {
            cout << "Exiting the system.\n";
            return;
        }
        default:
            cout << "Invalid choice. Try again.\n";
        }
    } while (choice != 6);
}

int main() {
    System system;
    system.loadDataFromFile();
    system.loadAttendanceFromFile();
    mainMenuPtr = mainMenu;
    //    system.viewOutstandingLeaves();
    mainMenu(system);

    system.saveDataToFile();

    return 0;
}