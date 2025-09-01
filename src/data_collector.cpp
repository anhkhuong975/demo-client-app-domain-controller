#include "data_collector.h"
#include <iostream>
#include <random>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>

DataCollector::DataCollector() : target_domain("") {
    std::cout << "[DataCollector] Data collector initialized" << std::endl;
}

DataCollector::~DataCollector() {
    clear_data();
    std::cout << "[DataCollector] Data collector destroyed" << std::endl;
}

void DataCollector::set_target_domain(const std::string& domain_name) {
    target_domain = domain_name;
    std::cout << "[DataCollector] Target domain set to: " << domain_name << std::endl;
}

bool DataCollector::collect_working_hours(const std::vector<std::string>& user_list) {
    if (target_domain.empty()) {
        std::cout << "[DataCollector] Error: No target domain specified" << std::endl;
        return false;
    }
    
    if (user_list.empty()) {
        std::cout << "[DataCollector] Error: No users provided for data collection" << std::endl;
        return false;
    }
    
    std::cout << "[DataCollector] Starting data collection for " << user_list.size() 
              << " users in domain: " << target_domain << std::endl;
    
    // Clear previous data
    clear_data();
    
    // Generate mock data for each user
    collected_data = generate_mock_data(user_list.size());
    
    // Assign actual usernames to the generated data
    for (size_t i = 0; i < collected_data.size() && i < user_list.size(); i++) {
        collected_data[i].employee_id = user_list[i];
        collected_data[i].employee_name = user_list[i]; // In real app, would lookup full name
    }
    
    std::cout << "[DataCollector] Data collection completed. Collected " 
              << collected_data.size() << " records" << std::endl;
    
    return true;
}

std::vector<WorkingHoursData> DataCollector::generate_mock_data(int employee_count) {
    std::vector<WorkingHoursData> mock_data;
    
    // Random number generators
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> hours_dist(6.0, 10.0); // 6-10 hours per day
    std::uniform_int_distribution<> dept_dist(0, 4);
    std::uniform_int_distribution<> project_dist(0, 6);
    
    // Mock departments and projects
    std::vector<std::string> departments = {"IT", "Finance", "HR", "Marketing", "Operations"};
    std::vector<std::string> projects = {"PROJ001", "PROJ002", "PROJ003", "PROJ004", 
                                       "PROJ005", "PROJ006", "PROJ007"};
    
    // Get current date
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream date_stream;
    date_stream << std::put_time(&tm, "%Y-%m-%d");
    std::string current_date = date_stream.str();
    
    // Generate data for each employee
    for (int i = 0; i < employee_count; i++) {
        WorkingHoursData data;
        data.employee_id = "EMP" + std::to_string(i + 1);
        data.employee_name = "Employee " + std::to_string(i + 1);
        data.department = departments[dept_dist(gen)];
        data.hours_worked = hours_dist(gen);
        data.date = current_date;
        data.project_code = projects[project_dist(gen)];
        
        mock_data.push_back(data);
    }
    
    std::cout << "[DataCollector] Generated " << mock_data.size() 
              << " mock working hours records" << std::endl;
    
    return mock_data;
}

std::vector<WorkingHoursData> DataCollector::get_collected_data() const {
    return collected_data;
}

void DataCollector::clear_data() {
    collected_data.clear();
    std::cout << "[DataCollector] Collected data cleared" << std::endl;
}

std::map<std::string, double> DataCollector::get_data_summary() const {
    std::map<std::string, double> summary;
    
    if (collected_data.empty()) {
        summary["total_hours"] = 0.0;
        summary["employee_count"] = 0.0;
        summary["average_hours"] = 0.0;
        return summary;
    }
    
    double total_hours = 0.0;
    std::map<std::string, int> dept_count;
    
    for (const auto& data : collected_data) {
        total_hours += data.hours_worked;
        dept_count[data.department]++;
    }
    
    summary["total_hours"] = total_hours;
    summary["employee_count"] = static_cast<double>(collected_data.size());
    summary["average_hours"] = total_hours / collected_data.size();
    summary["department_count"] = static_cast<double>(dept_count.size());
    
    return summary;
}

bool DataCollector::export_to_csv(const std::string& filename) const {
    if (collected_data.empty()) {
        std::cout << "[DataCollector] Warning: No data to export" << std::endl;
        return false;
    }
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cout << "[DataCollector] Error: Could not open file for writing: " << filename << std::endl;
        return false;
    }
    
    // Write CSV header
    file << "Employee_ID,Employee_Name,Department,Hours_Worked,Date,Project_Code\n";
    
    // Write data rows
    for (const auto& data : collected_data) {
        file << data.employee_id << ","
             << data.employee_name << ","
             << data.department << ","
             << std::fixed << std::setprecision(2) << data.hours_worked << ","
             << data.date << ","
             << data.project_code << "\n";
    }
    
    file.close();
    std::cout << "[DataCollector] Data exported to: " << filename << std::endl;
    return true;
}
