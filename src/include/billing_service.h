#ifndef BILLING_SERVICE_H
#define BILLING_SERVICE_H

#include <string>
#include <vector>
#include "data_collector.h"

/**
 * @brief Structure to hold billing report data
 */
struct BillingReport {
    std::string domain_name;
    std::string report_date;
    double total_hours;
    int employee_count;
    std::vector<WorkingHoursData> detailed_data;
    double total_cost;
    std::string currency;
};

/**
 * @brief Main billing service that orchestrates data collection and reporting
 * 
 * This class coordinates between domain authentication, data collection,
 * and report generation. It represents the main business logic of the
 * billing client application.
 */
class BillingService {
private:
    std::string service_name;
    bool is_running;
    std::vector<BillingReport> generated_reports;
    double hourly_rate;
    
    /**
     * @brief Calculate billing cost based on working hours
     * @param hours_data Vector of working hours data
     * @return Total cost
     */
    double calculate_billing_cost(const std::vector<WorkingHoursData>& hours_data);
    
public:
    /**
     * @brief Constructor for BillingService
     */
    BillingService();
    
    /**
     * @brief Destructor for BillingService
     */
    ~BillingService();
    
    /**
     * @brief Initialize the billing service
     * @param service_name Name of the service instance
     * @param rate Hourly billing rate
     * @return true if initialization successful, false otherwise
     */
    bool initialize(const std::string& service_name, double rate = 50.0);
    
    /**
     * @brief Start the billing service
     * @return true if service started successfully, false otherwise
     */
    bool start_service();
    
    /**
     * @brief Stop the billing service
     */
    void stop_service();
    
    /**
     * @brief Generate billing report for a specific domain
     * @param domain_name Target domain name
     * @param collected_data Working hours data from data collector
     * @return BillingReport structure
     */
    BillingReport generate_report(const std::string& domain_name, 
                                 const std::vector<WorkingHoursData>& collected_data);
    
    /**
     * @brief Get all generated reports
     * @return Vector of BillingReport
     */
    std::vector<BillingReport> get_all_reports() const;
    
    /**
     * @brief Export report to file
     * @param report BillingReport to export
     * @param filename Output filename
     * @return true if export successful, false otherwise
     */
    bool export_report(const BillingReport& report, const std::string& filename);
    
    /**
     * @brief Check if service is running
     * @return true if running, false otherwise
     */
    bool is_service_running() const;
    
    /**
     * @brief Set hourly billing rate
     * @param rate New hourly rate
     */
    void set_hourly_rate(double rate);
    
    /**
     * @brief Get current hourly rate
     * @return Current hourly rate
     */
    double get_hourly_rate() const;
};

#endif // BILLING_SERVICE_H
