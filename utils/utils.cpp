//
// Created by scrayil on 08/05/23.
//

#include <json.hpp>
#include <fstream>

// FUNCTIONS
nlohmann::json parse_configuration(const std::filesystem::path& project_folder) {
    std::filesystem::path config_path = project_folder / "config" / "default.json";
    std::ifstream config_file;
    config_file.open(config_path);
    nlohmann::json json_config = nlohmann::json::parse(config_file);
    config_file.close();
    return json_config;
}


std::filesystem::path find_project_path() {
    // Todo: Add subfolders check??
    std::filesystem::path project_folder = std::filesystem::current_path();
    while(!project_folder.string().ends_with("RandomMazeSolver"))
        if(project_folder.string() == "/") {
            project_folder.clear();
            break;
        } else {
            project_folder = project_folder.parent_path();
        }
    return project_folder;
}