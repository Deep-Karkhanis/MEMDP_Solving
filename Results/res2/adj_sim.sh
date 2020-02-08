#!/bin/bash

cd ../Data/

# python ./prepare_maze_with_failures.py -i Mazes/s1k_x1k_e6_h1000.maze
# python ./prepare_maze_with_failures.py -i Mazes/s1k_x10k_e6_h500.maze

cd ../Code/

# python ./prepare_maze_with_failures.py -i Mazes/s1k_x10k_e2_h500.maze
./run.sh -m pamcp -x 1000 -h 1000 -d mz -n s1k_x1k_e6_h1000 -c > adj_long_corr_out_s1k_x1k_e6_h1000
echo "Ended Simulation 3"
./run.sh -m pamcp -x 10000 -h 500 -d mz -n s1k_x10k_e6_h500 -c > adj_long_corr_out_s1k_x10k_e6_h500
echo "Ended Simulation 4"
# ./run.sh -m pamcp -x 10000 -h 500 -d mz -n s1k_x10k_e6_h500 -c > long_corr_out_s1k_x10k_e6_h500
# echo "Ended Simulation 2"

# ./run.sh -m pamcp -h 500 -d mz -n s1k_x10k_e2_h500 -c > out_s1k_x10k_e2_h500
# echo "Ended Simulation 3"


# #!/bin/bash

# cd ../Data/

# python ./prepare_maze_with_failures.py -i Mazes/adj_s1k_x100_e6_h500.maze

# cd ../Code/

# ./run.sh -m pamcp -x 100 -h 500 -d mz -n adj_s1k_x100_e6_h500 -c > adjout_s1k_x100_e6_h500
# echo "Ended Simulation 2"
