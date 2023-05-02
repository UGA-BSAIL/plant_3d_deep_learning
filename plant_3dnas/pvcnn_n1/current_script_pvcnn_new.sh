#!/bin/bash
#SBATCH --job-name=pvcnn_shapenet_p100 # Job name
#SBATCH --partition=gpu_p # Partition (queue) name, i.e., gpu_p
#SBATCH --gres=gpu:A100:1 # Requests one GPU device; --gres=gpu:P100:1, --gres=gpu:K40:1
#SBATCH --ntasks=1 # Run a single task
#SBATCH --cpus-per-task=4 # Number of CPU cores per task
#SBATCH --mem=40gb # Job memory request
#SBATCH --time=48:00:00 # Time limit hrs:min:sec
#SBATCH --export=NONE # Do not load any users’ explicit environment variables
#SBATCH --output=%x_%j.out # Standard output and error log, e.g., amber_1234.out
#SBATCH --mail-type=END,FAIL # Mail events (BEGIN, END, FAIL, ALL)
#SBATCH --mail-user=fs47816@uga.edu # Where to send mail

cd /scratch/fs47816/workdir/sample_scripts/pvcnn_n1/pvcnn


ml TensorFlow/2.4.1-fosscuda-2020b
ml protobuf/3.14.0-GCCcore-10.2.0
ml tensorboard/2.8.0-fosscuda-2020b-Python-3.8.6
ml PyTorch/1.10.0-fosscuda-2020b-Python-3.8.6
ml tqdm/4.61.2-GCCcore-10.2.0
ml numba/0.55.1-fosscuda-2020b-Python-3.8.6
ml Ninja/1.10.1-GCCcore-10.2.0
ml scikit-learn/0.24.2-fosscuda-2020b


python train.py configs/shapenet/pvcnn/c1.py --devices 0

