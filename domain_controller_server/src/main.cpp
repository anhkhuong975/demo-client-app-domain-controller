#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <signal.h>
#include "billing_server.h"

// Global server instance for signal handling
BillingServer* global_server = nullptr;

/**
 * @brief Signal handler for graceful shutdown
 * @param signal Signal number
 */
void signal_handler(int signal) {
    std::cout << "\n[Main] Received signal " << signal << ", shutting down server..." << std::endl;
    if (global_server) {
        global_server->stop();
    }
}

/**
 * @brief Display application banner
 */
void display_banner() {
    std::cout << "========================================" << std::endl;
    std::cout << "   Domain Controller Billing Server" << std::endl;
    std::cout << "   Receiving Working Hours Data" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
}

/**
 * @brief Display menu options
 */
void display_menu() {
    std::cout << "\n--- Server Control Menu ---" << std::endl;
    std::cout << "1. Start Server" << std::endl;
    std::cout << "2. Stop Server" << std::endl;
    std::cout << "3. View Statistics" << std::endl;
    std::cout << "4. Export Data to CSV" << std::endl;
    std::cout << "5. View Recent Data" << std::endl;
    std::cout << "6. Exit" << std::endl;
    std::cout << "Enter your choice (1-6): ";
}

/**
 * @brief Display recent received data
 * @param server Reference to BillingServer instance
 */
void display_recent_data(const BillingServer& server) {
    auto data = server.get_received_data();
    
    std::cout << "\n=== Recent Working Hours Data ===" << std::endl;
    if (data.empty()) {
        std::cout << "No data received yet." << std::endl;
        return;
    }
    
    // Show last 10 records
    int start_index = std::max(0, static_cast<int>(data.size()) - 10);
    
    std::cout << "Showing last " << (data.size() - start_index) << " records:" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    for (size_t i = start_index; i < data.size(); i++) {
        const auto& record = data[i];
        std::cout << "Employee: " << record.employee_name << " (" << record.employee_id << ")" << std::endl;
        std::cout << "  Department: " << record.department << std::endl;
        std::cout << "  Hours: " << record.hours_worked << " on " << record.date << std::endl;
        std::cout << "  Project: " << record.project_code << std::endl;
        std::cout << "  Received: " << record.received_timestamp << " from " << record.client_source << std::endl;
        std::cout << "----------------------------------------" << std::endl;
    }
}

/**
 * @brief Main application entry point
 * 
 * This application runs a domain controller server that receives
 * working hours data from Windows billing clients and provides
 * a web dashboard for viewing the collected data.
 * 
 * @return 0 on success, non-zero on error
 */
int main() {
    display_banner();
    
    // Set up signal handlers for graceful shutdown
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Create billing server instance
    BillingServer server(8080);
    global_server = &server;
    
    std::cout << "Domain Controller Billing Server initialized." << std::endl;
    std::cout << "This server will receive working hours data from billing clients." << std::endl;
    std::cout << std::endl;
    
    // Main application loop
    int choice = 0;
    while (choice != 6) {
        display_menu();
        std::cin >> choice;
        
        switch (choice) {
            case 1: {
                // Start Server
                if (server.is_running()) {
                    std::cout << "\nServer is already running!" << std::endl;
                } else {
                    std::cout << "\nStarting billing server..." << std::endl;
                    if (server.start()) {
                        std::cout << "\n🎉 Server started successfully!" << std::endl;
                        std::cout << "\nHow to test:" << std::endl;
                        std::cout << "1. Run the billing client application" << std::endl;
                        std::cout << "2. Connect to domain and collect working hours data" << std::endl;
                        std::cout << "3. Use option 4 to transmit data to this server" << std::endl;
                        std::cout << "4. View dashboard at: http://localhost:8080/dashboard" << std::endl;
                    } else {
                        std::cout << "\n❌ Failed to start server!" << std::endl;
                    }
                }
                break;
            }
            
            case 2: {
                // Stop Server
                if (!server.is_running()) {
                    std::cout << "\nServer is not running!" << std::endl;
                } else {
                    std::cout << "\nStopping server..." << std::endl;
                    server.stop();
                    std::cout << "Server stopped." << std::endl;
                }
                break;
            }
            
            case 3: {
                // View Statistics
                server.display_statistics();
                break;
            }
            
            case 4: {
                // Export Data to CSV
                std::string filename;
                std::cout << "\nEnter CSV filename (e.g., billing_data.csv): ";
                std::cin >> filename;
                
                if (server.export_to_csv("../domain_controller_server/data/" + filename)) {
                    std::cout << "✅ Data exported successfully!" << std::endl;
                } else {
                    std::cout << "❌ Failed to export data!" << std::endl;
                }
                break;
            }
            
            case 5: {
                // View Recent Data
                display_recent_data(server);
                break;
            }
            
            case 6: {
                // Exit
                std::cout << "\nShutting down server..." << std::endl;
                if (server.is_running()) {
                    server.stop();
                }
                break;
            }
            
            default: {
                std::cout << "\nInvalid choice. Please select 1-6." << std::endl;
                break;
            }
        }
        
        // Small delay to prevent menu spam
        if (choice != 6) {
            std::cout << "\nPress Enter to continue...";
            std::cin.ignore();
            std::cin.get();
        }
    }
    
    std::cout << "\nDomain Controller Billing Server terminated successfully." << std::endl;
    std::cout << "Thank you for using the billing server!" << std::endl;
    
    return 0;
}
