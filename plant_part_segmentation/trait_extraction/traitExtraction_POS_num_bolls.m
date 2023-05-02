rng(0);
s = rng;

root = "E:\Work\Daily_work\20210707\inferences\denormalization_pvcnn\post_processed_POS1\";
fnames = ["denorm_0.txt","denorm_1.txt","denorm_2.txt","denorm_3.txt","denorm_4.txt","denorm_5.txt","denorm_6.txt","denorm_7.txt","denorm_8.txt"];
fnames = ["denorm_8.txt"];
    
% root = "E:\Work\Daily_work\20210707\inferences\denormalization_pvcnn\";
% fnames = ["gt_denorm_0.txt","gt_denorm_1.txt","gt_denorm_2.txt","gt_denorm_3.txt","gt_denorm_4.txt","gt_denorm_5.txt","gt_denorm_6.txt","gt_denorm_7.txt","gt_denorm_8.txt"];
% fnames = ["gt_denorm_8.txt"];

i = 1; %filenumber

result_vals=[];
for k=1:1
% read point cloud from file
T = readtable(strcat(root, fnames(1,k)));
input_pc = pointCloud(T{:,1:3}, 'Color', T{:,4:6});
figure; pcshow(input_pc); title('Input point cloud');

% get the blue points
index = find(T{:,6} == 1); % | T{:,6} == 1 );
blue_pts = T{index,1:3};
blue_clr = T{index,4:6};
figure; pcshow(blue_pts, blue_clr); title('Blue points');


%%% segmenting green points based on minDistance threshold (it is connected comps)
minDistance = 0.005;
minPoints = 100;
pc = pointCloud( blue_pts, 'Color', blue_clr );
[labels,numClusters] = pcsegdist(pc,minDistance,'NumClusterPoints',minPoints);
figure; pcshow(pc.Location,labels)
colormap(hsv(numClusters)); title('all labels');

%%% valid points and labels
idxValidPoints = find(labels);
labelColorIndex = labels(idxValidPoints);
segmentedPtCloud = select(pc,idxValidPoints);
figure;colormap(hsv(numClusters));
pcshow(segmentedPtCloud.Location,labelColorIndex); title('valid labels');
end
