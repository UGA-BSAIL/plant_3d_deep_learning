rng(0);
s = rng;

figure; pcshow(pc.Location,labels); % this is from trait Extraction num_branches_final
colormap(hsv(numClusters)); title('near by clusters');
clr = zeros(size(pc.Location));
for i=1:numClusters
    idx = find(labels == i);
 clr(idx,:) = repmat([rand() rand() rand()], size(idx,1),1);
end
figure;
pcshow(pc.Location, clr);

for i = 1: 1 %numClusters
idx = find(labels == i);
% figure;
% pcshow(pc.Location(idx,:), clr(idx,:));
% title('idx');
spherePt = pc.Location(idx,:);
coeff = pca(spherePt);
pc1 = coeff(:,1)';
pcPro = [pc1(1),pc1(2),0];
A = mean(spherePt,1);
center = A;
B = pc1 + A;       
angle = rad2deg(atan2(norm(cross(pc1,pcPro)), dot(pc1,pcPro)));
fprintf('the angle betwen PC1 and xy plane is: %.3f\n',angle);
theta = 90-angle;
x = pc.Location(idx,1); y = pc.Location(idx,2); z = pc.Location(idx,3);
X = x*cos(theta) + z*sin(theta);
Y = y;
Z = z*cos(theta) - x*sin(theta);
% X = x;
% Y = y*cos(theta) - z*sin(theta);
% Z = y*sin(theta) + z*cos(theta);
pts = [X Y Z];
diameter=sprintf(' %.2f', (max(X)- min(X))*100);
angle=sprintf(' %.2f',angle);
% figure();
% pcshow([X Y Z],clr(idx,:));
pc1 = pointCloud([X Y Z],'Color',clr(idx,:));
pc2 = pointCloud(pc.Location(idx,:),'Color',clr(idx,:));
% figure;pcshow(pc1);
figure;pcshow(pc2);
title(strcat('diameter ',diameter, 'cm angle ', angle ));
end
