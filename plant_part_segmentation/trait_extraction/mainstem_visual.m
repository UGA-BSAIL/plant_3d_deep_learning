
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% random seed
rng(0);
s = rng;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% file path
root = "E:\Work\Daily_work\20210707\inferences\denormalization_pvcnn\post_processed_POS1\";
root = "E:\Work\Daily_work\20210823\postprocessing\inferences\denormalization_mod_pnpp_final\post_processed_POS1\";
fnames = ["denorm_0.txt","denorm_1.txt","denorm_2.txt","denorm_3.txt","denorm_4.txt","denorm_5.txt","denorm_6.txt","denorm_7.txt","denorm_8.txt","denorm_9.txt"];

% root = "E:\Work\Daily_work\20210707\inferences\denormalization_pvcnn\";
% fnames = ["gt_denorm_0.txt","gt_denorm_1.txt","gt_denorm_2.txt","gt_denorm_3.txt","gt_denorm_4.txt","gt_denorm_5.txt","gt_denorm_6.txt","gt_denorm_7.txt","gt_denorm_8.txt"];


heights = [];
diameters = [];

for k=6:6
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% visualize point
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% cloud
T = readtable(strcat(root, fnames(1,k)));
pc = pointCloud(T{:,1:3},'Color',T{:,4:6});
figure; 
pcshow(pc);title('Input pc');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% visualize red
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% points
idxRed_pts = find(T{:,4} == 1);
red_pts = T{idxRed_pts, 1:3};
red_clr = T{idxRed_pts, 4:6};
figure;
pcshow(red_pts,red_clr);title('Red pts pc');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% height of main stem
min_z = min(red_pts(:,3));
max_z = max(red_pts(:,3));
height = max_z - min_z
heights =[heights height];
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% diameter of main stem
index = find(red_pts(:,3) >= min_z & red_pts(:,3) < min_z+0.01);
figure; pcshow(red_pts(index,:),red_clr(index,:)); title('selected points');
rootPt2 = red_pts(index,:);

rootPtXY = rootPt2(:,[1:2]);
Par = CircleFitByPratt(rootPtXY);
radius = Par(3);
figure
plot(rootPtXY(:,1),rootPtXY(:,2),'b.');
hold on 
plot(Par(1), Par(2), 'r.','markersize', 20);
hold on 
circle(Par(1), Par(2),Par(3));
hold off
title('center point for the first sphere')
diameter = radius*2;

diameters =[diameters diameter];
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% number of branches

end