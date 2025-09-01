#ifndef DATA_TRANSMITTER_H
#define DATA_TRANSMITTER_H

#include "data_collector.h"
#include <string>
#include <vector>
#include <map>

/**
 * @brief Handles transmission of collected data to domain controller
 * 
 * This class manages network communication with the domain controller
 * to send working hours data for centralized billing processing.
 */
class DataTransmitter {
private:
    std::string domain_controller_ip;
    int server_port;
    std::string service_account;
    bool use_ssl;
    int connection_timeout;
    
    /**
     * @brief Authenticate with domain controller
     * @return true if authentication successful, false otherwise
     */
    bool authenticate_with_domain();
    
    /**
     * @brief Create HTTP request payload from working hours data
     * @param data Vector of working hours data
     * @return JSON formatted string
     */
    std::string create_json_payload(const std::vector<WorkingHoursData>& data);
    
    /**
     * @brief Send HTTP POST request to domain controller
     * @param payload JSON data to send
     * @return true if transmission successful, false otherwise
     */
    bool send_http_request(const std::string& payload);

public:
    /**
     * @brief Constructor for DataTransmitter
     */
    DataTransmitter();
    
    /**
     * @brief Destructor for DataTransmitter
     */
    ~DataTransmitter();
    
    /**
     * @brief Initialize transmitter with domain controller settings
     * @param dc_ip Domain controller IP address
     * @param port Server port (default 8080)
     * @param account Service account name
     * @param ssl_enabled Use SSL/TLS encryption
     * @param timeout Connection timeout in seconds
     * @return true if initialization successful, false otherwise
     */
    bool initialize(const std::string& dc_ip, int port = 8080, 
                   const std::string& account = "", bool ssl_enabled = false, 
                   int timeout = 30);
    
    /**
     * @brief Transmit working hours data to domain controller
     * @param data Vector of working hours data to transmit
     * @param domain_name Target domain name
     * @return true if transmission successful, false otherwise
     */
    bool transmit_working_hours(const std::vector<WorkingHoursData>& data, 
                               const std::string& domain_name);
    
    /**
     * @brief Test connection to domain controller
     * @return true if connection successful, false otherwise
     */
    bool test_connection();
    
    /**
     * @brief Get transmission statistics
     * @return Map with transmission stats (sent_records, success_rate, etc.)
     */
    std::map<std::string, double> get_transmission_stats() const;
};

#endif // DATA_TRANSMITTER_H
