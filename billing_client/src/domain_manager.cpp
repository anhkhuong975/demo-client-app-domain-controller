#include "domain_manager.h"
#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#include <ldap.h>
#include <cstring>

DomainManager::DomainManager(ConfigManager* config_mgr) 
    : current_domain(""), domain_controller(""), is_authenticated(false), username(""), config_manager(config_mgr) {
    std::cout << "[DomainManager] Initialized domain manager with LDAP support" << std::endl;
}

DomainManager::~DomainManager() {
    if (is_authenticated) {
        disconnect();
    }
    std::cout << "[DomainManager] Domain manager destroyed" << std::endl;
}

bool DomainManager::authenticate(const std::string& domain_name, 
                                const std::string& user_name, 
                                const std::string& password) {
    std::cout << "[DomainManager] Attempting LDAP authentication with domain: " << domain_name << std::endl;
    std::cout << "[DomainManager] Username: " << user_name << std::endl;
    
    // Validate input parameters
    if (domain_name.empty() || user_name.empty() || password.empty()) {
        std::cout << "[DomainManager] Authentication failed: Missing credentials" << std::endl;
        return false;
    }
    
    if (!config_manager) {
        std::cout << "[DomainManager] Authentication failed: No configuration manager available" << std::endl;
        return false;
    }
    
    // Get LDAP configuration
    LdapConfig ldap_config = config_manager->get_ldap_config();
    
    // Validate LDAP configuration
    if (ldap_config.ldap_server.empty() || ldap_config.base_dn.empty()) {
        std::cout << "[DomainManager] Authentication failed: Invalid LDAP configuration" << std::endl;
        std::cout << "[DomainManager] Please check your LDAP settings in config file" << std::endl;
        return false;
    }
    
    std::cout << "[DomainManager] Connecting to LDAP server: " << ldap_config.ldap_server 
              << ":" << ldap_config.ldap_port << std::endl;
    
    LDAP* ldap_connection = nullptr;
    int result;
    
    // Initialize LDAP connection
    std::string ldap_uri = (ldap_config.ldap_use_ssl ? "ldaps://" : "ldap://") 
                          + ldap_config.ldap_server + ":" + std::to_string(ldap_config.ldap_port);
    
    result = ldap_initialize(&ldap_connection, ldap_uri.c_str());
    if (result != LDAP_SUCCESS) {
        std::cout << "[DomainManager] LDAP initialization failed: " << ldap_err2string(result) << std::endl;
        return false;
    }
    
    // Set LDAP version to 3
    int ldap_version = LDAP_VERSION3;
    result = ldap_set_option(ldap_connection, LDAP_OPT_PROTOCOL_VERSION, &ldap_version);
    if (result != LDAP_OPT_SUCCESS) {
        std::cout << "[DomainManager] Failed to set LDAP version: " << ldap_err2string(result) << std::endl;
        ldap_unbind_ext_s(ldap_connection, nullptr, nullptr);
        return false;
    }
    
    // Set connection timeout
    struct timeval timeout;
    timeout.tv_sec = ldap_config.ldap_timeout;
    timeout.tv_usec = 0;
    result = ldap_set_option(ldap_connection, LDAP_OPT_NETWORK_TIMEOUT, &timeout);
    if (result != LDAP_OPT_SUCCESS) {
        std::cout << "[DomainManager] Warning: Failed to set LDAP timeout" << std::endl;
    }
    
    // Create bind DN from username
    std::string bind_dn = ldap_config.bind_dn_format;
    size_t pos = bind_dn.find("%s");
    if (pos != std::string::npos) {
        bind_dn.replace(pos, 2, user_name);
    }
    
    std::cout << "[DomainManager] Attempting to bind with DN: " << bind_dn << std::endl;
    
    // Perform LDAP bind (authentication)
    struct berval cred;
    cred.bv_val = const_cast<char*>(password.c_str());
    cred.bv_len = password.length();
    
    result = ldap_sasl_bind_s(ldap_connection, bind_dn.c_str(), LDAP_SASL_SIMPLE, &cred, nullptr, nullptr, nullptr);
    
    if (result == LDAP_SUCCESS) {
        // Authentication successful
        std::cout << "[DomainManager] LDAP authentication successful!" << std::endl;
        
        current_domain = domain_name;
        username = user_name;
        domain_controller = ldap_config.ldap_server;
        is_authenticated = true;
        
        std::cout << "[DomainManager] Connected to domain controller: " << domain_controller << std::endl;
        
        // Clean up
        ldap_unbind_ext_s(ldap_connection, nullptr, nullptr);
        return true;
    } else {
        // Authentication failed
        std::cout << "[DomainManager] LDAP authentication failed: " << ldap_err2string(result) << std::endl;
        
        if (result == LDAP_INVALID_CREDENTIALS) {
            std::cout << "[DomainManager] Invalid username or password" << std::endl;
        } else if (result == LDAP_SERVER_DOWN) {
            std::cout << "[DomainManager] Cannot connect to LDAP server. Check server address and network connectivity" << std::endl;
        } else if (result == LDAP_TIMEOUT) {
            std::cout << "[DomainManager] Connection timeout. Check network connectivity" << std::endl;
        } else {
            std::cout << "[DomainManager] Please check your credentials and LDAP configuration" << std::endl;
        }
        
        // Clean up
        ldap_unbind_ext_s(ldap_connection, nullptr, nullptr);
        return false;
    }
}

bool DomainManager::is_connected() const {
    return is_authenticated;
}

std::string DomainManager::get_current_domain() const {
    return current_domain;
}

std::vector<std::string> DomainManager::get_domain_users() {
    std::vector<std::string> users;
    
    if (!is_authenticated) {
        std::cout << "[DomainManager] Error: Not authenticated with any domain" << std::endl;
        return users;
    }
    
    std::cout << "[DomainManager] Querying domain users from: " << current_domain << std::endl;
    
    // Mock user data - in real implementation, this would use NetUserEnum or LDAP queries
    std::vector<std::string> mock_users = {
        "john.doe", "jane.smith", "mike.johnson", "sarah.wilson", 
        "david.brown", "lisa.davis", "robert.miller", "emily.garcia",
        "james.martinez", "maria.rodriguez"
    };
    
    // Simulate network delay for domain query
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Return a subset of mock users to simulate real domain query
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(5, 10);
    
    int user_count = dis(gen);
    for (int i = 0; i < user_count && i < static_cast<int>(mock_users.size()); i++) {
        users.push_back(mock_users[i]);
    }
    
    std::cout << "[DomainManager] Found " << users.size() << " users in domain" << std::endl;
    return users;
}

std::string DomainManager::get_domain_controller() const {
    return domain_controller;
}

void DomainManager::disconnect() {
    if (is_authenticated) {
        std::cout << "[DomainManager] Disconnecting from domain: " << current_domain << std::endl;
        
        current_domain.clear();
        domain_controller.clear();
        username.clear();
        is_authenticated = false;
        
        std::cout << "[DomainManager] Successfully disconnected" << std::endl;
    }
}
