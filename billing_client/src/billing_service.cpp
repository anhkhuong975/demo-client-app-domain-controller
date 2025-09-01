#include "billing_service.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <set>

BillingService::BillingService() 
    : service_name(""), is_running(false), hourly_rate(50.0) {
    std::cout << "[BillingService] Billing service initialized" << std::endl;
}

BillingService::~BillingService() {
    if (is_running) {
        stop_service();
    }
    std::cout << "[BillingService] Billing service destroyed" << std::endl;
}

bool BillingService::initialize(const std::string& service_name, double rate) {
    this->service_name = service_name;
    this->hourly_rate = rate;
    
    std::cout << "[BillingService] Service initialized: " << service_name << std::endl;
    std::cout << "[BillingService] Hourly rate set to: $" << std::fixed 
              << std::setprecision(2) << rate << std::endl;
    
    return true;
}

bool BillingService::start_service() {
    if (service_name.empty()) {
        std::cout << "[BillingService] Error: Service not initialized" << std::endl;
        return false;
    }
    
    if (is_running) {
        std::cout << "[BillingService] Warning: Service is already running" << std::endl;
        return true;
    }
    
    is_running = true;
    std::cout << "[BillingService] Service started: " << service_name << std::endl;
    return true;
}

void BillingService::stop_service() {
    if (is_running) {
        is_running = false;
        std::cout << "[BillingService] Service stopped: " << service_name << std::endl;
    }
}

BillingReport BillingService::generate_report(const std::string& domain_name, 
                                             const std::vector<WorkingHoursData>& collected_data) {
    std::cout << "[BillingService] Generating billing report for domain: " << domain_name << std::endl;
    
    BillingReport report;
    report.domain_name = domain_name;
    report.detailed_data = collected_data;
    report.currency = "USD";
    
    // Get current date for report
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream date_stream;
    date_stream << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    report.report_date = date_stream.str();
    
    // Calculate totals
    report.total_hours = 0.0;
    std::set<std::string> unique_employees;
    
    for (const auto& data : collected_data) {
        report.total_hours += data.hours_worked;
        unique_employees.insert(data.employee_id);
    }
    
    report.employee_count = static_cast<int>(unique_employees.size());
    report.total_cost = calculate_billing_cost(collected_data);
    
    // Store the generated report
    generated_reports.push_back(report);
    
    std::cout << "[BillingService] Report generated successfully:" << std::endl;
    std::cout << "  - Domain: " << report.domain_name << std::endl;
    std::cout << "  - Total Hours: " << std::fixed << std::setprecision(2) 
              << report.total_hours << std::endl;
    std::cout << "  - Employee Count: " << report.employee_count << std::endl;
    std::cout << "  - Total Cost: $" << std::fixed << std::setprecision(2) 
              << report.total_cost << std::endl;
    
    return report;
}

double BillingService::calculate_billing_cost(const std::vector<WorkingHoursData>& hours_data) {
    double total_cost = 0.0;
    
    for (const auto& data : hours_data) {
        total_cost += data.hours_worked * hourly_rate;
    }
    
    return total_cost;
}

std::vector<BillingReport> BillingService::get_all_reports() const {
    return generated_reports;
}

bool BillingService::export_report(const BillingReport& report, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cout << "[BillingService] Error: Could not open file for writing: " << filename << std::endl;
        return false;
    }
    
    // Write report header
    file << "=== BILLING REPORT ===" << std::endl;
    file << "Domain: " << report.domain_name << std::endl;
    file << "Report Date: " << report.report_date << std::endl;
    file << "Currency: " << report.currency << std::endl;
    file << std::endl;
    
    // Write summary
    file << "=== SUMMARY ===" << std::endl;
    file << "Total Hours: " << std::fixed << std::setprecision(2) << report.total_hours << std::endl;
    file << "Employee Count: " << report.employee_count << std::endl;
    file << "Total Cost: $" << std::fixed << std::setprecision(2) << report.total_cost << std::endl;
    file << "Average Hours per Employee: " << std::fixed << std::setprecision(2) 
         << (report.employee_count > 0 ? report.total_hours / report.employee_count : 0.0) << std::endl;
    file << std::endl;
    
    // Write detailed data
    file << "=== DETAILED DATA ===" << std::endl;
    file << std::left << std::setw(15) << "Employee ID" 
         << std::setw(20) << "Employee Name"
         << std::setw(15) << "Department"
         << std::setw(12) << "Hours"
         << std::setw(12) << "Date"
         << std::setw(12) << "Project"
         << std::setw(10) << "Cost" << std::endl;
    file << std::string(96, '-') << std::endl;
    
    for (const auto& data : report.detailed_data) {
        double individual_cost = data.hours_worked * hourly_rate;
        file << std::left << std::setw(15) << data.employee_id
             << std::setw(20) << data.employee_name
             << std::setw(15) << data.department
             << std::setw(12) << std::fixed << std::setprecision(2) << data.hours_worked
             << std::setw(12) << data.date
             << std::setw(12) << data.project_code
             << "$" << std::setw(9) << std::fixed << std::setprecision(2) << individual_cost << std::endl;
    }
    
    file.close();
    std::cout << "[BillingService] Report exported to: " << filename << std::endl;
    return true;
}

bool BillingService::is_service_running() const {
    return is_running;
}

void BillingService::set_hourly_rate(double rate) {
    hourly_rate = rate;
    std::cout << "[BillingService] Hourly rate updated to: $" << std::fixed 
              << std::setprecision(2) << rate << std::endl;
}

double BillingService::get_hourly_rate() const {
    return hourly_rate;
}
