#ifndef BILLING_SERVER_H
#define BILLING_SERVER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <thread>
#include <atomic>

/**
 * @brief Structure to hold received working hours data
 */
struct ReceivedWorkingHoursData {
    std::string employee_id;
    std::string employee_name;
    std::string department;
    double hours_worked;
    std::string date;
    std::string project_code;
    std::string received_timestamp;
    std::string client_source;
};

/**
 * @brief Structure to hold client information
 */
struct ClientInfo {
    std::string timestamp;
    std::string client_version;
    std::string data_source;
    std::string client_ip;
};

/**
 * @brief HTTP Billing Server for Domain Controller
 * 
 * This server receives working hours data from Windows billing clients
 * and provides endpoints for viewing and managing the collected data.
 */
class BillingServer {
private:
    int server_port;
    std::atomic<bool> running;
    std::thread server_thread;
    
    // Data storage
    std::vector<ReceivedWorkingHoursData> received_data;
    std::vector<ClientInfo> client_connections;
    
    // Server socket
    int server_socket;
    
    /**
     * @brief Initialize server socket
     * @return true if successful, false otherwise
     */
    bool initialize_socket();
    
    /**
     * @brief Main server loop
     */
    void server_loop();
    
    /**
     * @brief Handle incoming HTTP request
     * @param client_socket Client socket descriptor
     * @param client_ip Client IP address
     */
    void handle_request(int client_socket, const std::string& client_ip);
    
    /**
     * @brief Parse HTTP request headers
     * @param request Raw HTTP request string
     * @return Map of headers
     */
    std::map<std::string, std::string> parse_headers(const std::string& request);
    
    /**
     * @brief Parse JSON payload from billing client
     * @param json_payload JSON string containing working hours data
     * @param client_ip Source client IP
     * @return true if parsing successful, false otherwise
     */
    bool parse_billing_data(const std::string& json_payload, const std::string& client_ip);
    
    /**
     * @brief Generate HTTP response for billing data submission
     * @param success Whether the data was processed successfully
     * @return HTTP response string
     */
    std::string generate_response(bool success);
    
    /**
     * @brief Generate HTML dashboard for viewing data
     * @return HTML string
     */
    std::string generate_dashboard();
    
    /**
     * @brief Generate JSON API response with all data
     * @return JSON string
     */
    std::string generate_api_response();

public:
    /**
     * @brief Constructor for BillingServer
     * @param port Server port (default 8080)
     */
    BillingServer(int port = 8080);
    
    /**
     * @brief Destructor for BillingServer
     */
    ~BillingServer();
    
    /**
     * @brief Start the billing server
     * @return true if server started successfully, false otherwise
     */
    bool start();
    
    /**
     * @brief Stop the billing server
     */
    void stop();
    
    /**
     * @brief Check if server is running
     * @return true if running, false otherwise
     */
    bool is_running() const;
    
    /**
     * @brief Get received working hours data
     * @return Vector of received data
     */
    std::vector<ReceivedWorkingHoursData> get_received_data() const;
    
    /**
     * @brief Get client connection information
     * @return Vector of client info
     */
    std::vector<ClientInfo> get_client_info() const;
    
    /**
     * @brief Display server statistics
     */
    void display_statistics() const;
    
    /**
     * @brief Save received data to CSV file
     * @param filename Output filename
     * @return true if successful, false otherwise
     */
    bool export_to_csv(const std::string& filename) const;
};

#endif // BILLING_SERVER_H
