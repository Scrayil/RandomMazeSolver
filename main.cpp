//
// Created by scrayil on 13/05/23.
//

#include <iostream>
#include <json.hpp>
#include <random>
#include <chrono>
#include <thread>

#include "utils/utils.h"
#include "sequential/sequential_version.h"
#include "parallel/parallel_version.h"


// TODO: Good seeds:
//"generation_seed": 3199718447,
//"solution_seed": 3896320784,


// PROTOTYPES
std::mt19937 evaluate_seed(long seed, const std::string& operation);
void process_size(int &size, std::mt19937 &rng);


// GLOBAL VARIABLES
int SIDE_MAX = 701; // A value higher that this one results into a sigkill from the os on the current machine
int SIDE_MIN = 51;


// FUNCTIONS
int main() {
    // Retrieving the project's folder
    std::filesystem::path project_folder = find_project_path();
    if(project_folder.empty()) {
        std::cout << "Unable to locate the project's folder!";
        exit(1);
    } else {
        std::cout << "Project's path: " << project_folder << std::endl;
    }

    // Load the settings
    nlohmann::json config = parse_configuration(project_folder);

    // Retrieves the number of executions to perform
    int n_executions = config["n_executions"];

    // Retrieves the specified number of particles to generate
    int n_particles = config["n_particles"];

    // Reads the specified size and try to use it to generate a corresponding maze
    int size = 0;
    if(config.contains("maze_size")) {
        size = config["maze_size"];
    }

    // Checks if intermediate steps must be shown or not
    bool show_steps = false;
    if(config.contains("show_steps")) {
        show_steps = config["show_steps"];
    }

    // Search for a specific seed in the configuration
    // This allows to generate the same maze everytime. (other parameters like size, must remain the same too)
    long generation_seed = -1;
    if(config.contains("generation_seed")) {
        generation_seed = config["generation_seed"];
    }

    // Search for a specific seed in the configuration
    // This allows to generate the same particles movements everytime.
    long solution_seed = -1;
    if(config.contains("solution_seed")) {
        solution_seed = config["solution_seed"];
    }

    // Tests the 2 versions non-stop with the configuration seeds if given. Otherwise, a new pair of seeds is generated
    // at each iteration. But both the versions will share the seeds everytime so that the generated maze and solution
    // moves are the same.
    for(int i = 0; i < n_executions; i++) {
        system("clear");
        // Evaluating seeds
        std::mt19937 generation_rng = evaluate_seed(generation_seed, "generation");
        std::mt19937 solution_rng = evaluate_seed(solution_seed, "solution");

        // Ensures the maze has an odd size and checks if the size is withing the allowed range
        process_size(size, generation_rng);
        std::cout << "Maze Size: [" << size << ", " << size << "]" << std::endl;

        // SEQUENTIAL VERSION
        std::cout << "\n\nSEQUENTIAL VERSION:" << std::endl;
        auto start_ts = std::chrono::high_resolution_clock::now();
        sequential_solution(size, n_particles, generation_rng, solution_rng, show_steps);
        auto end_ts = std::chrono::high_resolution_clock::now();
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "The execution took " << duration_cast<std::chrono::microseconds>(end_ts - start_ts).count() / 1000.f
        << " ms" << std::endl;

        std::cout << "-----------------------------------------------------------" << std::endl;

        // PARALLEL VERSION
        std::cout << "\n\nPARALLEL VERSION:" << std::endl;
        start_ts = std::chrono::high_resolution_clock::now();
        parallel_solution(size, n_particles, generation_rng, solution_rng, show_steps);
        end_ts = std::chrono::high_resolution_clock::now();
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "The execution took " << duration_cast<std::chrono::microseconds>(end_ts - start_ts).count() / 1000.f
                  << " ms" << std::endl;

        // Sleeps for 3 seconds beofore continuing with the next test
        std::cout<< "Sleeping for 7 seconds.." << std::endl;
//        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        std::this_thread::sleep_for(std::chrono::milliseconds(7000));
    }

    return 0;
}


std::mt19937 evaluate_seed(long seed, const std::string& operation) {
    // If the seed has not been set or is equal to -1
    // Generates a random seed with /dev/random
    if(seed == -1) {
        std::random_device rd;
        seed = rd();
    }

    std::cout << "Current seed for " << operation << ": " << seed << std::endl;

    // Used to set a new seed everytime
    std::mt19937 rng(seed); // Random-number engine used (Mersenne-Twister in this case)
    return rng;
}


/**
 * Checks and enforces that the given size respect all the constraints,
 *
 *  The function checks if the size is in between the specified limits and ensures that it corresponds to an odd number.
 *  In case it doesn't a new random odd size is picked.
 *
 *  @param size Represents the length of each maze's side.
 *  @param rng This is the random number engine to use in order to generate random values.
 */
void process_size(int &size, std::mt19937 &rng) {
    // Ensures the maze has an odd size and checks if the size is withing the allowed range
    if(size % 2 == 0 || size < SIDE_MIN || size > SIDE_MAX) {
        std::cout << "Invalid maze size specified. The values must be in [" << SIDE_MIN << ", " << SIDE_MAX << "]" << std::endl;
        // Creates two uniform distributions based on a range and uses it to assign the random integers values
        // The value is based onto a random device "/dev/random".
        // This way a new unpredictable seed is used everytime
        std::uniform_int_distribution<int> uniform_dist(SIDE_MIN, SIDE_MAX - 1); // Guaranteed unbiased
        size = uniform_dist(rng);
        // The maze must have an odd size
        while(size % 2 == 0)
            size = uniform_dist(rng);
    }
}