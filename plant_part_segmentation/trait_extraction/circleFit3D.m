function [r, xyzc, c] = circleFit3D(X)

% X=idCluster{1};
XC = mean(X,1);
Y=X-XC;
[~,~,V]=svd(Y,0);
Q = V(:,[1 2]); % basis of the plane
Y=Y*Q;
xc=Y(:,1);
yc=Y(:,2);
M=[xc.^2+yc.^2,-2*xc,-2*yc];
% Fit ellipse through (xc,yc)
P = M\ones(size(xc));
a=P(1);
P = P/a;
r=sqrt(P(2)^2+P(3)^2+1/a); % radius
xyzc = XC' + Q*P(2:3); % center
theta = linspace(0,2*pi);
c = xyzc + r*Q*[cos(theta); sin(theta)]; % fit circle

% figure
% close all
% plot3(X(:,1),X(:,2),X(:,3),'.');
% hold on
% plot3(c(1,:),c(2,:),c(3,:),'r','LineWidth', 2);
% axis equal