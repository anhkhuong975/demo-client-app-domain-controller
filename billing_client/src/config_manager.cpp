#include "config_manager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

ConfigManager::ConfigManager(const std::string& config_path) 
    : config_file_path(config_path), config_loaded(false) {
    std::cout << "[ConfigManager] Config manager initialized with path: " << config_path << std::endl;
}

ConfigManager::~ConfigManager() {
    std::cout << "[ConfigManager] Config manager destroyed" << std::endl;
}

bool ConfigManager::load_config() {
    std::cout << "[ConfigManager] Loading configuration from: " << config_file_path << std::endl;
    
    // Check if config file exists
    if (!std::filesystem::exists(config_file_path)) {
        std::cout << "[ConfigManager] Config file not found at: " << config_file_path << std::endl;
        std::cout << "[ConfigManager] Please ensure the configuration file exists in the correct location" << std::endl;
        return false;
    }
    
    // Parse existing config file
    if (parse_config_file()) {
        config_loaded = true;
        std::cout << "[ConfigManager] Configuration loaded successfully" << std::endl;
        return true;
    }
    
    std::cout << "[ConfigManager] Failed to load configuration" << std::endl;
    return false;
}

bool ConfigManager::parse_config_file() {
    std::ifstream file(config_file_path);
    if (!file.is_open()) {
        std::cout << "[ConfigManager] Error: Could not open config file" << std::endl;
        return false;
    }
    
    std::string line;
    std::string current_section = "";
    DomainConfig current_domain_config;
    bool in_domain_section = false;
    
    while (std::getline(file, line)) {
        // Remove whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Check for section headers
        if (line[0] == '[' && line.back() == ']') {
            // Save previous domain config if we were in a domain section
            if (in_domain_section && !current_domain_config.domain_name.empty()) {
                domain_configs[current_domain_config.domain_name] = current_domain_config;
                current_domain_config = DomainConfig(); // Reset
            }
            
            current_section = line.substr(1, line.length() - 2);
            in_domain_section = (current_section.find("domain:") == 0);
            
            if (in_domain_section) {
                current_domain_config.domain_name = current_section.substr(7); // Remove "domain:"
            }
            continue;
        }
        
        // Parse key-value pairs
        size_t equals_pos = line.find('=');
        if (equals_pos == std::string::npos) {
            continue;
        }
        
        std::string key = line.substr(0, equals_pos);
        std::string value = line.substr(equals_pos + 1);
        
        // Remove whitespace from key and value
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        
        if (in_domain_section) {
            // Parse domain-specific settings
            if (key == "domain_controller") {
                current_domain_config.domain_controller = value;
            } else if (key == "service_account") {
                current_domain_config.service_account = value;
            } else if (key == "connection_timeout") {
                current_domain_config.connection_timeout = std::stoi(value);
            } else if (key == "use_ssl") {
                current_domain_config.use_ssl = (value == "true" || value == "1");
            } else if (key == "data_sources") {
                // Parse comma-separated list
                std::stringstream ss(value);
                std::string item;
                while (std::getline(ss, item, ',')) {
                    item.erase(0, item.find_first_not_of(" \t"));
                    item.erase(item.find_last_not_of(" \t") + 1);
                    if (!item.empty()) {
                        current_domain_config.data_sources.push_back(item);
                    }
                }
            }
        } else if (current_section == "global") {
            // Parse global settings
            global_settings[key] = value;
        } else if (current_section == "ldap") {
            // Parse LDAP settings
            if (key == "ldap_server") {
                ldap_config.ldap_server = value;
            } else if (key == "ldap_port") {
                ldap_config.ldap_port = std::stoi(value);
            } else if (key == "ldap_use_ssl") {
                ldap_config.ldap_use_ssl = (value == "true" || value == "1");
            } else if (key == "base_dn") {
                ldap_config.base_dn = value;
            } else if (key == "user_search_filter") {
                ldap_config.user_search_filter = value;
            } else if (key == "bind_dn_format") {
                ldap_config.bind_dn_format = value;
            } else if (key == "ldap_timeout") {
                ldap_config.ldap_timeout = std::stoi(value);
            }
        }
    }
    
    // Save the last domain config if we were in a domain section
    if (in_domain_section && !current_domain_config.domain_name.empty()) {
        domain_configs[current_domain_config.domain_name] = current_domain_config;
    }
    
    file.close();
    return true;
}

void ConfigManager::create_default_config() {
    std::cout << "[ConfigManager] Creating default configuration" << std::endl;
    
    // Create default global settings
    global_settings["log_level"] = "INFO";
    global_settings["data_collection_interval"] = "3600";
    global_settings["max_retry_attempts"] = "3";
    global_settings["output_directory"] = "output";
    
    // Create default domain configuration
    DomainConfig default_domain;
    default_domain.domain_name = "COMPANY.LOCAL";
    default_domain.domain_controller = "DC01.COMPANY.LOCAL";
    default_domain.service_account = "billing_service";
    default_domain.connection_timeout = 30;
    default_domain.use_ssl = true;
    default_domain.data_sources = {"WMI", "EventLog", "Registry"};
    
    domain_configs[default_domain.domain_name] = default_domain;
    
    // Create default LDAP configuration
    ldap_config.ldap_server = "DC01.COMPANY.LOCAL";
    ldap_config.ldap_port = 389;
    ldap_config.ldap_use_ssl = false;
    ldap_config.base_dn = "DC=COMPANY,DC=LOCAL";
    ldap_config.user_search_filter = "(sAMAccountName=%s)";
    ldap_config.bind_dn_format = "%s@COMPANY.LOCAL";
    ldap_config.ldap_timeout = 10;
    
    config_loaded = true;
}

bool ConfigManager::save_config() {
    std::cout << "[ConfigManager] Saving configuration to: " << config_file_path << std::endl;
    
    // Create directory if it doesn't exist
    std::filesystem::path config_path(config_file_path);
    std::filesystem::create_directories(config_path.parent_path());
    
    std::ofstream file(config_file_path);
    if (!file.is_open()) {
        std::cout << "[ConfigManager] Error: Could not open config file for writing" << std::endl;
        return false;
    }
    
    // Write header comment
    file << "# Windows Billing Client Configuration File\n";
    file << "# Generated automatically - modify with care\n\n";
    
    // Write global settings
    file << "[global]\n";
    for (const auto& setting : global_settings) {
        file << setting.first << " = " << setting.second << "\n";
    }
    file << "\n";
    
    // Write domain configurations
    for (const auto& domain_pair : domain_configs) {
        const DomainConfig& config = domain_pair.second;
        file << "[domain:" << config.domain_name << "]\n";
        file << "domain_controller = " << config.domain_controller << "\n";
        file << "service_account = " << config.service_account << "\n";
        file << "connection_timeout = " << config.connection_timeout << "\n";
        file << "use_ssl = " << (config.use_ssl ? "true" : "false") << "\n";
        
        // Write data sources as comma-separated list
        file << "data_sources = ";
        for (size_t i = 0; i < config.data_sources.size(); i++) {
            if (i > 0) file << ", ";
            file << config.data_sources[i];
        }
        file << "\n\n";
    }
    
    file.close();
    std::cout << "[ConfigManager] Configuration saved successfully" << std::endl;
    return true;
}

DomainConfig ConfigManager::get_domain_config(const std::string& domain_name) const {
    auto it = domain_configs.find(domain_name);
    if (it != domain_configs.end()) {
        return it->second;
    }
    
    std::cout << "[ConfigManager] Warning: Domain config not found for: " << domain_name << std::endl;
    return DomainConfig(); // Return empty config
}

void ConfigManager::set_domain_config(const DomainConfig& config) {
    domain_configs[config.domain_name] = config;
    std::cout << "[ConfigManager] Domain config updated for: " << config.domain_name << std::endl;
}

std::vector<std::string> ConfigManager::get_configured_domains() const {
    std::vector<std::string> domains;
    for (const auto& pair : domain_configs) {
        domains.push_back(pair.first);
    }
    return domains;
}

std::string ConfigManager::get_global_setting(const std::string& key, 
                                             const std::string& default_value) const {
    auto it = global_settings.find(key);
    if (it != global_settings.end()) {
        return it->second;
    }
    return default_value;
}

void ConfigManager::set_global_setting(const std::string& key, const std::string& value) {
    global_settings[key] = value;
    std::cout << "[ConfigManager] Global setting updated: " << key << " = " << value << std::endl;
}

bool ConfigManager::is_config_loaded() const {
    return config_loaded;
}

LdapConfig ConfigManager::get_ldap_config() const {
    return ldap_config;
}
