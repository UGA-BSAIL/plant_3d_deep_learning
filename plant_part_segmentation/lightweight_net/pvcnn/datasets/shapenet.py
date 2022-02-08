import json
import os

import numpy as np
from torch.utils.data import Dataset

__all__ = ['ShapeNet']



output_dir =  'input_plant'
output_dir1 =  'input_plant_unnorm'

color_map_file =  'part_color_mapping.json'
color_map = json.load(open(color_map_file, 'r'))



def output_color_point_cloud(data, seg, out_file):

    #print("************************************************************************************",out_file)

    color_map[0] = [1,0,0]
    color_map[1] = [0,1,0]
    color_map[2] = [0,0,1]

    with open(out_file, 'w') as f:
        l = len(seg)
        for i in range(l):
            color = color_map[seg[i]]
            f.write('v %f %f %f %f %f %f\n' % (data[0][i], data[1][i], data[2][i], color[0], color[1], color[2]))




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
        self.partition = self.split


        shape_dir_to_shape_id = {}
        with open(os.path.join(self.root, 'synsetoffset2category.txt'), 'r') as f:
            for shape_id, line in enumerate(f):
                shape_name, shape_dir = line.strip().split()
                shape_dir_to_shape_id[shape_dir] = shape_id
        file_paths = []
        if self.split == 'train':
            split = ['train', 'val']
        else:
            split = ['test']
        for s in split:
            with open(os.path.join(self.root, 'train_test_split', f'shuffled_{s}_file_list.json'), 'r') as f:
                file_list = json.load(f)
                for file_path in file_list:
                    _, shape_dir, filename = file_path.split('/')
                    file_paths.append(
                        (os.path.join(self.root, shape_dir, filename + '.txt'),
                         shape_dir_to_shape_id[shape_dir])
                    )
        self.file_paths = file_paths
        self.num_shapes = 1 #16
        self.num_classes = 3 #50




        if self.partition == 'train':
            split = ['train', 'val']
            #file_list = ['pc (1).txt','pc (2).txt','pc (3).txt','pc (4).txt','pc (5).txt','pc (6).txt','pc (7).txt','pc (8).txt','pc (9).txt','pc (10).txt','pc (11).txt','pc (12).txt','pc (13).txt','pc (14).txt','pc (15).txt','pc (16).txt','pc (17).txt','pc (18).txt','pc (19).txt']

            file_list = ['SPL_0501.txt','SPL_0511.txt','SPL_0403.txt','SPL_0701.txt','SPL_0512.txt','SPL_0407.txt','SPL_0508.txt','SPL_0702.txt','AcalaMaxxa - Cloud.txt', 'DG365 - Cloud.txt', 'DP1646 - Cloud.txt', 'T0018MDN - Cloud.txt', 'T0246BC3MDN - Cloud.txt', 'TamcotSphinx - Cloud.txt', 'UA48 - Cloud.txt','AcalaMaxxa_200706.txt','MDN0101_200504.txt','ST5020_200708.txt','T0018MDN_200307.txt','TamcotSphinx_200808.txt','UA48_200704.txt']

        else:
            split = ['test']
            #file_list = ['pc (20).txt','pc (21).txt','pc (22).txt','pc (23).txt','pc (24).txt','pc (25).txt','pc (26).txt','pc (27).txt','pc (28).txt']
            file_list = ['SPL_0509.txt','SPL_0603.txt','SPL_0401.txt','DES56 - Cloud.txt', 'MDN0101 - Cloud.txt', 'ST5020 - Cloud.txt','DES56_200908.txt','T0246BC3MDN_200507.txt','DG3615_201108.txt']


        self.file_paths = []
        root1 =   '/scratch/fs47816/point_cloud_data/plant_data/'
        for fname in file_list:
            self.file_paths.append(os.path.join(root1,fname))






        self.cache = {}  # from index to (point_set, cls, seg) tuple
        self.cache_size = 0 #20000

    def __getitem__(self, index):
        if index in self.cache:
            coords, normal, label, shape_id = self.cache[index]
        else:
            shape_id = 0
            file_path = self.file_paths[index]
            data = np.loadtxt(file_path).astype(np.float32)
            coords = data[:, :3]
            if self.normalize:
                coords = self.normalize_point_cloud(coords)
            normal = data[:, 3:6]
            label = data[:, -1].astype(np.int64)
            if len(self.cache) < self.cache_size:
                self.cache[index] = (coords, normal, label, shape_id)


        if np.random.random(1)[0] < 0.34:
            coords[:, 0:3] = self.rotate_point_cloud_y(coords[:, 0:3])


        elif np.random.random(1)[0] > 0.5:
            coords[:, 0:3] = self.rotate_point_cloud_x(coords[:, 0:3])


        choice = np.random.choice(label.shape[0], self.num_points, replace=True)
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

        #out_file_name = os.path.join(output_dir, file_path.split('/')[-1])
        #print("********************* out_file_name",out_file_name)
        #output_color_point_cloud(point_set, label[choice], out_file_name)


        return point_set, label[choice].transpose()

    def __len__(self):
        return len(self.file_paths)




    @staticmethod
    def rotate_point_cloud_x(batch_data):
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
    def rotate_point_cloud_y(batch_data):
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
            rotation_matrix = np.array([[1,0,0], [0, cosval, -sinval],[0, sinval, cosval]])
            shape_pc = batch_data
            rotated_data = np.dot(shape_pc.reshape((-1, 3)), rotation_matrix)
        return rotated_data




    @staticmethod
    def normalize_point_cloud(points):
        centroid = np.mean(points, axis=0)
        points = points - centroid
        return points / np.max(np.linalg.norm(points, axis=1))

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
