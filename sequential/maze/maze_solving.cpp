//
// Created by scrayil on 12/05/23.
//

#include <random>
#include <iostream>
#include "maze_solving.h"


// ENUM AND STRUCTS
enum MOVES {
    F = 0, // Used just for particles' initializations (freeze)
    N = 1,
    E = 2,
    S = 3,
    W = 4,
};


struct Coordinates {
    Coordinates(int c_x, int c_y) : row(c_x), col(c_y) {}

    int row;
    int col;
};


struct Particle {
    // F is used just for particles' initializations (freeze)
    explicit Particle(Coordinates coords) : pos(coords), move(MOVES::F) {
        this->path.push_back(coords);
    }

    void update_coordinates(MOVES new_move) {
//        Coordinates prev_coords = this->pos;

        switch (new_move) {
            case MOVES::N:
                this->pos = Coordinates(this->pos.row - 1, this->pos.col);
                break;
            case MOVES::S:
                this->pos = Coordinates(this->pos.row + 1, this->pos.col);
                break;
            case MOVES::E:
                this->pos = Coordinates(this->pos.row, this->pos.col + 1);
                break;
            case MOVES::W:
                this->pos = Coordinates(this->pos.row, this->pos.col - 1);
                break;
            case MOVES::F:
                std::cout<< "Unexpected error: the F move is meant only for particles' initialization!" << std::endl;
                exit(1);
        }

        // Going back reverts the path. Case in which the new position set is the last but one in the track!
        // This is extremely useful as it frees up memory that would be wastes as the contained coordinates are
        // not relevant for the maze's solution
        int path_size = static_cast<int>(this->path.size());

        // The starting position is always kept
        if(path_size > 1 && this->pos.row == this->path[path_size - 2].row && this->pos.col == this->path[path_size - 2].col)
            this->path.pop_back();
        else
            this->path.push_back(this->pos);
        this->move = new_move;
    }

    Coordinates pos;
    std::vector<Coordinates> path;
    MOVES move;
};


// PROTOTYPES
void reach_exit_randomly(std::vector<std::vector<MAZE_PATH>> &maze, int &size, Coordinates &initial_position, std::vector<Particle> &particles, std::mt19937 &rng, bool show_steps);
std::vector<MOVES> get_possible_moves(std::vector<std::vector<MAZE_PATH>> &maze, int &size, Particle &curr_particle);
void backtrack_exited_particle(std::vector<std::vector<MAZE_PATH>> &maze, std::vector<std::vector<MAZE_PATH>> &maze_copy, std::vector<Particle> &particles, std::vector<Coordinates> exited_particle_path, bool show_steps);


// FUNCTIONS
void solve(std::vector<std::vector<MAZE_PATH>> maze, int size, int n_particles, std::mt19937 solution_rng, bool show_steps) {
    std::cout << "SOLVING THE MAZE.." << std::endl;

    // Choosing a random starting position
    std::uniform_int_distribution<int> uniform_dist(size / 6, 5 * size / 6); // Guaranteed unbiased

    // Selects an initial position
    Coordinates initial_position = Coordinates(uniform_dist(solution_rng), uniform_dist(solution_rng));
    while(maze[initial_position.row][initial_position.col] != MAZE_PATH::EMPTY)
        initial_position = Coordinates(uniform_dist(solution_rng), uniform_dist(solution_rng));

    // AoS
    std::vector<Particle> particles;
    particles.reserve(n_particles);

    for(int i = 0; i < n_particles; i++) {
        Particle particle = Particle(initial_position);
        particles.push_back(particle);
    }

    // Initial point always shown
    // Adds the particles to the maze copy in order to show them
    std::vector<std::vector<MAZE_PATH>> maze_copy = maze;
    maze_copy[initial_position.row][initial_position.col] = MAZE_PATH::START;
    display_ascii_maze(maze_copy, size);

    // Starts the solving procedure
    reach_exit_randomly(maze, size, initial_position, particles, solution_rng, show_steps);
}

void reach_exit_randomly(std::vector<std::vector<MAZE_PATH>> &maze, int &size, Coordinates &initial_position, std::vector<Particle> &particles, std::mt19937 &rng, bool show_steps) {
    bool exit_reached = false;
    int exited_particle_index = -1;
    std::vector<std::vector<MAZE_PATH>> maze_copy;
    maze_copy.clear();

    while(!exit_reached) {
        if(show_steps)
            // Copies the maze to show the particles positions
            maze_copy = maze;

        for(int index = 0; index < particles.size(); index++) {
            Particle curr_particle = particles[index];

            std::vector<MOVES> moves = get_possible_moves(maze, size, curr_particle);
            bool same_move_allowed = false;
            for(MOVES move : moves) {
                // Keeps going on if it can go only on opposite directions
                if(moves.size() == 2 && curr_particle.move == move) {
                    same_move_allowed = true;
                    curr_particle.update_coordinates(move);
                    break;
                }
            }
            // The same move wasn't available because of the walls nearby
            if(!same_move_allowed) {
                // Choosing a random move
                std::uniform_int_distribution<int> uniform_dist(0, static_cast<int>(moves.size()) - 1); // Guaranteed unbiased
                MOVES new_move = moves[uniform_dist(rng)];
                curr_particle.update_coordinates(new_move);
            }

            // Updates the particle in the vector
            particles[index] = curr_particle;

            if(show_steps) {
                // Add the particles to the maze_copy
                maze_copy[curr_particle.pos.row][curr_particle.pos.col] = MAZE_PATH::PARTICLE;
                // Shows the start everytime
                maze_copy[initial_position.row][initial_position.col] = MAZE_PATH::START;
            }

            // The particle has reached the exit
            if(maze[curr_particle.pos.row][curr_particle.pos.col] == MAZE_PATH::EXIT) {
                exited_particle_index = index;
                exit_reached = true;
                break;
            }
        }

        if(show_steps && !maze_copy.empty())
            // Displays the particles in the maze
            display_ascii_maze(maze_copy, size);

    }

    // Backtracking the first particle that went out
    std::vector<Coordinates> exited_particle_path = particles[exited_particle_index].path;
//    backtrack_exited_particle(maze, maze_copy, particles, exited_particle_path, show_steps);

    std::cout << "EXIT REACHED!!" << std::endl;
    // Shows the maze's path that lead to the solution
    for(Coordinates coord : exited_particle_path) {
        maze[coord.row][coord.col] = MAZE_PATH::SOLUTION;
    }
    maze[initial_position.row][initial_position.col] = MAZE_PATH::START;

    display_ascii_maze(maze, size);
}


std::vector<MOVES> get_possible_moves(std::vector<std::vector<MAZE_PATH>> &maze, int &size, Particle &curr_particle) {
    std::vector<MOVES> moves;
    moves.clear();

    if(-1 < curr_particle.pos.row - 1 < size && maze[curr_particle.pos.row - 1][curr_particle.pos.col] != MAZE_PATH::WALL)
        moves.push_back(MOVES::N);

    if(-1 < curr_particle.pos.row + 1 < size && maze[curr_particle.pos.row + 1][curr_particle.pos.col] != MAZE_PATH::WALL)
        moves.push_back(MOVES::S);

    if(-1 < curr_particle.pos.col - 1 < size && maze[curr_particle.pos.row][curr_particle.pos.col - 1] != MAZE_PATH::WALL)
        moves.push_back(MOVES::W);

    if(-1 < curr_particle.pos.col + 1 < size && maze[curr_particle.pos.row][curr_particle.pos.col + 1] != MAZE_PATH::WALL)
        moves.push_back(MOVES::E);

    return moves;
}


void backtrack_exited_particle(std::vector<std::vector<MAZE_PATH>> &maze, std::vector<std::vector<MAZE_PATH>> &maze_copy, std::vector<Particle> &particles, std::vector<Coordinates> exited_particle_path, bool show_steps) {
    std::vector<bool> particles_on_track_map;
    std::fill(particles_on_track_map.begin(), particles_on_track_map.end(), false);

    // Backtracking the particles movements until they are on the solution path
    // After that they follow the first exited particle's movements
    for(int particle_index = 0; particle_index < particles.size(); particle_index++) {
        Particle particle = particles[particle_index];
        // If the particle wasn't on the solution path in the previous iteration checks if it is now
        if(!particles_on_track_map[particle_index]) {
            for(int path_index = 0; path_index < exited_particle_path.size(); path_index++) {
                Coordinates coord = exited_particle_path[path_index];
                // The particle is now onto the right track
                if(particle.pos.row == coord.row && particle.pos.col == coord.col) {
                    particles_on_track_map[particle_index] = true;
                    break;
                }
            }
        } else {

        }

        if(show_steps) {

        }
    }
}