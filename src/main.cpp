#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <set>

// Include our custom headers
#include "domain_manager.h"
#include "data_collector.h"
#include "config_manager.h"

/**
 * @brief Display application banner and information
 */
void display_banner() {
    std::cout << "========================================" << std::endl;
    std::cout << "    Windows Billing Client Demo" << std::endl;
    std::cout << "    Multi-Domain Data Collection" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
}

/**
 * @brief Display configuration summary on startup
 * @param config_manager Reference to ConfigManager instance
 */
void display_config_summary(const ConfigManager& config_manager) {
    std::cout << "\n=== Configuration Summary ===" << std::endl;
    
    // Show configured domains count
    auto domains = config_manager.get_configured_domains();
    std::cout << "Configured Domains: " << domains.size() << std::endl;
    
    for (const auto& domain_name : domains) {
        auto domain_config = config_manager.get_domain_config(domain_name);
        std::cout << "  - " << domain_config.domain_name 
                  << " (" << domain_config.domain_controller << ")" << std::endl;
    }
    
    // Show key global settings
    std::cout << "LDAP Server: " << config_manager.get_global_setting("ldap_server", "Not configured") << std::endl;
    std::cout << "Log Level: " << config_manager.get_global_setting("log_level", "INFO") << std::endl;
    std::cout << "Output Directory: " << config_manager.get_global_setting("output_directory", "output") << std::endl;
    
    std::cout << "\n💡 Configuration file location: config/billing_client.conf" << std::endl;
    std::cout << "   Edit this file to modify domains, settings, and authentication parameters." << std::endl;
    std::cout << "========================================" << std::endl;
}

/**
 * @brief Display menu options to user
 */
void display_menu() {
    std::cout << "\n--- Main Menu ---" << std::endl;
    std::cout << "1. View Domain Controller Configuration and Status" << std::endl;
    std::cout << "2. Connect to Domain" << std::endl;
    std::cout << "3. Collect Working Hours Data" << std::endl;
    std::cout << "4. Exit" << std::endl;
    std::cout << "Enter your choice (1-4): ";
}

/**
 * @brief Display domain controller configuration and connection status
 * @param config_manager Reference to ConfigManager instance
 * @param domain_manager Reference to DomainManager instance
 * @param current_domain Current connected domain name
 * @param domain_connected Connection status
 */
void display_domain_controller_status(const ConfigManager& config_manager, 
                                     const DomainManager& domain_manager,
                                     const std::string& current_domain,
                                     bool domain_connected) {
    std::cout << "\n=== Domain Controller Configuration and Status ===" << std::endl;
    
    // Display configured domains
    auto domains = config_manager.get_configured_domains();
    std::cout << "\nConfigured Domains: " << domains.size() << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    for (const auto& domain_name : domains) {
        auto domain_config = config_manager.get_domain_config(domain_name);
        std::cout << "Domain: " << domain_config.domain_name << std::endl;
        std::cout << "  Controller IP: " << domain_config.domain_controller << std::endl;
        std::cout << "  Service Account: " << domain_config.service_account << std::endl;
        std::cout << "  SSL Enabled: " << (domain_config.use_ssl ? "Yes" : "No") << std::endl;
        std::cout << "  Connection Timeout: " << domain_config.connection_timeout << "s" << std::endl;
        std::cout << "  Data Sources: " << std::endl;
        for (const auto& source : domain_config.data_sources) {
            std::cout << "    - " << source << std::endl;
        }
        
        // Show connection status for this domain
        if (domain_connected && domain_name == current_domain) {
            std::cout << "  Status: CONNECTED (Active)" << std::endl;
        } else {
            std::cout << "  Status: DISCONNECTED" << std::endl;
        }
        std::cout << "----------------------------------------" << std::endl;
    }
    
    // Display LDAP configuration
    std::cout << "\nLDAP Authentication Settings:" << std::endl;
    std::cout << "  LDAP Server: " << config_manager.get_global_setting("ldap_server", "Not configured") << std::endl;
    std::cout << "  LDAP Port: " << config_manager.get_global_setting("ldap_port", "389") << std::endl;
    std::cout << "  SSL/TLS: " << (config_manager.get_global_setting("ldap_use_ssl", "false") == "true" ? "Enabled" : "Disabled") << std::endl;
    std::cout << "  Base DN: " << config_manager.get_global_setting("base_dn", "Not configured") << std::endl;
    std::cout << "  Timeout: " << config_manager.get_global_setting("ldap_timeout", "10") << "s" << std::endl;
    
    // Display global settings
    std::cout << "\nGlobal Settings:" << std::endl;
    std::cout << "  Log Level: " << config_manager.get_global_setting("log_level", "INFO") << std::endl;
    std::cout << "  Collection Interval: " << config_manager.get_global_setting("data_collection_interval", "3600") << "s" << std::endl;
    std::cout << "  Output Directory: " << config_manager.get_global_setting("output_directory", "output") << std::endl;
    std::cout << "  Max Retry Attempts: " << config_manager.get_global_setting("max_retry_attempts", "3") << std::endl;
    
    // Current connection summary
    std::cout << "\nCurrent Connection Status:" << std::endl;
    if (domain_connected) {
        std::cout << "  Connected to: " << current_domain << std::endl;
        std::cout << "  Status: ACTIVE" << std::endl;
    } else {
        std::cout << "  Status: NO ACTIVE CONNECTIONS" << std::endl;
    }
}

/**
 * @brief Main application entry point
 * 
 * This function demonstrates the complete workflow of a Windows billing client:
 * 1. Load configuration for multiple domains
 * 2. Authenticate with Windows domains
 * 3. Collect working hours data from domain users
 * 
 * @return 0 on success, non-zero on error
 */
int main() {
    display_banner();
    
    // Initialize core components
    std::unique_ptr<ConfigManager> config_manager = std::make_unique<ConfigManager>("config/billing_client.conf");
    std::unique_ptr<DomainManager> domain_manager = std::make_unique<DomainManager>(config_manager.get());
    std::unique_ptr<DataCollector> data_collector = std::make_unique<DataCollector>();
    
    // Load configuration automatically on startup
    std::cout << "Loading configuration..." << std::endl;
    if (!config_manager->load_config()) {
        std::cout << "Error: Failed to load configuration file" << std::endl;
        return 1;
    }
    std::cout << "Configuration loaded successfully!" << std::endl;
    
    // Display configuration summary
    display_config_summary(*config_manager);
    
    // Application state variables
    bool domain_connected = false;
    bool data_collected = false;
    std::string current_domain = "";
    std::vector<WorkingHoursData> collected_data;
    
    // Main application loop
    int choice = 0;
    while (choice != 4) {
        display_menu();
        std::cin >> choice;
        
        switch (choice) {
            case 1: {
                // View Domain Controller Configuration and Status
                display_domain_controller_status(*config_manager, *domain_manager, current_domain, domain_connected);
                break;
            }
            
            case 2: {
                // Connect to Domain
                
                auto domains = config_manager->get_configured_domains();
                if (domains.empty()) {
                    std::cout << "\nError: No domains configured." << std::endl;
                    break;
                }
                
                // Auto-select if only one domain is available
                if (domains.size() == 1) {
                    current_domain = domains[0];
                    std::cout << "\nOnly one domain available. Auto-selecting: " << current_domain << std::endl;
                } else {
                    // Multiple domains - let user choose
                    std::cout << "\nAvailable domains:" << std::endl;
                    for (size_t i = 0; i < domains.size(); i++) {
                        std::cout << (i + 1) << ". " << domains[i] << std::endl;
                    }
                    
                    std::cout << "Select domain (1-" << domains.size() << "): ";
                    int domain_choice;
                    std::cin >> domain_choice;
                    
                    if (domain_choice < 1 || domain_choice > static_cast<int>(domains.size())) {
                        std::cout << "Invalid domain selection." << std::endl;
                        break;
                    }
                    
                    current_domain = domains[domain_choice - 1];
                }
                
                // Get credentials (in real app, this would be more secure)
                std::string username, password;
                std::cout << "Enter username: ";
                std::cin >> username;
                std::cout << "Enter password: ";
                std::cin >> password;
                
                // Attempt domain authentication
                if (domain_manager->authenticate(current_domain, username, password)) {
                    domain_connected = true;
                    data_collector->set_target_domain(current_domain);
                    std::cout << "Successfully connected to domain: " << current_domain << std::endl;
                } else {
                    std::cout << "Failed to connect to domain." << std::endl;
                }
                break;
            }
            
            case 3: {
                // Collect Working Hours Data
                if (!domain_connected) {
                    std::cout << "\nError: Please connect to a domain first." << std::endl;
                    break;
                }
                
                std::cout << "\nCollecting working hours data..." << std::endl;
                
                // Get domain users
                auto users = domain_manager->get_domain_users();
                if (users.empty()) {
                    std::cout << "No users found in domain." << std::endl;
                    break;
                }
                
                // Collect data for all users
                if (data_collector->collect_working_hours(users)) {
                    collected_data = data_collector->get_collected_data();
                    data_collected = true;
                    
                    // Display summary
                    auto summary = data_collector->get_data_summary();
                    std::cout << "\nData Collection Summary:" << std::endl;
                    std::cout << "  Total Hours: " << summary["total_hours"] << std::endl;
                    std::cout << "  Employee Count: " << summary["employee_count"] << std::endl;
                    std::cout << "  Average Hours: " << summary["average_hours"] << std::endl;
                } else {
                    std::cout << "Failed to collect working hours data." << std::endl;
                }
                break;
            }
            
            case 4: {
                // Exit
                std::cout << "\nShutting down billing client..." << std::endl;
                break;
            }
            
            default: {
                std::cout << "\nInvalid choice. Please select 1-4." << std::endl;
                break;
            }
        }
    }
    
    // Cleanup
    if (domain_connected) {
        domain_manager->disconnect();
    }
    
    std::cout << "\nBilling client terminated successfully." << std::endl;
    std::cout << "Thank you for using Windows Billing Client Demo!" << std::endl;
    
    return 0;
}
