#ifndef DATA_COLLECTOR_H
#define DATA_COLLECTOR_H

#include <string>
#include <vector>
#include <map>

/**
 * @brief Structure to hold working hours data for billing
 */
struct WorkingHoursData {
    std::string employee_id;
    std::string employee_name;
    std::string department;
    double hours_worked;
    std::string date;
    std::string project_code;
};

/**
 * @brief Collects billing-related data from domain resources
 * 
 * This class simulates data collection from various domain sources
 * such as user logon/logoff times, application usage, and system resources.
 * In a real implementation, this would query WMI, Event Logs, and other Windows APIs.
 */
class DataCollector {
private:
    std::string target_domain;
    std::vector<WorkingHoursData> collected_data;
    
    /**
     * @brief Generate random working hours data for demonstration
     * @param employee_count Number of employees to generate data for
     * @return Vector of WorkingHoursData
     */
    std::vector<WorkingHoursData> generate_mock_data(int employee_count);
    
public:
    /**
     * @brief Constructor for DataCollector
     */
    DataCollector();
    
    /**
     * @brief Destructor for DataCollector
     */
    ~DataCollector();
    
    /**
     * @brief Set the target domain for data collection
     * @param domain_name Domain to collect data from
     */
    void set_target_domain(const std::string& domain_name);
    
    /**
     * @brief Collect working hours data from domain users
     * @param user_list List of users to collect data for
     * @return true if collection successful, false otherwise
     */
    bool collect_working_hours(const std::vector<std::string>& user_list);
    
    /**
     * @brief Get collected working hours data
     * @return Vector of WorkingHoursData
     */
    std::vector<WorkingHoursData> get_collected_data() const;
    
    /**
     * @brief Clear all collected data
     */
    void clear_data();
    
    /**
     * @brief Get summary statistics of collected data
     * @return Map with statistics (total_hours, employee_count, etc.)
     */
    std::map<std::string, double> get_data_summary() const;
    
    /**
     * @brief Export collected data to CSV format
     * @param filename Output filename
     * @return true if export successful, false otherwise
     */
    bool export_to_csv(const std::string& filename) const;
    
    /**
     * @brief Transmit collected data to domain controller
     * @param dc_ip Domain controller IP address
     * @param port Server port (default 8080)
     * @param service_account Service account name
     * @return true if transmission successful, false otherwise
     */
    bool transmit_to_domain_controller(const std::string& dc_ip, int port = 8080, 
                                      const std::string& service_account = "");
};

#endif // DATA_COLLECTOR_H
