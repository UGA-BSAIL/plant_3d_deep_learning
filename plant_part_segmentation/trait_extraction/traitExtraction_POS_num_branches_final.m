rng(0);
s = rng;

root = "E:\Work\Daily_work\20210707\inferences\denormalization_pvcnn\post_processed_POS1\";
fnames = ["denorm_0.txt","denorm_1.txt","denorm_2.txt","denorm_3.txt","denorm_4.txt","denorm_5.txt","denorm_6.txt","denorm_7.txt","denorm_8.txt","denorm_9.txt","denorm_10.txt"];
% fnames = ["denorm_1.txt"];
%     
% root = "E:\Work\Daily_work\20210707\inferences\denormalization_pvcnn\";
% fnames = ["gt_denorm_0.txt","gt_denorm_1.txt","gt_denorm_2.txt","gt_denorm_3.txt","gt_denorm_4.txt","gt_denorm_5.txt","gt_denorm_6.txt","gt_denorm_7.txt","gt_denorm_8.txt"];
% fnames = ["gt_denorm_1.txt"];

i = 1; %filenumber

result_vals=[];
for k=1:1
% read point cloud from file
T = readtable(strcat(root, fnames(1,k)));
input_pc = pointCloud(T{:,1:3}, 'Color', T{:,4:6});
figure; pcshow(input_pc); title('Input point cloud');

% remove all points in top 10 cm of the plant
index = find(T{:,3} > max(T{:,3}) - 0.2 );
T(index,: ) = [] ; %1:3};
%%%%figure; %pcshow(T{:,1:3}, T{:,4:6})

% get the red points
index = find(T{:,4} == 1); % | T{:,6} == 1 );
red_pts = T{index,1:3};
red_clr = T{index,4:6};
%%%%figure; %pcshow(red_pts, red_clr); title('Red points');

% get the green points
index = find(T{:,5} == 1); % | T{:,6} == 1 );
green_pts = T{index,1:3};
green_clr = T{index,4:6};
%%%%figure; %pcshow(green_pts, green_clr); title('Green points');

% get the blue points
index = find(T{:,6} == 1); % | T{:,6} == 1 );
blue_pts = T{index,1:3};
blue_clr = T{index,4:6};
%%%%figure; %pcshow(blue_pts, blue_clr); title('Blue points');

%%% remove green points immediately near by red points
bdists = pdist2(green_pts, red_pts);
%%% get green points nearby red points
[rows, cols] = find( bdists<0.01);
rows1 = unique(rows);
bdists(rows1,:) = [] ;
green_pts(rows1,:) = [];
green_clr(rows1,:) = [];

% Select green and blue points and segment them
% Remove the small segments
green_blue_pts = [green_pts; blue_pts];
green_blue_clr = [green_clr; blue_clr];
%%%%figure; %pcshow(green_blue_pts, green_blue_clr); title('Green and Blue points');

%%% remove branches with small clusters
%%% segmenting green points based on minDistance threshold (it is connected comps)
minDistance = 0.02;
minPoints = 100;
pc = pointCloud( green_blue_pts, 'Color', green_blue_clr );
[labels,numClusters] = pcsegdist(pc,minDistance,'NumClusterPoints',minPoints);
figure; pcshow(pc.Location,labels)
colormap(hsv(numClusters)); title('all labels');
%%% valid points and labels
idxValidPoints = find(labels);
labelColorIndex = labels(idxValidPoints);
segmentedPtCloud = select(pc,idxValidPoints);
figure;colormap(hsv(numClusters));
pcshow(segmentedPtCloud.Location,labelColorIndex); title('valid labels');
valid_pts = segmentedPtCloud.Location;
valid_clr = segmentedPtCloud.Color;

% get nearby green points that are also in valid points

%%% get green points nearby red points
[rows, cols] = find(bdists<0.03); %0.025:old2 ,,  0.05:old1
rows1 = unique(rows);
near_bpts = green_pts(rows1,:);
near_bpts_clr = green_clr(rows1,:);
figure; pcshow(near_bpts,near_bpts_clr); title('all nearby points');
[Lia,Locb] = ismember(near_bpts,valid_pts, 'rows');
near_bpts = near_bpts(Lia, :);
near_bpts_clr = near_bpts_clr(Lia, :);
figure; pcshow(near_bpts,near_bpts_clr); title('valid nearby points');

%%% cluster the near by points
minDistance = 0.02;
% minPoints = 100;
pc = pointCloud( near_bpts, 'Color', near_bpts_clr );
[labels,numClusters] = pcsegdist(pc,minDistance); %,'NumClusterPoints',minPoints);
figure; pcshow(pc.Location,labels)
colormap(hsv(numClusters)); title('near by clusters');
result_vals = [result_vals numClusters];
end