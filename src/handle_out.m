close all;
clear all;

% parameters
freq = 100;
h = 1/freq;

% load
load traj_p.txt;
load traj_v.txt;
load traj_a.txt;
wayps = [0.0 0.0 1.0;
100.0 1.0 5.0;
30.0 60.0 3.0;
-10.0 -5.0 10.0;
0.0 0.0 1.0];

% variables
pos = traj_p;
vel = traj_v;
acc = traj_a;

steps = size(pos,1);
end_time = (steps-1)/freq;
time = 0:h:end_time;
%%
newcolors = [
    135,206,235;      %light blue
    219,112,147;        % red
    154,205,50;        % light green
    0,0,205;      % dark blue
    139,0,0;        % dark red
    85,107,47;      % dark green
    ];
newcolors = newcolors/255;
colororder(newcolors);


%% plot states

figure(1);
y = 3;
x = 1;

subplot(y,x,1);
plot(time,pos);
hold on;
grid on;
legend ('x','y','z');

subplot(y,x,2);
plot(time,vel);
hold on;
grid on;
legend ('v_x','v_y','v_z');

subplot(y,x,3);
plot(time,acc);
hold on;
grid on;
legend ('a_x','a_y','a_z');



%% plot in 3d

figure(2);

x = pos(:,1);
y = pos(:,2);
z = pos(:,3);

px = wayps(:,1);
py = wayps(:,2);
pz = wayps(:,3);

plot3(x,y,z);
hold on;
grid on;
plot3(px,py,pz,'*r');
xlabel('x');
ylabel('y');
zlabel('z');


