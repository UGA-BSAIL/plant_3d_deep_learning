import json
import os
import torch

import numpy as np
from torch.utils.data import Dataset

from modules.voxelization import Voxelization

__all__ = ['ShapeNet']


class _ShapeNetDataset(Dataset):
    def __init__(self, root, num_points, split='train', with_normal=True, with_one_hot_shape_id=True,
                 normalize=True, jitter=True):
        assert split in ['train', 'test']
        self.root = root
        self.num_points = num_points
        self.split = split
        self.with_normal = with_normal
        self.with_one_hot_shape_id = with_one_hot_shape_id
        self.normalize = normalize
        self.jitter = jitter

        shape_dir_to_shape_id = {}
        #with open(os.path.join(self.root, 'synsetoffset2category.txt'), 'r') as f:
        #    for shape_id, line in enumerate(f):
        #        shape_name, shape_dir = line.strip().split()
        #        shape_dir_to_shape_id[shape_dir] = shape_id


        file_paths = []
        if self.split == 'train':
            split = ['train', 'val']
            #file_list = ['pc (1).txt','pc (2).txt','pc (3).txt','pc (4).txt','pc (5).txt','pc (6).txt','pc (7).txt','pc (8).txt','pc (9).txt','pc (10).txt','pc (11).txt','pc (12).txt','pc (13).txt','pc (14).txt','pc (15).txt','pc (16).txt','pc (17).txt','pc (18).txt','pc (19).txt']
 
            file_list = ['SPL_0501.txt','SPL_0511.txt','SPL_0403.txt','SPL_0701.txt','SPL_0512.txt','SPL_0407.txt', 'AcalaMaxxa - Cloud.txt', 'DG365 - Cloud.txt', 'DP1646 - Cloud.txt', 'T0018MDN - Cloud.txt', 'T0246BC3MDN - Cloud.txt', 'TamcotSphinx - Cloud.txt', 'UA48 - Cloud.txt','AcalaMaxxa_200706.txt','MDN0101_200504.txt','ST5020_200708.txt','T0018MDN_200307.txt','TamcotSphinx_200808.txt','UA48_200704.txt']

        else:
            split = ['test']
            #file_list = ['pc (20).txt','pc (21).txt','pc (22).txt','pc (23).txt','pc (24).txt','pc (25).txt','pc (26).txt','pc (27).txt','pc (28).txt']
            file_list = ['SPL_0509.txt','SPL_0603.txt','SPL_0401.txt','DES56 - Cloud.txt', 'MDN0101 - Cloud.txt', 'ST5020 - Cloud.txt','DES56_200908.txt','T0246BC3MDN_200507.txt','DG3615_201108.txt']

        for filename in file_list:
            #file_paths.append( (os.path.join(self.root, filename ), 0) )
            file_paths.append( os.path.join(self.root, filename) )

        self.voxels = []

        for filename in file_paths:
            data = np.loadtxt(filename) 
            self.get_voxels(data, self.voxels) 
            
        #for s in split:
        #    with open(os.path.join(self.root, 'train_test_split', f'shuffled_{s}_file_list.json'), 'r') as f:
        #        file_list = json.load(f)
        #        for file_path in file_list:
        #            _, shape_dir, filename = file_path.split('/')
        #            file_paths.append(
        #                (os.path.join(self.root, shape_dir, filename + '.txt'),
        #                 shape_dir_to_shape_id[shape_dir])
        #            )
        self.file_paths = file_paths
        self.num_shapes = 1 #16
        self.num_classes = 3 #50

        self.cache = {}  # from index to (point_set, cls, seg) tuple
        self.cache_size = 0


    def get_voxels(self, data, voxels):
        coords = data[:,:3]
        label = data[:,-1]

        num_points = 500000
        choice = np.random.choice(data.shape[0], num_points, replace=True)
        coords = coords[choice, :].transpose()
        label = label[choice]

        eps = 0
        r = 8
        coords = torch.from_numpy(coords).unsqueeze(0)
        # coords = coords.detach()
        norm_coords = coords - coords.mean(2, keepdim=True)
        norm_coords = norm_coords / (norm_coords.norm(dim=1, keepdim=True).max(dim=2, keepdim=True).values * 2.0 + eps) + 0.5
        norm_coords1 = norm_coords.clone()

        norm_coords = torch.clamp(norm_coords * r, 0, r - 1)
        vox_coords = torch.round(norm_coords).to(torch.int32)
        v1 = vox_coords.squeeze(0)
        for i in range(r):
            for j in range(r):
                for k in range(r):
                    if ( (v1[0,:] == i) & (v1[1,:] == j) & (v1[2,:] == k) ).sum() >= 2048:
                        self.voxels.append((coords[0,:,(v1[0,:] == i) & (v1[1,:] == j) & (v1[2,:] == k) ].numpy().transpose(), label[(v1[0,:] == i) & (v1[1,:] == j) & (v1[2,:] == k)]))





    def __getitem__(self, index):
        if index in self.cache:
            coords, normal, label, shape_id = self.cache[index]
        else:
            data, label = self.voxels[index]
            data = data.astype(np.float32)
            shape_id = 0
            coords = data[:, :3]
            if self.normalize:
                coords = self.normalize_point_cloud(coords)
            normal = data[:, :3] #:6]
            label = label.astype(np.int64) #-19
            #label[label == 1] = 0

            #label[label == 2] = 1


            if len(self.cache) < self.cache_size:
                self.cache[index] = (coords, normal, label, shape_id)


        #if np.random.random(1)[0] > 0.5:
        #    coords[:, 0:3] = self.shift_point_cloud(coords[:, 0:3])
        #    coords[:, 0:3] = self.rotate_point_cloud(coords[:, 0:3]) ## added _z
        #elif np.random.random(1)[0] > 0.5:
        #    coords[:, 0:3] = self.rotate_point_cloud(coords[:, 0:3])
        #    # coords[:, 0:3] = self.random_scale_point_cloud(coords[:, 0:3])
        #    coords[:, 0:3] = self.shift_point_cloud(coords[:, 0:3])
        #elif np.random.random(1)[0] > 0.5:
        #    # coords[:, 0:3] = self.random_scale_point_cloud(coords[:, 0:3])
        #    coords[:, 0:3] = self.shift_point_cloud(coords[:, 0:3])
			


        #print('+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++',coords.dtype)

        choice = np.random.choice(label.shape[0], self.num_points, replace=True) # here we can add augmentation
        coords = coords[choice, :].transpose()

        if self.jitter:
            coords = self.jitter_point_cloud(coords)
        if self.with_normal:
            normal = normal[choice, :].transpose()
            if self.with_one_hot_shape_id:
                shape_one_hot = np.zeros((self.num_shapes, self.num_points), dtype=np.float32)
                shape_one_hot[shape_id, :] = 1.0
                point_set = np.concatenate([coords, normal, shape_one_hot])
            else:
                point_set = np.concatenate([coords, normal])
        else:
            if self.with_one_hot_shape_id:
                shape_one_hot = np.zeros((self.num_shapes, self.num_points), dtype=np.float32)
                shape_one_hot[shape_id, :] = 1.0
                point_set = np.concatenate([coords, shape_one_hot])
            else:
                point_set = coords

        #print('**************************************************************************************',coords.dtype)
        return point_set, label[choice].transpose()

    def __len__(self):
        return len(self.voxels) #len(self.file_paths)

    @staticmethod
    def rotate_point_cloud(batch_data):
        """ Randomly rotate the point clouds to augument the dataset
        rotation is per shape based along up direction
        Input:
        BxNx3 array, original batch of point clouds
        Return:
        BxNx3 array, rotated batch of point clouds
        """
        rotated_data = np.zeros(batch_data.shape, dtype=np.float32)
        B=1
        for k in range(B):
            rotation_angle = np.random.uniform() * 2 * np.pi
            cosval = np.cos(rotation_angle)
            sinval = np.sin(rotation_angle)
            rotation_matrix = np.array([[cosval, 0, sinval],[0, 1, 0],[-sinval, 0, cosval]])
            shape_pc = batch_data
            rotated_data = np.dot(shape_pc.reshape((-1, 3)), rotation_matrix)
        return rotated_data


    @staticmethod
    def rotate_point_cloud_z(batch_data):
        """ Randomly rotate the point clouds to augument the dataset
        rotation is per shape based along up direction
        Input:
        BxNx3 array, original batch of point clouds
        Return:
        BxNx3 array, rotated batch of point clouds
        """
        rotated_data = np.zeros(batch_data.shape, dtype=np.float32)
        B=1
        for k in range(B):
            rotation_angle = np.random.uniform() * 2 * np.pi
            cosval = np.cos(rotation_angle)
            sinval = np.sin(rotation_angle)
            rotation_matrix = np.array([[cosval, 0, sinval],[0, 1, 0],[-sinval, 0, cosval]])
            shape_pc = batch_data
            rotated_data = np.dot(shape_pc.reshape((-1, 3)), rotation_matrix)
        return rotated_data




    @staticmethod
    def shift_point_cloud(batch_data, shift_range=0.1):
        """ Randomly shift point cloud. Shift is per point cloud.
        Input:
        BxNx:3 array, original batch of point clouds
        Return:
        BxNx3 array, shifted batch of point clouds
        """
        N, C = batch_data.shape
        B=1
        shifts = np.random.uniform(-shift_range, shift_range, (B,3))
        for batch_index in range(B):
            batch_data[:,:] += shifts[batch_index,:]
        return batch_data


    @staticmethod
    def random_scale_point_cloud(batch_data, scale_low=0.8, scale_high=1.25):
        """ Randomly scale the point cloud. Scale is per point cloud.
        Input:
        BxNx3 array, original batch of point clouds
        Return:
        BxNx3 array, scaled batch of point clouds
        """
        N, C = batch_data.shape
        B=1
        scales = np.random.uniform(scale_low, scale_high, B)
        for batch_index in range(B):
            batch_data *= scales[batch_index]
        return batch_data


    @staticmethod
    def normalize_point_cloud(points):
        centroid = np.mean(points, axis=0)
        points = points - centroid
        return points / np.max(np.linalg.norm(points, axis=1))


    @staticmethod
    def normalize_point_cloud1(points):
        centroid = np.mean(points, axis=0) # centroid = [ 63.75,  92.75, 119.  ]
        points = points - centroid
        max_val = np.max(np.linalg.norm(points, axis=1))
        return points / (max_val*2) + 0.5

    @staticmethod
    def jitter_point_cloud(points, sigma=0.01, clip=0.05):
        """ Randomly jitter points. jittering is per point.
            Input:
              3xN array, original batch of point clouds
            Return:
              3xN array, jittered batch of point clouds
        """
        assert (clip > 0)
        return np.clip(sigma * np.random.randn(*points.shape), -1 * clip, clip).astype(np.float32) + points


class ShapeNet(dict):
    def __init__(self, root, num_points, split=None, with_normal=True, with_one_hot_shape_id=True,
                 normalize=True, jitter=True):
        super().__init__()
        if split is None:
            split = ['train', 'test']
        elif not isinstance(split, (list, tuple)):
            split = [split]
        for s in split:
            self[s] = _ShapeNetDataset(root=root, num_points=num_points, split=s,
                                       with_normal=with_normal, with_one_hot_shape_id=with_one_hot_shape_id,
                                       normalize=normalize, jitter=jitter if s == 'train' else False)

