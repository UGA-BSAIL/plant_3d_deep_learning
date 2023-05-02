#!/bin/bash
#SBATCH --job-name=pointnetplantnetp100_aug # Job name
#SBATCH --partition=gpu_p # Partition (queue) name, i.e., gpu_p
#SBATCH --gres=gpu:K40:1 # Requests one GPU device; --gres=gpu:P100:1, --gres=gpu:K40:1
#SBATCH --ntasks=1 # Run a single task
#SBATCH --cpus-per-task=4 # Number of CPU cores per task
#SBATCH --mem=40gb # Job memory request
#SBATCH --time=48:00:00 # Time limit hrs:min:sec
#SBATCH --export=NONE # Do not load any users’ explicit environment variables
#SBATCH --output=%x_%j.out # Standard output and error log, e.g., amber_1234.out
#SBATCH --mail-type=END,FAIL # Mail events (BEGIN, END, FAIL, ALL)
#SBATCH --mail-user=fs47816@@uga.edu # Where to send mail
cd /scratch/fs47816/workdir/sample_scripts/Pointnet2_plantnet_p100_aug_500ep_nd/Pointnet_Pointnet2_pytorch
ml Python/3.7.4-GCCcore-8.3.0
ml TensorFlow/2.2.0-fosscuda-2019b-Python-3.7.4
ml protobuf/3.10.0-GCCcore-8.3.0
ml tensorboard/2.4.1-fosscuda-2019b-Python-3.7.4
ml PyTorch/1.6.0-fosscuda-2019b-Python-3.7.4
ml tqdm/4.41.1-GCCcore-8.3.0


python inference.py --batch_size 1 --num_point 100000 --log_dir pointnet2_part_seg_msg


