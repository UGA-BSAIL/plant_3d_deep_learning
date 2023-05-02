% set(0,'DefaultFigureVisible','off');
rng(0);
s = rng;

pd_root = "E:\Work\Daily_work\20210707\inferences\denormalization_pvcnn\post_processed_POS1\denorm_";
gt_root = "E:\Work\Daily_work\20210707\inferences\denormalization_pvcnn\gt_denorm_";


% for every branch in the the predicted pointcloud, find the corresponding
% branch in the groundtruth pointcloud
pred_angles=[];
pred_diameters=[];
gt_angles=[];
gt_diameters=[];

for fcount=0:8 %for each point cloud
pd_fname = strcat(pd_root,string(fcount),'.txt');
gt_fname = strcat(gt_root,string(fcount),'.txt');
[numClusters1, labels1, pc1] = traitExtraction_POS_branch_diameter_step1(pd_fname);
[numClusters2, labels2, pc2] = traitExtraction_POS_branch_diameter_step1(gt_fname);

for i=1:numClusters1 % each branch cluster in pred file
idx1 = find(labels1 == i); % i
pts1 = pc1.Location(idx1,:);

for j=1:numClusters2 % find the corresponding branch cluster in ground truth file
    idx2 = find(labels2 == j); % j
    pts2 = pc2.Location(idx2,:);
    cpts = intersect(round(pts1,4), round(pts2,4), 'rows');
    cratio1 = size(cpts,1) / size(pts1,1);
    cratio2 = size(cpts,1) / size(pts2,1);
    if cratio1 > 0.7 & size(pts1,1) > 5 &  size(pts2,1) > 5 
         fprintf('i is %.0f and j is %.0f\n', i, j);
         [angle1, diameter1] = branch_angle_diameter(pts1);
         if diameter1 > 1.5
%              pts = pts1;
             pts1 = pcdenoise(pointCloud(pts1)).Location;
             min_z = min(pts1(:,3));
             bidx = find(pts1(:,3)<min_z+0.02);
             pts = pts1(bidx,:);
             [angle1, diameter1] = branch_angle_diameter(pts);
         end
         
         [angle2, diameter2] = branch_angle_diameter(pts2);
         if diameter2 > 1.5
%              pts = pts2;
             pts2 = pcdenoise(pointCloud(pts2)).Location;
             min_z = min(pts2(:,3));
             bidx = find(pts2(:,3)<min_z+0.02);
             pts = pts2(bidx,:);
             [angle2, diameter2] = branch_angle_diameter(pts);
         end
         
         if angle1 > 70 & angle2 <20
         nan_fname = pd_fname;
         nan_pts1 = pts1;
         nan_pts2 = pts2;
         end
         pred_angles=[pred_angles,angle1];
         pred_diameters=[pred_diameters, diameter1];
         gt_angles=[gt_angles,angle2];
         gt_diameters=[gt_diameters, diameter2];
    end
end
end
end
