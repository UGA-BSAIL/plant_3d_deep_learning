rng(0);
s = rng;

root = "E:\Work\Daily_work\20210707\inferences\denormalization_pvcnn\post_processed_POS1\";
% % root = "E:\Work\Daily_work\20210823\postprocessing\inferences\denormalization_mod_pnpp_final\post_processed_POS1\";

fnames = ["denorm_0.txt","denorm_1.txt","denorm_2.txt","denorm_3.txt","denorm_4.txt","denorm_5.txt","denorm_6.txt","denorm_7.txt","denorm_8.txt","denorm_9.txt"];
% fnames = ["denorm_1.txt"];
%     
% root = "E:\Work\Daily_work\20210707\inferences\denormalization_pvcnn\";
% root = "E:\Work\Daily_work\20210823\postprocessing\inferences\denormalization_mod_pnpp_final\";
% fnames = ["gt_denorm_0.txt","gt_denorm_1.txt","gt_denorm_2.txt","gt_denorm_3.txt","gt_denorm_4.txt","gt_denorm_5.txt","gt_denorm_6.txt","gt_denorm_7.txt","gt_denorm_8.txt"];
% fnames = ["gt_denorm_1.txt"];
result_vals1 = [];
% k=6;

for k = 1:1
% read point cloud from file
T = readtable(strcat(root, fnames(1,k)));
input_pc = pointCloud(T{:,1:3}, 'Color', T{:,4:6});
figure; pcshow(input_pc); title('Input point cloud');

% remove all points in top 10 cm of the plant
index = find(T{:,3} > max(T{:,3}) - 0.2 );
T(index,: ) = [] ; %1:3};
%figure; pcshow(T{:,1:3}, T{:,4:6}); title('remove all points in top 10 cm of the plant');

% get the red points
index = find(T{:,4} == 1); % | T{:,6} == 1 );
red_pts = T{index,1:3};
red_clr = T{index,4:6};
%figure; pcshow(red_pts, red_clr); title('Red points');

% get the green points
index = find(T{:,5} == 1); % | T{:,6} == 1 );
green_pts = T{index,1:3};
green_clr = T{index,4:6};
%figure; pcshow(green_pts, green_clr); title('Green points');

% get the blue points
index = find(T{:,6} == 1); % | T{:,6} == 1 );
blue_pts = T{index,1:3};
blue_clr = T{index,4:6};
%figure; pcshow(blue_pts, blue_clr); title('Blue points');
% 
% pc = pcdenoise(pointCloud(blue_pts,'Color',blue_clr));
% blue_pts = pc.Location;
% blue_clr = pc.Color;
%%%% Clustering on blue points
minDistance = 0.005; minPoints = 100;
ptCloud = pointCloud(blue_pts, 'Color',blue_clr );
[labels,numClusters] = pcsegdist(ptCloud,minDistance,'NumClusterPoints',minPoints);
%figure; pcshow(ptCloud.Location,labels)
colormap(hsv(numClusters)); title('all labels'); hold off;

disp('nummmmmmm:')
numClusters
%%% valid points and labels
idxValidPoints = find(labels);
labelColorIndex = labels(idxValidPoints);
segmentedPtCloud = select(ptCloud,idxValidPoints);
figure;
% pcshow(input_pc);hold on;
colormap(hsv(numClusters));
pcshow(segmentedPtCloud.Location,labelColorIndex); title('valid labels');hold on;
valid_pts = segmentedPtCloud.Location;
valid_clr = segmentedPtCloud.Color;
% figure; pcshow(valid_pts, valid_clr);

%%% remove cotton bolls with less than 12cm height
new_pts= segmentedPtCloud.Location;
new_labels = labelColorIndex;
nc = numClusters;
for c=1:numClusters
idx = find(new_labels == c);
min_z = min(new_pts(idx,3));
max_z = max(new_pts(idx,3));
max_z-min_z
if max_z-min_z<0.03
    nc = nc-1;
new_pts(idx,:) = [];
new_labels(idx,:) = [];
end
end

new_clr = zeros(size(new_pts));
for c=1:numClusters
   
idx = find(new_labels == c);
new_clr(idx,:)  = repmat([rand(), rand(), rand()],size(idx,1),1);
end
figure; pcshow(new_pts,new_clr); title('here it is')

% Frequency of each label 
figure;
% h = histogram(new_labels);
h = histogram(labelColorIndex);
freq = h.Values;
TF = isoutlier(double(freq),'quartiles');
outlier_elements = freq(TF);
counts = round(freq(TF) / mean(freq));
n = sum(counts-1) + nc; %numClusters;
result_vals1 = [result_vals1  n];


Lia = ismember(new_labels,find(TF)); 
new_clr(Lia,:) = repmat([1,1,1],size(new_clr(Lia,:),1),1);
figure; pcshow(new_pts,new_clr); title('final');

end
%%%%%%%%%% after segmentation select a cluster and fit sphere on it
% maxDistance = 0.01
% idx = find(labelColorIndex  == 8);
% cluster_pts = segmentedPtCloud.Location(idx,:);
% cluster_clr = segmentedPtCloud.Color(idx,:);
% cluster_cloud = pointCloud(cluster_pts, 'Color', cluster_clr);
% [model,inlierIndices, outlierIndices] = pcfitsphere(cluster_cloud,maxDistance);
% globe = select(cluster_cloud,inlierIndices);
% 
% %figure; pcshow(cluster_cloud); title('sample');
% % hold on
% % plot(model)
% nc = 2;
% [idx,C] = kmeans(cluster_pts,nc, 'Distance', 'cityblock');
