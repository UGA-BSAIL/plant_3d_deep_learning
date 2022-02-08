import torch
import torch.nn as nn

from models.utils import create_pointnet2_sa_components, create_pointnet2_fp_modules, create_mlp_components

__all__ = ['PointNet2SSG', 'PointNet2MSG']


class PointNet2(nn.Module):
    def __init__(self, num_classes, num_shapes, sa_blocks, fp_blocks, with_one_hot_shape_id=True,
                 extra_feature_channels=3, width_multiplier=1, voxel_resolution_multiplier=1):
        super().__init__()
        assert extra_feature_channels >= 0

        self.in_channels = extra_feature_channels + 3
        self.num_shapes = num_shapes
        self.with_one_hot_shape_id = with_one_hot_shape_id

        sa_layers, sa_in_channels, channels_sa_features, _ = create_pointnet2_sa_components(
            sa_blocks=sa_blocks, extra_feature_channels=extra_feature_channels, width_multiplier=width_multiplier
        )
        self.sa_layers = nn.ModuleList(sa_layers)

        # use one hot vector in the last fp module
        sa_in_channels[0] += num_shapes if with_one_hot_shape_id else 0
        fp_layers, channels_fp_features = create_pointnet2_fp_modules(
            fp_blocks=fp_blocks, in_channels=channels_sa_features, sa_in_channels=sa_in_channels,
            width_multiplier=width_multiplier, voxel_resolution_multiplier=voxel_resolution_multiplier
        )
        self.fp_layers = nn.ModuleList(fp_layers)

        channels_fp_features = 644 #1220 #644 #900 #1028 #644 #772 #516 #452 #1476 #1220 #1476 #1220 #1476 #1220 #580
        #layers, _ = create_mlp_components(in_channels=channels_fp_features, out_channels=[128, 0.5, num_classes],
        #layers, _ = create_mlp_components(in_channels=channels_fp_features, out_channels=[512, 0.2, 256, 0.2, 128, num_classes],
        #layers, _ = create_mlp_components(in_channels=channels_fp_features, out_channels=[512, 0.2, 256, 0.2, 128, 0.5, num_classes],
        #layers, _ = create_mlp_components(in_channels=channels_fp_features, out_channels=[512, 0.2, 256, 0.2, 128, num_classes],
        #layers, _ = create_mlp_components(in_channels=channels_fp_features, out_channels=[512, 256, 128, num_classes],
        #layers, _ = create_mlp_components(in_channels=channels_fp_features, out_channels=[1024, 0.2, 512, 0.2, 256, 0.2, 128, num_classes],
        #layers, _ = create_mlp_components(in_channels=channels_fp_features, out_channels=[128, 0.5, num_classes],
        #layers, _ = create_mlp_components(in_channels=channels_fp_features, out_channels=[512, 0.2, 256, 0.2, 128, 0.2, 64, 0.2, 32, num_classes],
        layers, _ = create_mlp_components(in_channels=channels_fp_features, out_channels=[512, 0.2, 256, 0.2, 128, num_classes],
                                       classifier=True, dim=2, width_multiplier=width_multiplier)
       
        self.classifier = nn.Sequential(*layers)

    def forward(self, inputs):
        # inputs : [B, in_channels + S, N]
        features = inputs[:, :self.in_channels, :]
        if self.with_one_hot_shape_id:
            assert inputs.size(1) == self.in_channels + self.num_shapes
            features_with_one_hot_vectors = inputs
        else:
            features_with_one_hot_vectors = features

        coords, features = features[:, :3, :].contiguous(), features[:, 3:, :].contiguous()
        coords_list, in_features_list = [], []
        for sa_module in self.sa_layers:
            in_features_list.append(features)
            coords_list.append(coords)
            #features, coords = sa_module((features, coords))
            features, coords = sa_module((in_features_list[0], coords_list[0]))
            #print("+++++++++++++++++++++++++++++++++++++++++++",in_features_list[-1].shape)
        in_features_list.append(features)
        coords_list.append(coords)

        #print("+++++++++++++++++++++++++++++++++++++++++++",in_features_list[-1].shape)
        in_features_list[0] = features_with_one_hot_vectors.contiguous()
        #print("+++++++++++++++++++++++++++++++++++++++++++",in_features_list[0].shape)

        #for fp_idx, fp_module in enumerate(self.fp_layers):
        #    features, coords = fp_module((coords_list[-1-fp_idx], coords, features, in_features_list[-1-fp_idx]))

        #return self.classifier(features)

        #for ii in range(len(in_features_list)):
            #print("++++++++++++++++++++++++++++++++++++++++++++++++++++++",in_features_list[ii].shape)

        #print("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&",torch.cat( in_features_list, dim=1).shape)
        return self.classifier(torch.cat( in_features_list, dim=1))

class PointNet2SSG(PointNet2):
    sa_blocks = [

        (None, (512, 0.02, 64, (64, 64, 128))),
        (None, (128, 0.04, 64, (128, 128, 256))),
        (None, (None, None, None, (256, 512, 1024))),



        (None, (512, 0.2, 64, (64, 64, 128))),
        (None, (128, 0.4, 64, (128, 128, 256))),
        (None, (None, None, None, (256, 512, 1024))),
    ]
    fp_blocks = [((256, 256), None), ((256, 128), None), ((128, 128, 128), None)]

    def __init__(self, num_classes, num_shapes, extra_feature_channels=3, width_multiplier=1,
                 voxel_resolution_multiplier=1):
        super().__init__(
            num_classes=num_classes, num_shapes=num_shapes, sa_blocks=self.sa_blocks, fp_blocks=self.fp_blocks,
            with_one_hot_shape_id=False, extra_feature_channels=extra_feature_channels,
            width_multiplier=width_multiplier, voxel_resolution_multiplier=voxel_resolution_multiplier
        )


#(None, (10240, [0.02, 0.04, 0.08], [32, 64, 128], [(32, 32, 64), (64, 64, 128), (64, 96, 128)])),
#(None, (5120, [0.08, 0.12], [64, 128], [(128, 128, 256), (128, 196, 256)])),
#(None, (1240, [0.2, 0.6], [64, 128], [(128, 128, 256), (128, 196, 256)])),
#(None, (None, None, None, (256, 512, 1024))),




class PointNet2MSG(PointNet2):
    sa_blocks = [


        #(None, (1024, [0.01], [64], [(64, 64, 128)])),       ### miou =  0.818210 # in_channels = 644 #  [512, 0.2, 256, 0.2, 128, 0.2, 64, 0.2, 32, num_classes]
        #(None, (512, [0.05], [ 128], [(128, 128, 256)])),
        #(None, (256, [0.3], [ 512], [(128, 128, 256)])),


        #(None, (1024, [0.01], [64], [(64, 64, 128)])),       ### miou =  0.820300 # in_channels = 644 # [128, 0.5, num_classes]
        #(None, (512, [0.05], [ 128], [(128, 128, 256)])),
        #(None, (256, [0.3], [ 512], [(128, 128, 256)])),


        (None, (1024, [0.01], [64], [(64, 64, 128)])),       ### miou = 0.836285  # in_channels = 644 # [512, 0.2, 256, 0.2, 128,  num_classes]
        (None, (512, [0.05], [ 128], [(128, 128, 256)])),    ### miou = 0.839855 on V100 node ### 84.2 on testing
        (None, (256, [0.3], [ 512], [(128, 128, 256)])),


        #(None, (1024, [0.01], [64], [(64, 64, 128)])),       ### miou = 0.833446  # in_channels = 900 # [512, 0.2, 256, 0.2, 128,  num_classes]
        #(None, (512, [0.05], [ 128], [(128, 128, 256)])),
        #(None, (256, [0.3], [ 512], [(128, 256, 512)])),


        #(None, (1024, [0.01], [64], [(64, 64, 128)])),       ### miou =  0.828747 # in_channels = 900 # [512, 0.2, 256, 0.2, 128, 0.5,  num_classes]
        #(None, (512, [0.05], [ 128], [(128, 128, 256)])),
        #(None, (256, [0.3], [ 512], [(128, 256, 512)])),


        #(None, (1024, [0.01], [64], [(64, 64, 128)])),       ### miou =  0.834413 # in_channels = 644 # [512, 0.2, 256, 0.2, 128, 0.5,  num_classes]
        #(None, (512, [0.05], [ 128], [(128, 128, 256)])),
        #(None, (256, [0.3], [ 512], [(128, 128, 256)])),


        #(None, (1024, [0.01], [64], [(128, 128, 256)])),       ### miou = 0.829419  # in_channels = 772 # [512, 0.2, 256, 0.2, 128,  num_classes]
        #(None, (512, [0.05], [ 128], [(128, 128, 256)])),
        #(None, (512, [0.3], [ 512], [(128, 128, 256)])),

        #(None, (1024, [0.01], [64], [(64, 64, 128)])),       ### miou = 0.831750  # in_channels = 516 # [512, 0.2, 256, 0.2, 128,  num_classes]
        #(None, (512, [0.05], [ 128], [(64, 64, 128)])),
        #(None, (512, [0.3], [ 512], [(128, 128, 256)])),


        #(None, (10240, [0.01], [64], [(32, 32, 64)])),       ### miou = 0.789521   # in_channels = 452 # [512, 0.2, 256, 0.2, 128,  num_classes]
        #(None, (512, [0.05], [ 128], [(64, 64, 128)])),
        #(None, (512, [0.3], [ 512], [(128, 128, 256)])),


        #(None, (1024, [0.01], [32], [(32, 32, 64)])),       ### miou = 0.837567  # in_channels = 1476 # [512, 0.2, 256, 0.2, 128,  num_classes]
        #(None, (512, [0.02], [ 64], [(64, 64, 128)])),
        #(None, (512, [0.04], [ 128], [(128, 128, 256)])),
        #(None, (256, [0.08], [ 128], [(128, 128, 256)])),
        #(None, (256, [0.16], [ 256], [(128, 196, 256)])),
        #(None, (256, [0.32], [ 256], [(128, 196, 256)])),
        #(None, (256, [0.64], [ 1024], [(128, 196, 256)])),


        #(None, (1024, [0.01], [32], [(32, 32, 64)])),       ### miou = 0.838583  # in_channels = 1476 # [512, 0.2, 256, 0.2, 128,  num_classes]
        #(None, (512, [0.02], [ 64], [(64, 64, 128)])),
        #(None, (512, [0.04], [ 128], [(128, 128, 256)])),
        #(None, (256, [0.08], [ 128], [(128, 128, 256)])),
        #(None, (256, [0.16], [ 256], [(128, 196, 256)])),
        #(None, (256, [0.32], [ 256], [(128, 196, 256)])),
        #(None, (256, [0.64], [ 512], [(128, 196, 256)])),



        #(None, (1024, [0.01], [32], [(32, 32, 64)])),       ### miou = 0.837896  # in_channels = 1476 # [512, 0.2, 256, 0.2, 128,  num_classes]
        #(None, (512, [0.02], [ 64], [(64, 64, 128)])),
        #(None, (512, [0.04], [ 128], [(128, 128, 256)])),
        #(None, (256, [0.08], [ 128], [(128, 128, 256)])),
        #(None, (256, [0.16], [ 256], [(128, 196, 256)])),
        #(None, (256, [0.32], [ 256], [(128, 196, 256)])),
        #(None, (256, [0.64], [ 256], [(128, 196, 256)])),


        #(None, (1024, [0.02], [32], [(32, 32, 64)])),       ### miou = 0.806713 # in_channels = 1220 # [128, 0.5, num_classes]
        #(None, (512, [0.04], [ 64], [(64, 64, 128)])),
        #(None, (512, [0.06], [ 128], [(128, 128, 256)])),
        #(None, (256, [0.08], [ 128], [(128, 128, 256)])),
        #(None, (256, [0.12], [ 256], [(128, 196, 256)])),
        #(None, (256, [0.2], [ 256], [(128, 196, 256)])),


        #(None, (1024, [0.02], [32], [(32, 32, 64)])),       ### miou = 0.833496 # in_channels = 1220 # [1024, 0.2, 512, 0.2, 256, 0.2, 128,  num_classes]
        #(None, (512, [0.04], [ 64], [(64, 64, 128)])),
        #(None, (512, [0.08], [ 128], [(128, 128, 256)])),
        #(None, (256, [0.12], [ 128], [(128, 128, 256)])),
        #(None, (256, [0.2], [ 256], [(128, 196, 256)])),
        #(None, (256, [0.6], [ 256], [(128, 196, 256)])),


        #(None, (1024, [0.02], [32], [(32, 32, 64)])),       ### miou = 0.822843 # in_channels = 1220 # [512, 256, 128,  num_classes]
        #(None, (512, [0.04], [ 64], [(64, 64, 128)])),
        #(None, (512, [0.08], [ 128], [(128, 128, 256)])),
        #(None, (256, [0.12], [ 128], [(128, 128, 256)])),
        #(None, (256, [0.2], [ 256], [(128, 196, 256)])),
        #(None, (256, [0.6], [ 256], [(128, 196, 256)])),



        #(None, (1024, [0.02], [32], [(32, 32, 64)])),       ### miou = 0.841189 # in_channels = 1220 # [512, 0.2, 256, 0.2, 128,  num_classes]
        #(None, (512, [0.04], [ 64], [(64, 64, 128)])),
        #(None, (512, [0.08], [ 128], [(128, 128, 256)])),
        #(None, (256, [0.12], [ 128], [(128, 128, 256)])),
        #(None, (256, [0.2], [ 256], [(128, 196, 256)])),
        #(None, (256, [0.6], [ 256], [(128, 196, 256)])),



        #(None, (1024, [0.02], [32], [(32, 32, 64)])),       ### miou = 0.787748 # in_channels = 1220 # [512, 0.2, 256, 0.2, 128, 0.5, num_classes]
        #(None, (512, [0.04], [ 64], [(64, 64, 128)])),
        #(None, (512, [0.08], [ 128], [(128, 128, 256)])),
        #(None, (256, [0.12], [ 128], [(128, 128, 256)])),
        #(None, (256, [0.2], [ 256], [(128, 196, 256)])),
        #(None, (256, [0.6], [ 256], [(128, 196, 256)])),


        #(None, (10240, [0.02], [32], [(32, 32, 64)])),       ### miou = 0.833550   # in_channels= 1220
        #(None, (10240, [0.04], [ 64], [(64, 64, 128)])),
        #(None, (5120, [0.08], [ 64], [(128, 128, 256)])),
        #(None, (5120, [0.12], [ 128], [(128, 196, 256)])),
        #(None, (1240, [0.2], [ 64], [(128, 128, 256)])),
        #(None, (1240, [0.6], [ 128], [(128, 196, 256)])),
       



        #(None, (1024, [0.02], [32], [(32, 32, 64)])),       ### miou = 0.794321   # in_channels= 1476
        #(None, (512, [0.04], [ 64], [(64, 64, 128)])),
        #(None, (512, [0.08], [ 128], [(128, 128, 256)])),
        #(None, (256, [0.12], [ 128], [(128, 128, 256)])),
        #(None, (256, [0.2], [ 256], [(128, 196, 256)])),
        #(None, (128, [0.6], [ 256], [(128, 196, 256)])),
        #(None, (128, [1.2], [ 512], [(128, 196, 256)])),


        #(None, (1024, [0.02], [32], [(32, 32, 64)])),       ### miou = 0.836379 # in_channels = 1220
        #(None, (512, [0.04], [ 64], [(64, 64, 128)])),
        #(None, (512, [0.08], [ 128], [(128, 128, 256)])),
        #(None, (256, [0.12], [ 128], [(128, 128, 256)])),
        #(None, (256, [0.2], [ 256], [(128, 196, 256)])),
        #(None, (256, [0.6], [ 256], [(128, 196, 256)])),



        #(None, (1024, [0.02], [32], [(32, 32, 64)])),       ### miou = 0.808220 # in_channels = 1476
        #(None, (512, [0.04], [ 64], [(64, 64, 128)])),
        #(None, (512, [0.08], [ 128], [(128, 128, 256)])),
        #(None, (256, [0.12], [ 128], [(128, 128, 256)])),
        #(None, (256, [0.2], [ 256], [(128, 196, 256)])),
        #(None, (256, [0.6], [ 256], [(128, 196, 256)])),
        #(None, (128, [1.2], [ 256], [(128, 196, 256)])),



        #(None, (10240, [0.02], [32], [(32, 32, 64)])),   ### miou = 0.828690 in 111 epochs  compared to 83.-- for previous setting in 111 epochs ## in_channels = 1220  
        #(None, (5120, [0.04], [ 64], [(64, 64, 128)])),
        #(None, (5120, [0.08], [ 128], [(128, 128, 256)])),
        #(None, (2560, [0.12], [ 128], [(128, 128, 256)])),
        #(None, (2560, [0.2], [ 256], [(128, 196, 256)])),
        #(None, (2560, [0.6], [ 256], [(128, 196, 256)])),


        #(None, (1024, [0.02], [32], [(32, 32, 64)])),       ### miou = 0.840688 # in_channels = 1220
        #(None, (512, [0.04], [ 64], [(64, 64, 128)])),
        #(None, (512, [0.08], [ 128], [(128, 128, 256)])),
        #(None, (256, [0.12], [ 128], [(128, 128, 256)])),
        #(None, (256, [0.2], [ 256], [(128, 196, 256)])),
        #(None, (256, [0.6], [ 256], [(128, 196, 256)])),
        

        #(None, (1024, [0.02 ], [32], [(32, 32, 64)])),    ### miou = 80.2 # in_channels = 580
        #(None, (512, [0.08], [ 128], [(128, 128, 256)])),
        #(None, (124, [0.2], [ 128], [(128, 196, 256)])),
 

        #(None, (1024, [0.02, 0.04, 0.08], [320, 640, 1280], [(32, 32, 64), (64, 64, 128), (64, 96, 128)])),
        #(None, (512, [0.08, 0.12], [640, 1280], [(128, 128, 256), (128, 196, 256)])),
        #(None, (124, [0.2, 0.6], [640, 1280], [(128, 128, 256), (128, 196, 256)])),
        #(None, (None, None, None, (256, 512, 1024))),
      
        #### Final
        #(None, (10240, [0.02, 0.04, 0.08], [32, 64, 128], [(32, 32, 64), (64, 64, 128), (64, 96, 128)])),
        #(None, (5120, [0.08, 0.12], [64, 128], [(128, 128, 256), (128, 196, 256)])),
        #(None, (1240, [0.2, 0.6], [64, 128], [(128, 128, 256), (128, 196, 256)])),
        #(None, (None, None, None, (256, 512, 1024))),


        #(None, (10240, [0.02, 0.04, 0.08], [32, 64, 128], [(32, 32, 64), (64, 64, 128), (64, 96, 128)])),
        #(None, (5120, [0.08, 0.12], [64, 128], [(128, 128, 256), (128, 196, 256)])),
        #(None, (2560, [0.2, 0.5], [64, 128], [(128, 128, 256), (128, 196, 256)])),
        #(None, (1280, [0.6, 1.1], [64, 128], [(128, 128, 256), (128, 196, 256)])),
        #(None, (None, None, None, (256, 512, 1024))),


        #(None, (1024, [0.02, 0.04, 0.08], [32, 64, 128], [(32, 32, 64), (64, 64, 128), (64, 96, 128)])),
        #(None, (512, [0.08, 0.12], [64, 128], [(128, 128, 256), (128, 196, 256)])),
        #(None, (124, [0.2, 0.6], [64, 128], [(128, 128, 256), (128, 196, 256)])),
        #(None, (None, None, None, (256, 512, 1024))),


        #(None, (10240, [0.02, 0.04, 0.08], [32, 64, 128], [(32, 32, 64), (64, 64, 128), (64, 96, 128)])),
        #(None, (5120, [0.08, 0.12], [64, 128], [(128, 128, 256), (128, 196, 256)])),
        #(None, (1240, [0.2, 0.6], [64, 128], [(128, 128, 256), (128, 196, 256)])),
        #(None, (None, None, None, (256, 512, 1024))),


        #(None, (512, [0.02, 0.04, 0.08], [32, 64, 128], [(32, 32, 64), (64, 64, 128), (64, 96, 128)])),
        #(None, (128, [0.08, 0.12], [64, 128], [(128, 128, 256), (128, 196, 256)])),
        #(None, (None, None, None, (256, 512, 1024))),


        #(None, (512, [0.1, 0.2, 0.4], [32, 64, 128], [(32, 32, 64), (64, 64, 128), (64, 96, 128)])),
        #(None, (128, [0.4, 0.8], [64, 128], [(128, 128, 256), (128, 196, 256)])),
        #(None, (None, None, None, (256, 512, 1024))),
    ]

    fp_blocks = [((256, 256), None)]
    ####fp_blocks = [((256, 256), None), ((256, 256), None), ((256, 128), None), ((128, 128, 128), None)]

    #fp_blocks = [((256, 256), None), ((256, 256), None), ((256, 256), None), ((256, 128), None), ((128, 128, 128), None)]
    #fp_blocks = [((256, 256), None), ((256, 256), None), ((256, 128), None), ((128, 128, 128), None)]
    #fp_blocks = [((256, 256), None), ((256, 128), None), ((128, 128, 128), None)]

    def __init__(self, num_classes, num_shapes, extra_feature_channels=3, width_multiplier=1,
                 voxel_resolution_multiplier=1):
        super().__init__(
            num_classes=num_classes, num_shapes=num_shapes, sa_blocks=self.sa_blocks, fp_blocks=self.fp_blocks,
            with_one_hot_shape_id=True, extra_feature_channels=extra_feature_channels,
            width_multiplier=width_multiplier, voxel_resolution_multiplier=voxel_resolution_multiplier
        )
