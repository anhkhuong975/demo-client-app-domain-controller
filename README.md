# Windows Billing Client Demo

A C++ demonstration application that simulates a Windows Domain billing client for collecting working hours data across multiple domains.

## Overview

This application demonstrates the architecture and workflow of a Windows billing client that can:
- Connect to multiple Windows Domains
- Authenticate with domain controllers
- Collect working hours data from domain users
- Generate billing reports
- Export data for further processing

## Architecture

The application is structured with clear separation of concerns:

```
src/
├── include/
│   ├── domain_manager.h      # Windows Domain authentication and connection
│   ├── data_collector.h      # Working hours data collection
│   ├── config_manager.h      # Configuration file management
│   └── billing_service.h     # Billing report generation
├── main.cpp                  # Main application with interactive menu
├── domain_manager.cpp        # Domain authentication implementation
├── data_collector.cpp        # Data collection with mock data generation
├── config_manager.cpp        # Configuration parsing and management
└── billing_service.cpp       # Billing calculations and reporting
```

## Features

### 1. Domain Management
- **Authentication**: Simulates Windows Domain authentication
- **Multi-Domain Support**: Configurable for multiple domains
- **User Enumeration**: Mock domain user discovery

### 2. Data Collection
- **Working Hours**: Collects employee working hours data
- **Mock Data Generation**: Creates realistic sample data for demo
- **Department Tracking**: Organizes data by departments and projects

### 3. Configuration Management
- **File-Based Config**: Uses configuration files for domain settings
- **Global Settings**: Application-wide configuration options
- **Dynamic Loading**: Runtime configuration loading and validation

### 4. Billing Service
- **Report Generation**: Creates detailed billing reports
- **Cost Calculation**: Calculates costs based on hourly rates
- **Export Functionality**: Exports reports to text and CSV formats

## Building the Application

### Prerequisites
- CMake 3.10 or higher
- C++17 compatible compiler
- Windows environment (for full functionality)

### Build Steps
```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build the application
cmake --build .
```

## Running the Demo

### 1. Start the Application
```bash
./billing_client
```

### 2. Interactive Menu
The application provides an interactive menu with the following options:

1. **Load Configuration** - Loads domain and global settings
2. **Connect to Domain** - Authenticates with a Windows Domain
3. **Collect Working Hours Data** - Gathers employee working hours
4. **Generate Billing Report** - Creates billing reports with cost calculations
5. **Export Data to CSV** - Exports raw data for external processing
6. **View Configuration** - Displays current configuration settings
7. **Exit** - Terminates the application

### 3. Demo Workflow
1. Load configuration (creates default config if none exists)
2. Connect to domain (use any username/password for demo)
3. Collect working hours data (generates mock data)
4. Generate billing report (calculates costs and creates report)
5. Export data to CSV (saves data for external use)

## Configuration

The application uses a configuration file at `config/billing_client.conf`:

```ini
[global]
log_level = INFO
data_collection_interval = 3600
max_retry_attempts = 3
output_directory = output

[domain:COMPANY.LOCAL]
domain_controller = DC01.COMPANY.LOCAL
service_account = billing_service
connection_timeout = 30
use_ssl = true
data_sources = WMI, EventLog, Registry
```

## VirtualBox Demo Setup

### Windows Server Setup
1. Install Windows Server 2019/2022
2. Configure Active Directory Domain Services
3. Create domain: `COMPANY.LOCAL`
4. Add test users and computers

### Windows 10 Pro Client Setup
1. Install Windows 10 Pro
2. Join the domain `COMPANY.LOCAL`
3. Build and run the billing client application

### Network Configuration
- Ensure both VMs can communicate
- Configure DNS to point to domain controller
- Open necessary firewall ports (389, 636, 88, 53)

## Mock Data

The application generates realistic mock data including:
- **Employee IDs**: EMP001, EMP002, etc.
- **Departments**: IT, Finance, HR, Marketing, Operations
- **Working Hours**: Random hours between 6-10 per day
- **Project Codes**: PROJ001-PROJ007
- **Dates**: Current date for all entries

## Output Files

The application generates several output files:
- `output/billing_report_[domain].txt` - Detailed billing report
- `output/working_hours_[domain].csv` - Raw data in CSV format
- `config/billing_client.conf` - Configuration file

## Real-World Implementation Notes

In a production environment, this application would need:

### Security Enhancements
- Secure credential storage (Windows Credential Manager)
- Certificate-based authentication
- Encrypted configuration files
- Audit logging

### Windows API Integration
- **NetAPI32**: For domain user enumeration
- **ADSI/LDAP**: For Active Directory queries
- **WMI**: For system information collection
- **Event Log APIs**: For logon/logoff tracking

### Performance Optimizations
- Connection pooling
- Async data collection
- Caching mechanisms
- Batch processing

### Error Handling
- Network timeout handling
- Domain controller failover
- Data validation and sanitization
- Comprehensive logging

## Troubleshooting

### Common Issues
1. **Build Errors**: Ensure C++17 compiler and CMake are properly installed
2. **Configuration Not Found**: Run the application once to generate default config
3. **Output Directory**: Ensure write permissions to output directory

### Debug Mode
Enable debug logging by setting `log_level = DEBUG` in configuration file.

## License

This is a demonstration application for educational purposes.
