### Plant part segmentation using 3D Deep Learning

## Prerequisites

The code is built with following libraries (see [requirements.txt](requirements.txt)):
- Python >= 3.7
- [PyTorch](https://github.com/pytorch/pytorch) >= 1.3
- [ninja] (https://pypi.org/project/ninja) >= 1.10
- [numba](https://github.com/numba/numba)
- [numpy](https://github.com/numpy/numpy)
- [scipy](https://github.com/scipy/scipy)
- [six](https://github.com/benjaminp/six)
- [tensorboardX](https://github.com/lanpa/tensorboardX) >= 1.2
- [tqdm](https://github.com/tqdm/tqdm)
- [plyfile](https://github.com/dranjan/python-plyfile)
- [h5py](https://github.com/h5py/h5py)

## Running on GPU:

### PointNet
##### Training: 
```
python train.py configs/shapenet/pointnet.py --devices 0
```
##### Testing: 
```
python train.py configs/shapenet/pointnet.py --devices 0 --evaluate
```

### PointNet++
##### Training: 
```
python train.py configs/shapenet/pointnet2msg.py --devices 0
```
##### Testing: 
```
python train.py configs/shapenet/pointnet2msg.py --devices 0 --evaluate
```

### Point-voxel CNN
##### Training: 
```
python train.py configs/shapenet/pvcnn/c1.py --devices 0
```
##### Testing: 
```
python train.py configs/shapenet/pvcnn/c1.py --devices 0 --evaluate
```

## Running on GPU on Sapelo cluster:

Job sumission scripts for each network: 

### PointNet
##### Training: 
```
sbatch ./current_script_pointnet.sh
```
##### Testing: 
```
sbatch ./current_script_pointnet_test.sh
```

### PointNet++
##### Training: 
```
sbatch ./current_script_pointnet2.sh
```
##### Testing: 
```
sbatch ./current_script_pointnet2_test.sh
```

### Point-voxel CNN
##### Training: 
```
sbatch ./current_script_pvcnn.sh
```
##### Testing: 
```
sbatch ./current_script_pvcnn_test.sh
```

## Acknowledgement

- [GACRC] 
- [PointNet2](https://github.com/charlesq34/pointnet2) (MIT License) and [Pointnet2_PyTorch](https://github.com/erikwijmans/Pointnet2_PyTorch).
- [Point-voxel CNN](https://github.com/mit-han-lab/pvcnn) (MIT License).

