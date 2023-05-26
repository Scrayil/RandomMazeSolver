# RandomMazeSolver

RandomMazeSolver is a program capable of generating and solving mazes randomly.
It consists of two different versions:
- the sequential one in which all the instructions are executed serially
- the parallel one that is realized by using parallelization paradigms based onto the OpenMP library

The execution is based onto a configuration file in which some parameters can be specified.  
The parameters include things like the number of executions to perform, the versions of the program to execute,
the size of the maze, the number of particles to generate and so on.  
Specific seeds can eventually be set in order to generate predictable mazes and their relative solutions.  
**Note:** If not specified, at each execution seeds are randomly generated but both versions in the same execution
will share the same values for consistency.

The project is used to show and evaluate the benefits of parallelization over the sequential execution of the code.  
![Results](https://github.com/Scrayil/RandomMazeSolver/tree/main/results) and ![reports](https://github.com/Scrayil/RandomMazeSolver/tree/main/report) have been included into this repository.

## Reporting
For simplicity maze's images have been generated by using ascii characters and they look as follows:  

![Solved maze's image](https://github.com/Scrayil/RandomMazeSolver/blob/915422667fad7649c702ba4d776dca17ba04e04b/report/media/images/solved_maze.png)    

Here is a sample report that shows the record related to the above maze's image:  

https://github.com/Scrayil/RandomMazeSolver/blob/29e9452dc768a101b30393f1d8dd62700704b79d/results/executions_report.csv?plain=1#L1-L3  

## Notes
**Showing intermediate steps** while generating and solving big mazes by using a big amount of particles results into slow console updates, flickering and an harsh visual experience.  

At the moment the **mazes' images filenames generation** does guarantee uniqueness if the generation and the solution of 2 consecutive mazes don't happen during the same second.  
If 2 small mazes are solved consecutively in the same second (timestamp), the uniqueness is guaranteed only if different seeds have been used between the two executions. (default)  

## License
Copyright 2023 Mattia Bennati  
Licensed under the GNU GPL V2: https://www.gnu.org/licenses/old-licenses/gpl-2.0.html
