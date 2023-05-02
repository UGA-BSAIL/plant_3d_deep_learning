% set(0,'DefaultFigureVisible','on');

% main stem diameter
y = [0.0183;0.0243;0.031;0.0233;0.0188;0.014;0.0202;0.0133;0.0194].*100; %Population of states
x = [0.0183;0.0245;0.031;0.0233;0.0188;0.0141;0.0203;0.0133;0.0194].*100; %Accidents per state


% main stem height
 x = [0.9376;1.1803;1.333;1.1903;1.2401;1.0178;1.14;1.1239;1.3208];
 y = [0.939;1.2808;1.333;1.3867;1.2401;1.0178;1.14;1.1239;1.3208];
% 
% % number of nodes
% x = [9;16;13;16;14;9;12;10;13];
% y = [10;15;14;16;15;11;15;10;13];
% x = [9;15;13;16;14;9;12;10;13];
% y = [10;15;13;16;15;10;15;11;13];

x = [14;21;16;19;15;12;20;11;14];
y = [14;22;16;19;16;14;18;10;14];

% internode distance

%Main stem height
x = [0.91772507;1.280808534;1.332961588;1.328408842;1.2400882;1.017803348;1.139996353;1.123916608; 1.320795961];
y = [0.93899224;1.280808534;1.332961588;1.386716516;1.2400882;1.017803348;1.139996353;1.123916608; 1.320795961];
% Main stem diameter								
% x = [001.7656157;	002.4450814;	003.2081992;	002.3549039;	001.8101904;	001.4376209;	001.9045174;	001.2504682; 001.9441313];
% y = [001.7656157;	002.4450814;	003.2081992;	002.3549039;	001.8101904;	001.4376209;	001.9045174;	001.2504682; 001.9441313];
% Number of nodes
% x = [11;	13;	13;	14;	13;	10;	12;	10; 12];
% y = [11;	15;	14;	15;	12;	10;	12;	10; 12];
% % Number of branches
% x = [12; 17;16;	18;	13;	11;	16;	11; 13];
% y = [12;19;	16;	18;	13;	13;	15;	11; 13];


% number of bolls
% x = [18;22;20;21;19;18;22;13;16];
% y = [18;21;16;20;11;16;20;16;15];
x = [39;	80;	107;	27;	13;	20;	27;	17;	29];
y = [39;	75;	111; 27;	13;	18;	28;	17;	28];

% x=pred_angles';
% y=gt_angles';
% % % 
% x=(pred_diameters)';
% y=(gt_diameters)';

% Number of branches
x = [14; 19;	18;	19;	12;	11;	16;	11;	14];
y = [14; 21;	18;	19;	12;	13;	15;	11;	14];


% Number of nodes
x = [13;	14;	15;	15;	12;	9;	12;	10;	13];
y = [13;	16;	16;	16;	11;	10;	12;	10;	13];


format long
b1 = x\y;

yCalc1 = b1*x;
scatter(x,y,'MarkerFaceColor',[.0 .0 1.0])
hold on


%plot(x,yCalc1)
ylabel('Ground truth')
xlabel('Prediction')
% title('Linear Regression  ')
grid on

X = [ones(length(x),1) x];
b = X\y;

yCalc2 = X*b;
plot(x,yCalc2) %,'--')
%legend('Data','Slope & Intercept','Location','best');
hold on;
Rsq1 = 1 - sum((y - yCalc1).^2)/sum((y - mean(y)).^2);
Rsq2 = 1 - sum((y - yCalc2).^2)/sum((y - mean(y)).^2);
dim = [.2 .5 .3 .3];
title("Number of nodes")
str = ['R^{2}: 0.889' newline 'RMSE: 0.891' newline 'MAPE: 5.09'];
annotation('textbox',dim,'String',str,'FitBoxToText','on', 'FontSize',15);

mdl = fitlm(x,y)

disp('MAPE');
(sum( abs( ((yCalc2 - y) ./ y) ) ) * 100) / size(y,1) 
