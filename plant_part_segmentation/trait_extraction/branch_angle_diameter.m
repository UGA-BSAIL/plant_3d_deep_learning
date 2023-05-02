function [angle, diameter] = branch_angle_diameter(pts1,s)
% s=0
% pc = pointCloud(pts1);
% pc = pcdenoise(pc);
% 
% minDistance = 0.001;
% minPoints = 20;
% [labels,numClusters] = pcsegdist(pc,minDistance,'NumClusterPoints',minPoints);
% idxValidPoints = find(labels);
% labelColorIndex = labels(idxValidPoints);
% segmentedPtCloud = select(pc,idxValidPoints);
% h = histogram(labelColorIndex);
% [argvalue, argmax] = max(h.Values);
% 
% idx = find(labelColorIndex == argmax);
% 
% pts1 = segmentedPtCloud.Location(idx,:);
%figure; pcshow(pts1); title('input pc');

if (s==1)
pclr = repmat([0,1,0],size(pts1,1),1);
figure; pcshow(pts1,pclr, 'MarkerSize',100); title('input');
% pcshow(pc, 'MarkerSize',500);
end

coeff = pca(pts1);
pc1 = coeff(:,1)';
pcPro = [pc1(1),pc1(2),0];

angle = rad2deg(atan2(norm(cross(pc1,pcPro)), dot(pc1,pcPro)));
fprintf('angle is %.0f \n', angle);
         
% vectarrow(pcPro,pc1);
%hold on;
%pcshow(pc1,'MarkerSize',1000); hold on;
%pcshow(pcPro,'MarkerSize',1000);hold off;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
GG = @(A,B) [ dot(A,B) -norm(cross(A,B)) 0;
              norm(cross(A,B)) dot(A,B)  0;
              0              0           1];
FFi = @(A,B) [ A (B-dot(A,B)*A)/norm(B-dot(A,B)*A) cross(B,A) ];
UU = @(Fi,G) Fi*G*inv(Fi);
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


% rotating a onto b %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% a=[1 0 0]'; b=[0 1 0]';
% U = UU(FFi(a,b), GG(a,b));
% norm(U) % is it length-preserving?
% % ans = 1
% norm(b-U*a) % does it rotate a onto b?
% % ans = 0
% %  U
% % U =
% % 
% %    0  -1   0
% %    1   0   0
% %    0   0   1

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% rotating pc1 on to z axis
a=pc1'; b=[0 0 1]';
U = UU(FFi(a,b), GG(a,b));
pts1r = transpose(U*transpose(pts1));
if s ==1
pclr = repmat([0,1,0],size(pts1r,1),1);
figure;pcshow(pts1r,pclr, 'MarkerSize',100); title('rotated');
end

min_z = min(pts1r(:,3));
index = find(pts1r(:,3) >= min_z & pts1r(:,3) < min_z+0.01);
if size(index,1) < 5
index = find(pts1r(:,3) >= min_z);
end
rootPt2 = pcdenoise(pointCloud(pts1r(index,:))).Location;
if s ==1
figure; pcshow(rootPt2); title('selected points');
end


rootPtXY = rootPt2(:,[1:2]);
Par = CircleFitByPratt(rootPtXY);
radius = Par(3);
if s == 1
figure
plot(rootPtXY(:,1),rootPtXY(:,2),'b.');
hold on 
plot(Par(1), Par(2), 'r.','markersize', 20);
hold on 
circle(Par(1), Par(2),Par(3));
hold off
title('center point for the first sphere')
end
diameter = (radius*2)*100;
if diameter > 2
diameter=(max(pts1r(:,2))- min(pts1r(:,2)))*100;
end
diameterdist =(max(pts1r(:,2))- min(pts1r(:,2)))*100;
if diameterdist < diameter
diameter = diameterdist;
end
diameterdist =(max(pts1r(:,1))- min(pts1r(:,1)))*100;
if diameterdist < diameter
diameter = diameterdist;
end

% fprintf('diameterdist is %.4f \n', diameterdist);

fprintf('diameter is %.4f \n', diameter);
         
% if diameter > 2
% diameter = 2;
% end
% 
% diameter=(max(pts1r(:,2))- min(pts1r(:,2)))*100;
% fprintf('diameter is %.4f \n', diameter);
         
end