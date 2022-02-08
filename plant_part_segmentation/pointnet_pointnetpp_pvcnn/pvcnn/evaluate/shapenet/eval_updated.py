import argparse
import os
import random
import sys
import json
import numba
import numpy as np
import torch
import time

sys.path.append(os.getcwd())

__all__ = ['evaluate']

output_dir =  'output_plant'

color_map_file =  'part_color_mapping.json'
color_map = json.load(open(color_map_file, 'r'))


def normalize_point_cloud(points):
    centroid = np.mean(points, axis=0)
    points = points - centroid
    max_val = np.max(np.linalg.norm(points, axis=1))
    return points / max_val, max_val, centroid

def denormalize_point_cloud(points ,max_val, centroid):
    return ((points.transpose()*max_val)+centroid).transpose()

def get_voxels(fname):
    voxels = []
    data = np.loadtxt(fname).astype(np.float32)
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
    norm_coords = torch.clamp(norm_coords * r, 0, r - 1)
    vox_coords = torch.round(norm_coords).to(torch.int32)
    v1 = vox_coords.squeeze(0)
    for i in range(r):
        for j in range(r):
            for k in range(r):
                if ( (v1[0,:] == i) & (v1[1,:] == j) & (v1[2,:] == k) ).sum() >= 10:
                    voxels.append((coords[0,:,(v1[0,:] == i) & (v1[1,:] == j) & (v1[2,:] == k) ].numpy().transpose(), label[(v1[0,:] == i) & (v1[1,:] == j) & (v1[2,:] == k)]))
    return voxels




def output_color_point_cloud(data, seg, out_file):
    color_map[0] = [1,0,0]
    color_map[1] = [0,1,0]
    color_map[2] = [0,0,1]

    with open(out_file, 'w') as f:
        l = len(seg)
        for i in range(l):
            color = color_map[seg[i]]
            f.write('v %f %f %f %f %f %f\n' % (data[0][i], data[1][i], data[2][i], color[0], color[1], color[2]))

def output_color_point_cloud_red_blue(data, seg, out_file):
    with open(out_file, 'w') as f:
        l = len(seg)
        for i in range(l):
            if seg[i] == 1:
                color = [0, 0, 1]
            elif seg[i] == 0:
                color = [1, 0, 0]
            else:
                color = [0, 0, 0]

            f.write('v %f %f %f %f %f %f\n' % (data[0][i], data[1][i], data[2][i], color[0], color[1], color[2]))


def prepare():
    from utils.common import get_save_path
    from utils.config import configs
    from utils.device import set_cuda_visible_devices

    # since PyTorch jams device selection, we have to parse args before import torch (issue #26790)
    parser = argparse.ArgumentParser()
    parser.add_argument('configs', nargs='+')
    parser.add_argument('--devices', default=None)
    args, opts = parser.parse_known_args()
    if args.devices is not None and args.devices != 'cpu':
        gpus = set_cuda_visible_devices(args.devices)
    else:
        gpus = []

    print(f'==> loading configs from {args.configs}')
    configs.update_from_modules(*args.configs)
    # define save path
    save_path = get_save_path(*args.configs, prefix='runs')
    os.makedirs(save_path, exist_ok=True)
    configs.train.save_path = save_path
    configs.train.checkpoint_path = os.path.join(save_path, 'latest.pth.tar')
    configs.train.best_checkpoint_path = os.path.join(save_path, 'best.pth.tar')

    # override configs with args
    configs.update_from_arguments(*opts)
    if len(gpus) == 0:
        configs.device = 'cpu'
        configs.device_ids = []
    else:
        configs.device = 'cuda'
        configs.device_ids = gpus
    configs.dataset.split = configs.evaluate.dataset.split
    if 'best_checkpoint_path' not in configs.evaluate or configs.evaluate.best_checkpoint_path is None:
        if 'best_checkpoint_path' in configs.train and configs.train.best_checkpoint_path is not None:
            configs.evaluate.best_checkpoint_path = configs.train.best_checkpoint_path
        else:
            configs.evaluate.best_checkpoint_path = os.path.join(configs.train.save_path, 'best.pth.tar')
    assert configs.evaluate.best_checkpoint_path.endswith('.pth.tar')
    configs.evaluate.stats_path = configs.evaluate.best_checkpoint_path.replace('.pth.tar', '.eval.npy')

    return configs


def evaluate(configs=None):
    configs = prepare() if configs is None else configs

    import math
    import torch
    import torch.backends.cudnn as cudnn
    import torch.nn.functional as F
    from tqdm import tqdm

    from meters.shapenet import MeterShapeNet

    ###########
    # Prepare #
    ###########

    if configs.device == 'cuda':
        cudnn.benchmark = True
        if configs.get('deterministic', False):
            cudnn.deterministic = True
            cudnn.benchmark = False
    if ('seed' not in configs) or (configs.seed is None):
        configs.seed = torch.initial_seed() % (2 ** 32 - 1)
    seed = configs.seed
    random.seed(seed)
    np.random.seed(seed)
    torch.manual_seed(seed)

    print(configs)

    #if os.path.exists(configs.evaluate.stats_path):
    #    stats = np.load(configs.evaluate.stats_path)
    #    print('clssIoU: {}'.format('  '.join(map('{:>8.2f}'.format, stats[:, 0] / stats[:, 1] * 100))))
    #    print('meanIoU: {:4.2f}'.format(stats[:, 0].sum() / stats[:, 1].sum() * 100))
    #    return

    #################################
    # Initialize DataLoaders, Model #
    #################################

    print(f'\n==> loading dataset "{configs.dataset}"')
    dataset = configs.dataset()[configs.dataset.split]
    meter = MeterShapeNet()

    print(f'\n==> creating model "{configs.model}"')
    model = configs.model()
    if configs.device == 'cuda':
        model = torch.nn.DataParallel(model)
    model = model.to(configs.device)

    if os.path.exists(configs.evaluate.best_checkpoint_path):
        print(f'==> loading checkpoint "{configs.evaluate.best_checkpoint_path}"')
        checkpoint = torch.load(configs.evaluate.best_checkpoint_path)
        model.load_state_dict(checkpoint.pop('model'))
        del checkpoint
    else:
        return

    model.eval()


    mem_params = sum([param.nelement()*param.element_size() for param in model.parameters()])
    mem_bufs = sum([buf.nelement()*buf.element_size() for buf in model.buffers()])
    mem = mem_params + mem_bufs # in bytes
    print("Mem of model in bytes: ", mem)

    ##############
    # Evaluation #
    ##############

    cstats = np.zeros((configs.data.num_classes, 6))
    stats = np.zeros((configs.data.num_shapes, 2))

    for shape_idx, file_path in enumerate(tqdm(dataset.file_paths, desc='eval', ncols=0)):
        voxels = get_voxels(file_path)

        torch.cuda.reset_peak_memory_stats()


        #start = torch.cuda.Event(enable_timing=True)
        #end = torch.cuda.Event(enable_timing=True)

        #start.record()

        start_time = time.time()

        data = np.loadtxt(file_path).astype(np.float32)
        #shape_idx +=1
        start_time = time.time()
        choice = np.random.choice(data.shape[0], dataset.num_points, replace=True)
        data = data[choice, :]

        total_num_points_in_shape = data.shape[0]
        confidences = np.zeros(total_num_points_in_shape, dtype=np.float32)
        predictions = np.full(total_num_points_in_shape, -1, dtype=np.int64)
        
        coords = data[:, :3]

        mv = 0; cent = 0;
        if dataset.normalize:
            coords, mv, cent = normalize_point_cloud(coords)
        coords = coords.transpose()
        #ground_truth = data[:, -1].astype(np.int64)
        ground_truth = label.astype(np.int64)
        if dataset.with_normal:
            normal = data[:, 3:6].transpose()
            if dataset.with_one_hot_shape_id:
                shape_one_hot = np.zeros((dataset.num_shapes, coords.shape[-1]), dtype=np.float32)
                shape_one_hot[shape_id, :] = 1.0
                point_set = np.concatenate([coords, normal, shape_one_hot])
            else:
                point_set = np.concatenate([coords, normal])
        else:
            if dataset.with_one_hot_shape_id:
                shape_one_hot = np.zeros((dataset.num_shapes, coords.shape[-1]), dtype=np.float32)
                shape_one_hot[shape_id, :] = 1.0
                point_set = np.concatenate([coords, shape_one_hot])
            else:
                point_set = coords
        extra_batch_size = configs.evaluate.num_votes * math.ceil(total_num_points_in_shape / dataset.num_points)
        total_num_voted_points = extra_batch_size * dataset.num_points
        num_repeats = math.ceil(total_num_voted_points / total_num_points_in_shape)
        shuffled_point_indices = np.tile(np.arange(total_num_points_in_shape), num_repeats)
        shuffled_point_indices = shuffled_point_indices[:total_num_voted_points]
        np.random.shuffle(shuffled_point_indices)
        start_class, end_class = meter.part_class_to_shape_part_classes[ground_truth[0]]

        # model inference
        inputs = torch.from_numpy(
            point_set[:, shuffled_point_indices].reshape(-1, extra_batch_size, dataset.num_points).transpose(1, 0, 2)
        ).float().to(configs.device)
        with torch.no_grad():
            vote_confidences = F.softmax(model(inputs), dim=1)
            vote_confidences, vote_predictions = vote_confidences[:, start_class:end_class, :].max(dim=1)
            vote_confidences = vote_confidences.view(total_num_voted_points).cpu().numpy()
            vote_predictions = (vote_predictions + start_class).view(total_num_voted_points).cpu().numpy()

        update_shape_predictions(vote_confidences, vote_predictions, shuffled_point_indices,
                                 confidences, predictions, total_num_voted_points)
        update_stats(stats, cstats, ground_truth, predictions, shape_id, start_class, end_class)

        #### saving results
        coords = denormalize_point_cloud(coords,mv,cent)
        
        #act_coords = np.hstack((act_coords, coords))
        #act_gt = np.hstack((act_gt, ground_truth))
        #act_pd = np.hstack((act_pd, predictions))
        #
        #output_color_point_cloud(act_coords, act_gt, os.path.join(output_dir, str(shape_idx)+'_gt.txt'))
        #output_color_point_cloud(act_coords, act_pd, os.path.join(output_dir, str(shape_idx)+'_pred.txt'))
        #output_color_point_cloud_red_blue(act_coords, np.int32(act_gt == act_pd), os.path.join(output_dir, str(shape_idx)+'_diff.txt'))
        #shape_index+=1


        #end.record()

       # Waits for everything to finish running
        #torch.cuda.synchronize()

        #print('Time elapsed: ',start.elapsed_time(end))

        print("--- %s seconds ---" % (time.time() - start_time))

        print('torch.cuda.max_memory_allocated()', torch.cuda.max_memory_allocated())



    np.save(configs.evaluate.stats_path, stats)
    
    print('shapeIoU: {}'.format('  '.join(map('{:>8.2f}'.format, stats[:, 0] / stats[:, 1] * 100))))
    print('clssPrec: {}'.format('  '.join(map('{:>8.2f}'.format, cstats[:, 2] / cstats[:, 5] * 100))))
    print('clssRecall: {}'.format('  '.join(map('{:>8.2f}'.format, cstats[:, 3] / cstats[:, 5] * 100))))
    print('clssIoU: {}'.format('  '.join(map('{:>8.2f}'.format, cstats[:, 4] / cstats[:, 5] * 100))))
    print('Accuracy: {:4.2f}'.format(cstats[:, 1].sum() / cstats[:, 0].sum() * 100))
    print('meanIoU: {:4.2f}'.format(stats[:, 0].sum() / stats[:, 1].sum() * 100))


def update_shape_predictions(vote_confidences, vote_predictions, shuffled_point_indices,
                             shape_confidences, shape_predictions, total_num_voted_points):
    for p in range(total_num_voted_points):
        point_index = shuffled_point_indices[p]
        current_confidence = vote_confidences[p]
        if current_confidence > shape_confidences[point_index]:
            shape_confidences[point_index] = current_confidence
            shape_predictions[point_index] = vote_predictions[p]


@numba.jit()
def update_stats(stats, cstats, ground_truth, predictions, shape_id, start_class, end_class):
    iou = 0.0
    for i in range(start_class, end_class):
        igt = (ground_truth == i)
        ipd = (predictions == i)
        union = np.sum(igt | ipd)
        intersection = np.sum(igt & ipd)
        if union == 0:
            iou += 1
        else:
            iou += intersection / union
            
           


        sum_igt =  np.sum(igt)   
        sum_ipd =  np.sum(ipd)            
        cstats[i][0] += np.sum(igt) # num of gt pts in that class
        cstats[i][1] += intersection # num of truely predicted pts in that class (true positive)
        if sum_ipd == 0:
            cstats[i][2] += 1 # precision in that class
        else:
            cstats[i][2] += intersection/np.sum(ipd) # precision in that class
        if sum_igt == 0:
            cstats[i][3] += 1 # recall in that class
        else:
            cstats[i][3] += intersection/np.sum(igt) # recall in that class
        if union == 0:
            cstats[i][4] += 1 # iou in that class
        else:
            cstats[i][4] += intersection/union # iou in that class
        cstats[i][5] += 1 # number of point clouds in that class



        
    iou /= (end_class - start_class)
    stats[shape_id][0] += iou
    stats[shape_id][1] += 1


if __name__ == '__main__':
    evaluate()

