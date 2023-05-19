//
// Created by scrayil on 08/05/23.
//

#include <iostream>
#include <iterator>

#include "maze_generation.h"


// PROTOTYPES
std::vector<int> p_get_exit_coords(int &size, std::mt19937 &rng);
void p_initialize_maze(std::vector<std::vector<MAZE_PATH>> &maze, int &size, std::vector<int> exit_coords, bool parallelize);
void p_generate_paths(std::vector<std::vector<MAZE_PATH>> &maze, int &size, std::vector<int> exit_coords, std::mt19937 &rng, bool parallelize);
void p_visit_forward(std::vector<std::vector<MAZE_PATH>> &maze, int &size, int &curr_index, std::vector<int> &curr_cell, std::vector<std::vector<int>> &curr_track, std::vector<std::vector<bool>> &visited_cells, std::mt19937 &rng, bool is_exit, bool parallelize);
std::vector<std::vector<int>> p_get_unvisited_near_cells(std::vector<std::vector<MAZE_PATH>> &maze, std::vector<int> &curr_cell, int &size, std::vector<std::vector<bool>> &visited_cells, int &n_cells, bool is_exit);
void p_backtrack(std::vector<std::vector<MAZE_PATH>> &maze, int &size, int &curr_index, std::vector<int> &curr_cell, std::vector<std::vector<int>> &curr_track, std::vector<std::vector<bool>> &visited_cells,  std::mt19937 &rng, bool parallelize);

// FUNCTIONS

/**
 * Generates a random squared maze.
 *
 *  This function is exported to the other source files in the project as it allows to generate random square mazes
 *  with the given size. If the size is not odd or the relative values is outside the allowed range, it s generated
 *  randomly. A specific seed can be passed to the function in order to generate a specific maze.
 *
 *  @param maze It's the matrix representing the maze that is being generated.
 *  @param size Represents the length of each maze's side.
 *  @param generation_rng This is the random number engine to use in order to generate random values.
 *  @param parallelize Flag used to determine if it useful to parallelize the code inside the current function.
 */
void p_generate_square_maze(std::vector<std::vector<MAZE_PATH>> &maze, int &size, std::mt19937 generation_rng, bool parallelize) {
    std::cout << "Generating the maze.." << std::endl;

    // Selects the exit's coordinates randomly
    std::vector<int> exit_coords = p_get_exit_coords(size, generation_rng);

    // Initialize the maze to 0 places the initial walls and sets the random exit
    p_initialize_maze(maze, size, exit_coords, parallelize);

    // Generates the maze's paths
    p_generate_paths(maze, size, exit_coords, generation_rng, parallelize);
}


/**
 * Randomly selects and marks a cell as exit node.
 *
 *  The function generates the coordinates of a cell located on the edges of the maze. The coordinates will be related
 *  to the maze's exit.
 *
 *  @param size Represents the length of each maze's side.
 *  @param rng This is the random number engine to use in order to generate random values.
 *
 *  @return a vector containing the coordinates of the chosen exit cell.
 */
std::vector<int> p_get_exit_coords(int &size, std::mt19937 &rng) {
    std::vector<int> exit_coords;
    exit_coords.reserve(2);

    // Selects the exit's coordinates randomly
    std::uniform_int_distribution<int> uniform_coord(0, size - 1);
    std::uniform_int_distribution<int> uniform_axes_idx(0, 1);
    int random_coord = uniform_coord(rng);
    while (random_coord % 2 == 0)
        random_coord = uniform_coord(rng);
    int exit_x = 0;
    int exit_y = 0;
    if(uniform_axes_idx(rng) == 1)
        exit_y = random_coord;
    else
        exit_x = random_coord;

    exit_coords.push_back(exit_x);
    exit_coords.push_back(exit_y);
    return exit_coords;
}


/**
 * Generates the initial maze.
 *
 *  The function creates a matrix with the given size as sides' length and initializes the inner values as follows:
 *  Each empty cell is separated by the surrounding ones by a wall. This way a proper grid is generated.
 *  The exit cell is set here too.
 *
 *  @param maze It's the matrix representing the maze that is being generated.
 *  @param size Represents the length of each maze's side.
 *  @param exit_coords This is the random number engine to use in order to generate random values.
 *  @param parallelize Flag used to determine if it useful to parallelize the code inside the current function.
 */
void p_initialize_maze(std::vector<std::vector<MAZE_PATH>> &maze, int &size, std::vector<int> exit_coords, bool parallelize) {
    // Resizes the matrix to avoid sigsev errors
    #pragma omp parallel if(parallelize)
    {
        #pragma omp for
        for (int i = 0; i < maze.size(); ++i) {
            maze[i].resize(size);
        }

        // Initializes the matrix
        #pragma omp for
        for (int row = 0; row < size; row++) {
            for (int col = 0; col < size; col++) {
                // Place walls on even rows and columns to create the grid
                if (row % 2 == 0 || col % 2 == 0) {
                    #pragma omp critical
                    maze[row][col] = MAZE_PATH::WALL;
                }
                    // Set the walkable path
                else {
                    #pragma omp critical
                    maze[row][col] = MAZE_PATH::EMPTY;
                }
            }
        }
    }

    // Placing the exit in the maze
    maze[exit_coords[0]][exit_coords[1]] = MAZE_PATH::EXIT;
}


/**
 * Initializes the maze's inner structure generation.
 *
 * This function creates the visited cells matrix to keep track of which cell in the maze has been visited during
 * the actual generation. Then it starts the generation of the structure.
 *
 *  @param maze It's the matrix representing the maze that is being generated.
 *  @param size Represents the length of each maze's side.
 *  @param exit_coords This is the random number engine to use in order to generate random values.
 *  @param rng This is the random number engine to use in order to generate random values.
 *  @param parallelize Flag used to determine if it useful to parallelize the code inside the current function.
 */
void p_generate_paths(std::vector<std::vector<MAZE_PATH>> &maze, int &size, std::vector<int> exit_coords, std::mt19937 &rng, bool parallelize) {
    std::vector<std::vector<bool>> visited_cells;
    visited_cells.resize(size);
    std::vector<std::vector<int>> curr_track;

    // Initializes the current path tracking and sets the relative index to 0
    int curr_index = 0;
    curr_track.push_back(exit_coords);

//    // Initializes the visited_cells array to false as no cell has been visited yet
//    for(int row = 0; row < size; row++) {
//        std::vector<bool> curr_col;
//        curr_col.resize(size);
//        curr_col.reserve(size);
//        for(int col = 0; col < size; col++)
//        {
//            curr_col[col] = false;
//        }
//        visited_cells[row] = curr_col;
//    }

    // Initializes the visited_cells array to false as no cell has been visited yet
    #pragma omp parallel if(parallelize)
    {
        #pragma omp for
        for (int i = 0; i < size; ++i) {
            visited_cells[i].resize(size);
        }

        #pragma omp for
        for(int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                #pragma omp critical
                visited_cells[i][j] = false;
            }
        }
    }

    // Sets the exit cell as first visited cell
    visited_cells[exit_coords[0]][exit_coords[1]] = true;

    // Selects the exit as starting cell for the path generation
    std::vector<int> curr_cell = exit_coords;

    // Retrieves the nearest cells if there is any
    // Then randomly select one of them and proceeds visiting it
    // If a dead end is found, it backtracks all steps until it finds a new unvisited cell
    // If by backtracking all cells have been visited, the maze has been completely generated
    p_visit_forward(maze, size, curr_index, curr_cell, curr_track, visited_cells, rng, true, parallelize);
}


/**
 * Randomly generates the maze's paths by removing walls.
 *
 *  The function selects a random cell in between the ones nearby the one that is currently being processed and destroys
 *  the wall in between by creating a connection. If a cell has already been considered it cannot be connected to other
 *  ones. Once a dead end is reached, the algorithm follows back the steps until a new nearby cell that has never been
 *  considered is found and connects the current cell with it. If by backtracking the initial cell is reached, it means
 *  that there is no remaining cell to consider and the generation stops.
 *
 *  @param maze It's the matrix representing the maze that is being generated.
 *  @param size Represents the length of each maze's side.
 *  @param curr_index Represents the current index related to the visited cells tracking in the actual path. This will
 *  be used later in order to follow the steps back and find new unvisited cells (backtracking).
 *  @param exit_coords This is the random number engine to use in order to generate random values.
 *  @param curr_cell This is the current cell for which the near unvisited cells are being checked. If there is any,
 *  a connection between the 2 is performed by removing the wall in between.
 *  @param curr_track Contains all the coordinates of the visited cells in the current path, and keeps track
 *  of their traversal order.
 *  @param visited_cells This is the matrix used to keep track of all the cells that have been visited.
 *  @param rng This is the random number engine to use in order to generate random values.
 *  @param is_exit This flag is used to determine if the current cell corresponds to the exit. If so there is surely only
 *  one nearby unvisited cell, but no wall in between. So the wall removal is unneeded.
 *  @param parallelize Flag used to determine if it useful to parallelize the code inside the current function.
 */
void p_visit_forward(std::vector<std::vector<MAZE_PATH>> &maze, int &size, int &curr_index, std::vector<int> &curr_cell, std::vector<std::vector<int>> &curr_track, std::vector<std::vector<bool>> &visited_cells, std::mt19937 &rng, bool is_exit, bool parallelize) {
    // Retrieves the nearest cells if there is any
    // Then randomly select one of them and proceeds visiting it
    // If a dead end is found, it backtracks all steps until it finds a new unvisited cell
    // If by backtracking all cells have been visited, the maze has been completely generated
    int n_cells = 0;
    std::vector<std::vector<int>> near_cells = p_get_unvisited_near_cells(maze, curr_cell, size, visited_cells, n_cells, is_exit);

    // IMPORTANT:
    // The following loop can't be parallelized as the algorithm to generate the path follows a sequential movement logic
    // n_cells is updated by the function call above
    while(n_cells > 0) {
        curr_index += 1;
        // Creates the uniform_distributor that will select a random unvisited cell
        std::uniform_int_distribution<int> uniform_cell_idx(0, n_cells - 1);

        // Uses the distribution above to select the new cell to connect with
        int new_cell_index = uniform_cell_idx(rng);
        if(new_cell_index > n_cells - 1) {
            std::cout << "Wrong index found";
            exit(1);
        }
        std::vector<int> new_cell = near_cells[new_cell_index];

        // Removes the walls only if the current cell is not the exit as the exit cell has a near unvisited cell without
        // the wall in between.
        // Non exit cells have near unvisited cells separated by walls
        if(!is_exit) {
            int row_to_del = -1;
            int col_to_del = -1;

            // Removing the wall in between new_cell and curr_cell

            // Case in which the 2 cells are located onto the same column
            if(new_cell[1] == curr_cell[1])
            {
                // The wall's cell X coordinate is smaller than new_cell's one and bigger than curr_cell's
                if(new_cell[0] > curr_cell[0]) {
                    row_to_del = new_cell[0] - 1;
                    col_to_del = new_cell[1];
                }
                // The wall's cell X coordinate is bigger than new_cell's one and smaller than curr_cell's
                else {
                    row_to_del = new_cell[0] + 1;
                    col_to_del = new_cell[1];
                }
            // Case in which the 2 cells are located onto the same row
            } else if (new_cell[0] == curr_cell[0])
            {
                // The wall's cell Y coordinate is smaller than new_cell's one and bigger than curr_cell's
                if(new_cell[1] > curr_cell[1]) {
                    row_to_del = new_cell[0];
                    col_to_del = new_cell[1] - 1;
                }
                // The wall's cell Y coordinate is bigger than new_cell's one and smaller than curr_cell's
                else {
                    row_to_del = new_cell[0];
                    col_to_del = new_cell[1] + 1;
                }
            }

            // Deletes the wall in between the 2 cells
            if(row_to_del > -1 && col_to_del > -1) {
                maze[row_to_del][col_to_del] = MAZE_PATH::EMPTY;
            }
            else {
                std::cout << "Unexpected error while generating the maze path." << std::endl;
                std::cout << "The selected unvisited cell is not located beside the current one!\nExiting..";
                exit(1);
            }
        // We are moving from the exit cell to the near unvisited cell
        } else {
            is_exit = false;
        }

        // Sets the new cell as current one to consider
        curr_cell = new_cell;
        // Updates the matrix of all the visited cells so far
        visited_cells[curr_cell[0]][curr_cell[1]] = true;
        // Updates the current path for the eventual backtracking
        curr_track.push_back(curr_cell);
        // Gets the new unvisited cells nearby the current cell if there is any
        // n_cells is updated by the function call below
        near_cells = p_get_unvisited_near_cells(maze, curr_cell, size, visited_cells, n_cells, is_exit);
    }
    // Follows the path's steps back until a new unvisited cell is found, and then it resumes the path generation
    // process from there
    p_backtrack(maze, size, curr_index, curr_cell, curr_track, visited_cells, rng, parallelize);
}


/**
 * Checks for nearby unvisited cells in order to create paths.
 *
 * This function looks for the cells nearby the current one and if they have not been considered yet returns them as
 * possible path joining targets.
 *
 *  @param maze It's the matrix representing the maze that is being generated.
 *  @param curr_cell This is the current cell for which the near unvisited cells are being checked.
 *  @param size Represents the length of each maze's side.
 *  @param visited_cells This is the matrix used to keep track of all the cells that have been visited.
 *  @param n_cells This is a reference to a variable representing the total count of nearby unvisited cells.
 *  @param is_exit This flag is used to determine if the current cell corresponds to the exit. If so there is surely only
 *  one nearby unvisited cell, but no wall in between. So the wall removal is unneeded.
 *
 *  @return the matrix containing all the available cells' coordinates for joining paths.
 */
std::vector<std::vector<int>> p_get_unvisited_near_cells(std::vector<std::vector<MAZE_PATH>> &maze, std::vector<int> &curr_cell, int &size, std::vector<std::vector<bool>> &visited_cells, int &n_cells, bool is_exit) {
    // Initialize the matrix to -1 pairs
    std::vector<std::vector<int>> near_cells;
    near_cells.reserve(4);
    n_cells = 0;
    // If the current cell corresponds to the exit, we check for unvisited cells immediately near to it
    // otherwise we look for cells behind walls
    int index_offset = is_exit ? 1 : 2;
    std::vector<int> curr_pos;
    curr_pos.reserve(2);

    // Getting the indexes of the cells behind the walls
    // and checking if they have been visited already

    // Looking for an unvisited cell located at east in relation to the current cell
    int index = curr_cell[0] + index_offset;
    if(index >= 0 && index < size && !visited_cells[index][curr_cell[1]] && maze[index][curr_cell[1]] != MAZE_PATH::WALL)
    {
        curr_pos.push_back(index);
        curr_pos.push_back(curr_cell[1]);
        near_cells.push_back(curr_pos);
        n_cells += 1;
    }

    curr_pos.clear();

    // Looking for an unvisited cell located at north in relation to the current cell
    index = curr_cell[1] + index_offset;
    if(index >= 0 && index < size && !visited_cells[curr_cell[0]][index] && maze[curr_cell[0]][index] != MAZE_PATH::WALL) {
        curr_pos.push_back(curr_cell[0]);
        curr_pos.push_back(index);
        near_cells.push_back(curr_pos);
        n_cells += 1;
    }

    curr_pos.clear();

    // Looking for an unvisited cell located at west in relation to the current cell
    index = curr_cell[0] - index_offset;
    if(index >= 0 && index < size && !visited_cells[index][curr_cell[1]] && maze[index][curr_cell[1]] != MAZE_PATH::WALL)
    {
        curr_pos.push_back(index);
        curr_pos.push_back(curr_cell[1]);
        near_cells.push_back(curr_pos);
        n_cells += 1;
    }

    curr_pos.clear();

    // Looking for an unvisited cell located at south in relation to the current cell
    index = curr_cell[1] - index_offset;
    if(index >= 0 && index < size && !visited_cells[curr_cell[0]][index] && maze[curr_cell[0]][index] != MAZE_PATH::WALL) {
        curr_pos.push_back(curr_cell[0]);
        curr_pos.push_back(index);
        near_cells.push_back(curr_pos);
        n_cells += 1;
    }

    return near_cells;
}


/**
 * Follows the current path back until a new unvisited cell is found nearby.
 *
 *  This function is responsible for following the current path's steps back in order to find new possible routes to
 *  generate the other maze's paths. If by backtracking a cell with at least on nearby unvisited cell is found,
 *  the current track resumes from it and the path generation process can continue.
 *  If the starting cell is reached it means that all the cells in the maze have already been considered and the
 *  generation stops.
 *
 *  @param maze It's the matrix representing the maze that is being generated.
 *  @param size Represents the length of each maze's side.
 *  @param curr_index Represents the current index related to the visited cells tracking in the actual path. This will
 *  be used later in order to follow the steps back and find new unvisited cells (backtracking).
 *  @param curr_cell This is the current cell for which the near unvisited cells are being checked. If there is any,
 *  a connection between the 2 is performed by removing the wall in between.
 *  @param curr_track Contains all the coordinates of the visited cells in the current path, and keeps track
 *  of their traversal order.
 *  @param visited_cells This is the matrix used to keep track of all the cells that have been visited.
 *  @param rng This is the random number engine to use in order to generate random values.
 *  @param parallelize Flag used to determine if it useful to parallelize the code inside the current function.
 */
void p_backtrack(std::vector<std::vector<MAZE_PATH>> &maze, int &size, int &curr_index, std::vector<int> &curr_cell, std::vector<std::vector<int>> &curr_track, std::vector<std::vector<bool>> &visited_cells,  std::mt19937 &rng, bool parallelize) {
    // Follows the steps back until a new unvisited cell is found or
    // the maze has been completely visited
    // Starts from -2 since the latest element in the track corresponds to the latest visited cell (no near unvisited ones)
    int n_cells = 0;

    // If the track index is 0 we reached the exit again it means we visited all maze cells available,
    // so we can safely stop backtracking
    for(int index = curr_index - 1; index > 0; index--) {
        curr_cell = curr_track[index];
        curr_track.pop_back();
        curr_index = index;
        // n_cells is updated by the function call below
        p_get_unvisited_near_cells(maze, curr_cell, size, visited_cells, n_cells, false);
        if(n_cells > 0) {
            p_visit_forward(maze, size, curr_index, curr_cell, curr_track, visited_cells, rng, false, parallelize);
            break;
        }
    }
}