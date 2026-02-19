#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;

int main() {
    // Create "blocky" directory
    try {
        if (!fs::exists("blocky")) {
            if (!fs::create_directory("blocky")) {
                std::cerr << "Failed to create directory 'blocky'\n";
                return 1;
            }
            std::cout << "Directory 'blocky' created.\n";
        } else {
            std::cout << "Directory 'blocky' already exists.\n";
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << '\n';
        return 1;
    }

    // Change working directory to ./blocky
    try {
        fs::current_path("blocky"); // Changes current working directory
        std::cout << "Changed working directory to: " << fs::current_path() << '\n';
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Could not change directory to 'blocky': " << e.what() << '\n';
        return 1;
    }

    // Get version
    std::string version;
    std::cout << "Enter blocky version to install (with v): ";
    std::getline(std::cin, version);

    if (version.empty() || version.find(' ') != std::string::npos) {
        std::cerr << "Invalid version format.\n";
        return 1;
    }

    std::string filename = "blocky_" + version + "_Linux_x86_64.tar.gz";
    std::string download_cmd = "wget https://github.com/0xERR0R/blocky/releases/latest/download/" + filename;

    std::cout << "Downloading...\n";
    if (std::system(download_cmd.c_str()) != 0) {
        std::cerr << "Download failed! Check version name or install wget.\n";
        return 1;
    }

    std::cout << "Extracting...\n";
    std::string extract_cmd = "tar -xzf " + filename;
    if (std::system(extract_cmd.c_str()) != 0) {
        std::cerr << "Extraction failed! Is tar installed?\n";
        return 1;
    }

    // Cleanup archive
    std::string remove_cmd = "rm " + filename;
    std::system(remove_cmd.c_str()); // Best-effort cleanup

    // Write config.yml
    std::ofstream file("config.yml");
    if (!file.is_open()) {
        std::cerr << "Could not create config.yml\n";
        return 1;
    }

    std::string upstream;
    std::cout << "Enter your upstream DNS server: ";
    std::getline(std::cin, upstream);

    file << "upstream:\n"
         << " default:\n"
         << " - " << upstream << "\n"
         << "blocking:\n"
         << " blackLists:\n"
         << "  ads:\n"
         << "  - https://raw.githubusercontent.com/StevenBlack/hosts/master/hosts\n"
         << "ports:\n"
         << " dns: 53\n"
         << " http: 4000\n";
    file.close();
    std::cout << "Configuration saved to config.yml\n";

    // Set capabilities (requires sudo)
    std::cout << "Setting capabilities (binding to port 53 requires root privilege)...\n";
    if (std::system("sudo setcap 'cap_net_bind_service=+ep' ./blocky") != 0) {
        std::cerr << "Failed to set capabilities. You may need to run blocky with sudo.\n";
        std::cerr << "Alternative: sudo ./blocky --config config.yml\n";
    }

    // Run blocky
    std::cout << "Starting blocky...\n";
    int result = std::system("./blocky --config config.yml");
    if (result != 0) {
        std::cerr << "Blocky failed to start.\n";
    }

    return result;
}   
