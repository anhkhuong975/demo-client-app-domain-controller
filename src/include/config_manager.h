#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <map>
#include <vector>

/**
 * @brief Structure to hold domain configuration
 */
struct DomainConfig {
    std::string domain_name;
    std::string domain_controller;
    std::string service_account;
    int connection_timeout;
    bool use_ssl;
    std::vector<std::string> data_sources;
};

/**
 * @brief Structure to hold LDAP authentication configuration
 */
struct LdapConfig {
    std::string ldap_server;
    int ldap_port;
    bool ldap_use_ssl;
    std::string base_dn;
    std::string user_search_filter;
    std::string bind_dn_format;
    int ldap_timeout;
};

/**
 * @brief Manages application configuration and domain settings
 * 
 * This class handles loading and saving configuration files,
 * managing domain-specific settings, and providing configuration
 * data to other components of the application.
 */
class ConfigManager {
private:
    std::string config_file_path;
    std::map<std::string, DomainConfig> domain_configs;
    std::map<std::string, std::string> global_settings;
    LdapConfig ldap_config;
    bool config_loaded;
    
    /**
     * @brief Parse configuration file and load settings
     * @return true if parsing successful, false otherwise
     */
    bool parse_config_file();
    
    /**
     * @brief Create default configuration if none exists
     */
    void create_default_config();
    
public:
    /**
     * @brief Constructor for ConfigManager
     * @param config_path Path to configuration file
     */
    ConfigManager(const std::string& config_path = "config/billing_client.conf");
    
    /**
     * @brief Destructor for ConfigManager
     */
    ~ConfigManager();
    
    /**
     * @brief Load configuration from file
     * @return true if loading successful, false otherwise
     */
    bool load_config();
    
    /**
     * @brief Save current configuration to file
     * @return true if saving successful, false otherwise
     */
    bool save_config();
    
    /**
     * @brief Get domain configuration by name
     * @param domain_name Name of the domain
     * @return DomainConfig structure or empty config if not found
     */
    DomainConfig get_domain_config(const std::string& domain_name) const;
    
    /**
     * @brief Add or update domain configuration
     * @param config DomainConfig to add/update
     */
    void set_domain_config(const DomainConfig& config);
    
    /**
     * @brief Get list of configured domains
     * @return Vector of domain names
     */
    std::vector<std::string> get_configured_domains() const;
    
    /**
     * @brief Get global setting value
     * @param key Setting key
     * @param default_value Default value if key not found
     * @return Setting value or default value
     */
    std::string get_global_setting(const std::string& key, 
                                  const std::string& default_value = "") const;
    
    /**
     * @brief Set global setting
     * @param key Setting key
     * @param value Setting value
     */
    void set_global_setting(const std::string& key, const std::string& value);
    
    /**
     * @brief Check if configuration is loaded
     * @return true if config loaded, false otherwise
     */
    bool is_config_loaded() const;
    
    /**
     * @brief Get LDAP configuration
     * @return LdapConfig structure
     */
    LdapConfig get_ldap_config() const;
};

#endif // CONFIG_MANAGER_H
