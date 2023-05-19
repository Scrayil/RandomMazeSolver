//
// Created by scrayil on 13/05/23.
//

#include <iostream>
#include <json.hpp>
#include <random>
#include <chrono>
#include <fstream>

#include "utils/utils.h"
#include "sequential/sequential_version.h"
#include "parallel/parallel_version.h"
#include "sequential/maze/maze_generation.h"


// PROTOTYPES
std::mt19937 evaluate_seed(long seed, long &processed_seed, const std::string& operation);
void process_size(int &size, std::mt19937 &rng);
void save_results(std::filesystem::path &project_folder, bool is_sequential, long &generation_seed, long &solution_seed, float &elapsed_milliseconds, int n_particles, std::vector<std::vector<MAZE_PATH>> &maze, int &size);
std::filesystem::path save_maze_image(std::filesystem::path &image_path, std::string &version, std::vector<std::vector<MAZE_PATH>> &maze, int &size);

// GLOBAL VARIABLES
int SIDE_MAX = 301;
int SIDE_MIN = 51;


// FUNCTIONS
int main() {
    std::cout << std::endl << "[ Random Maze Generator and Solver ]" << std::endl << std::endl;

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

    // Initializing variables for timing checks
    std::chrono::high_resolution_clock::time_point start_ts;
    std::chrono::high_resolution_clock::time_point end_ts;
    float elapsed_milliseconds;

    // Initializes the variables that will hold the processed seeds
    // Used to not alter the original seeds
    long final_generation_seed;
    long final_solution_seed;

    // Tests the 2 versions non-stop with the configuration seeds if given. Otherwise, a new pair of seeds is generated
    // at each iteration. But both the versions will share the seeds everytime so that the generated maze and solution
    // moves are the same.
    for(int i = 0; i < n_executions; i++) {
        // Evaluating seeds
        std::mt19937 generation_rng = evaluate_seed(generation_seed, final_generation_seed, "generation");
        std::mt19937 solution_rng = evaluate_seed(solution_seed, final_solution_seed, "solution");

        // Ensures the maze has an odd size and checks if the size is withing the allowed range
        process_size(size, generation_rng);
        std::cout << "Maze Size: [" << size << ", " << size << "]" << std::endl;

        // Creates the maze matrix
        std::vector<std::vector<MAZE_PATH>> maze;
        maze.resize(size);
        maze.reserve(size);
        std::vector<std::vector<MAZE_PATH>> maze_with_solution;

        // SEQUENTIAL VERSION
        if(config["execute_sequential"]) {
            std::cout << "\n\nSEQUENTIAL VERSION:\n" << std::endl;
            start_ts = std::chrono::high_resolution_clock::now();
            maze_with_solution = sequential_solution(maze,size, n_particles, generation_rng, solution_rng, show_steps);
            end_ts = std::chrono::high_resolution_clock::now();
            elapsed_milliseconds = duration_cast<std::chrono::microseconds>(end_ts-start_ts).count() / 1000.f;
            std::cout << std::fixed << std::setprecision(3);
            std::cout << "The execution took " << elapsed_milliseconds << " ms" << std::endl;

            save_results(project_folder, true, final_generation_seed, final_solution_seed, elapsed_milliseconds, n_particles, maze_with_solution, size);

            std::cout << "-----------------------------------------------------------" << std::endl;
        }

        // PARALLEL VERSION
        if(config["execute_parallel"]) {
            std::cout << "\n\nPARALLEL VERSION:\n" << std::endl;
            start_ts = std::chrono::high_resolution_clock::now();
            maze_with_solution = parallel_solution(maze, size, n_particles, generation_rng, solution_rng, show_steps);
            end_ts = std::chrono::high_resolution_clock::now();
            elapsed_milliseconds = duration_cast<std::chrono::microseconds>(end_ts-start_ts).count() / 1000.f;
            std::cout << std::fixed << std::setprecision(3);
            std::cout << "The execution took " << elapsed_milliseconds << " ms" << std::endl;

            save_results(project_folder, false, final_generation_seed, final_solution_seed, elapsed_milliseconds, n_particles, maze_with_solution, size);
        }

        std::cout << "###########################################################" << std::endl;
    }

    return 0;
}


/**
 * This function is used to initialize the random-number engine.
 *
 * The function is used to set the specified seed value and initialize the engine.
 * If the seed has not been specified, a new random seed is generated with "/dev/random"
 * @param seed Allows to specify the seed to use if set.
 * @param operation This is the string used in order to print the proper seed category on screen.
 * @return The initialized random number engine to use for random values generation.
 */
std::mt19937 evaluate_seed(long seed, long &processed_seed, const std::string& operation) {
    // If the seed has not been set or is equal to -1
    // Generates a random seed with /dev/random
    if(seed == -1) {
        std::random_device rd;
        processed_seed = rd();
    }

    std::cout << "Current seed for " << operation << ": " << processed_seed << std::endl;

    // Used to set a new seed everytime
    std::mt19937 rng(processed_seed); // Random-number engine used (Mersenne-Twister in this case)
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


/**
 * Saves the juicy information related to the execution of the project.
 *
 * This function helps to keep track of the records and the measurements taken in order to report and confront them
 * later on.
 * @param project_folder This is the root path of the current project.
 * @param is_sequential Flag used to tell if the current reported version is sequential or parallel.
 * @param generation_seed This is the seed that has been used for the maze's generation.
 * @param solution_seed This is the seed that has been used for the maze's solution.
 * @param elapsed_milliseconds This is the total elapsed milliseconds required to generate and solve the maze.
 * @param maze This is the matrix that represents the maze's inner structure along with the solution path.
 * @param size This value represents each maze's side size.
 */
void save_results(std::filesystem::path &project_folder, bool is_sequential, long &generation_seed, long &solution_seed, float &elapsed_milliseconds, int n_particles, std::vector<std::vector<MAZE_PATH>> &maze, int &size) {
    std::cout << "Saving the results.." << std::endl;

    std::string version = is_sequential ? "sequential" : "parallel";

    std::filesystem::path images_path = project_folder / "results" / "mazes";

    // Creating the directories if they don't exist
    if(!std::filesystem::is_directory(images_path) || !std::filesystem::exists(images_path))
        std::filesystem::create_directories(images_path);

    // Saving the maze's image
    std::filesystem::path maze_image_path = save_maze_image(images_path, version, maze, size);

    // Writing/appending to the report file
    std::filesystem::path report_path = project_folder / "results" / "executions_report.csv";
    std::ofstream report_file;
    if(!std::filesystem::exists(report_path)) {
        report_file.open(report_path.c_str(), std::fstream::app);
        report_file << "version,elapsed_time,maze_size,n_particles,generation_seed,solution_seed,maze_image_path";
    } else {
        report_file.open(report_path.c_str(), std::fstream::app);
    }

    // Saves the current record
    report_file << "\n" << version << "," << elapsed_milliseconds << "," << size << "," << n_particles << "," << generation_seed << "," << solution_seed << "," << maze_image_path;

    // Closing the file
    report_file.close();
}


/**
 * Saves the current maze's ascii image to the disk.
 *
 * This function generates a file containing the maze's structure rendered with ascii character in order to
 * be able to visualize it later with it's solution path.
 * @param image_path This is the base location for maze's images.
 * @param version This string is used to tell if the current maze belongs to a sequential or parallel version.
 * @param maze This is the matrix that represents the maze's inner structure along with the solution path.
 * @param size This value represents each maze's side size.
 * @return `image_path`
 */
std::filesystem::path save_maze_image(std::filesystem::path &image_path, std::string &version, std::vector<std::vector<MAZE_PATH>> &maze, int &size) {
    // Building the unique image path
    std::time_t now = std::chrono::high_resolution_clock::to_time_t(std::chrono::high_resolution_clock::now());
    char buf[256] = { 0 };
    // ISO 8601 format for the timestamp
    std::strftime(buf, sizeof(buf), "%y-%m-%dT%H:%M:%S.%f", std::localtime(&now));
    image_path = image_path / (version + "_" + std::string(buf) + ".txt");

    // Generating the ascii maze image
    std::string ascii_maze = generate_ascii_maze(maze, size);

    // Saving the image to the disk
    std::ofstream output_image(image_path.c_str());
    output_image << ascii_maze;
    output_image.close();

    return image_path;
}