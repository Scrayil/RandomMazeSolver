//
// Created by scrayil on 08/05/23.
//

#ifndef RANDOMMAZESOLVER_UTILS_H
#define RANDOMMAZESOLVER_UTILS_H

nlohmann::json parse_configuration(const std::filesystem::path& project_folder);
std::filesystem::path find_project_path();

#endif //RANDOMMAZESOLVER_UTILS_H