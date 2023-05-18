//
// Created by scrayil on 08/05/23.
//

#ifndef RANDOMMAZESOLVER_UTILS_H
#define RANDOMMAZESOLVER_UTILS_H

#include <json.hpp>

#include "../sequential/maze/maze_generation.h"


nlohmann::json parse_configuration(const std::filesystem::path& project_folder);
std::filesystem::path find_project_path();
void display_ascii_maze(std::vector<std::vector<MAZE_PATH>> &maze, int &size);
std::string generate_ascii_maze(std::vector<std::vector<MAZE_PATH>> &maze, int &size);

#endif //RANDOMMAZESOLVER_UTILS_H