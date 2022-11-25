%% Robotics I
%% Project 2021 - 2022
%  Part B

% ===================================================

% initialization

% robot links
l0 = 10;
l1 = 15;
l2 = 30;
l3 = 30;

% ===================================================

% period
T = 2;

% 1kHz sampling frequency 
dt = 0.001;

tf1 = 1;
tf2 = 2;

% time axis
t1 = 0:dt:tf1;
t2 = tf1:dt:tf2;

% ===================================================

theta = zeros(size(t1));

for t = 1:length(t1)
    if t1(t) < 1/4
        theta(t) = 64*pi/3*t1(t)^3 - 128*pi/3*t1(t)^4;
    elseif t1(t) >= 1/4 && t1(t) <= 3/4
        theta(t) = 4*pi/3*(t1(t) - 1/8);
    else
        theta(t) = 4*pi/3*(t1(t) - 1/8) - 64*pi/3*(t1(t) - 3/4)^3 + 128*pi/3*(t1(t) - 3/4)^4;
    end
end

% ===================================================

pz2 = zeros(size(t2));

for t = 1:length(t2)
    if t2(t) < 1.2
        pz2(t) = 50 - 312.5*(t2(t) - 1)^3 + 781.25*(t2(t) - 1)^4;
    elseif t2(t) >= 1.2 && t2(t) <= 1.8
        pz2(t) = 50 - 12.5*(t2(t) - 1.1);
    else
        pz2(t) = 50 - 12.5*(t2(t) - 1.1) + 312.5*(t2(t) - 1.8)^3 - 781.25*(t2(t) - 1.8)^4;
    end
end

% ===================================================

% calculation of px, py, pz
t = 0:dt:2;

px = 15*ones(size(t));
py = -30*ones(size(t));
pz = zeros(size(t));

for i = 1:length(t1)
    py(i) = -30 + 5*sin(theta(i));
end

for i = 1:length(t)
    if t(i) < 1
        pz(i) = 45 - 5*cos(theta(i));
    else
        pz(i) = pz2(i - 1000);
    end
end

% ===================================================

% calculation of theta derivative
theta_der = zeros(size(t1));

for t = 1:length(t1)
    if t1(t) < 1/4
        theta_der(t) = 64*pi*t1(t)^2 - 512*pi/3*t1(t)^3;
    elseif t1(t) >= 1/4 && t1(t) <= 3/4
        theta_der(t) = 4*pi/3;
    else
        theta_der(t) = 4*pi/3 - 64*pi*(t1(t) - 3/4)^2 + 512*pi/3*(t1(t) - 3/4)^3;
    end
end

% ===================================================

% calculation of pz derivative
pz2_der = zeros(size(t2));

for t = 1:length(t2)
    if t2(t) < 1.2
        pz2_der(t) = -937.5*(t2(t) - 1)^2 + 3125*(t2(t) - 1)^3;
    elseif t2(t) >= 1.2 && t2(t) <= 1.8
        pz2_der(t) = -12.5;
    else
        pz2_der(t) = -12.5 + 937.5*(t2(t) - 1.8)^2 - 3125*(t2(t) - 1.8)^3;
    end
end

% ===================================================

% calculation of linear velocities px_der, py_der, pz_der
t = 0:dt:2;

px_der = zeros(size(t));
py_der = zeros(size(t));
pz_der = zeros(size(t));

for i = 1:length(t1)
    py_der(i) = 5*cos(theta(i))*theta_der(i);
end

for i = 1:length(t)
    if t(i) < 1
        pz_der(i) = 5*sin(theta(i))*theta_der(i);
    else
        pz_der(i) = pz2_der(i - 1000);
    end
end

% ===================================================
% ===================================================

% PLOT: y - z
figure(1)
plot(pz, py)
grid on
hold on
plot(40, -30, '-', 'MarkerSize', 15, 'color', 'g')
plot(50, -30, '-', 'MarkerSize', 15, 'color', 'g')
xlabel('z - axis')
ylabel('y - axis')

% ===================================================
% ===================================================

% PLOT: position x
figure(2)
plot(t, px)
grid on
hold on
xlabel('Time (sec)')
ylabel('p_x (cm)')
title('p_x(t)')

% ===================================================

% PLOT: position y
figure(3)
plot(t, py)
grid on
hold on
xlabel('Time (sec)')
ylabel('p_y (cm)')
title('p_y(t)')

% ===================================================

% PLOT: position z
figure(4)
plot(t, pz)
grid on
hold on
xlabel('Time (sec)')
ylabel('p_z (cm)')
title('p_z(t)')

% ===================================================

% PLOT: velocity x
figure(5)
plot(t, px_der)
grid on
hold on
xlabel('Time (sec)')
ylabel('derivative of p_x (cm/sec)')
title('derivative of p_x(t)')

% ===================================================

% PLOT: velocity y
figure(6)
plot(t, py_der)
grid on
hold on
xlabel('Time (sec)')
ylabel('derivative of p_y (cm/sec)')
title('derivative of p_y(t)')

% ===================================================

% PLOT: velocity z
figure(7)
plot(t, pz_der)
grid on
hold on
xlabel('Time (sec)')
ylabel('derivative of p_z (cm/sec)')
title('derivative of p_z(t)')

% ===================================================
% ===================================================

% Inverse Kinematic Model
% calculation of joint angles qi

q1 = atan2(l1, -sqrt(px.^2 + py.^2 - l1^2)) - atan2(px, py);
s1 = sin(q1);
c1 = cos(q1);

q3 = atan2((px.*s1-py.*c1).^2 + (pz-l0).^2 - l2^2 - l3^2, sqrt((2*l2*l3)^2 - ((px.*s1-py.*c1).^2 + (pz-l0).^2 - l2^2 - l3^2).^2));
s3 = sin(q3);
c3 = cos(q3);

q2 = atan2(pz - l0, px.*s1-py.*c1) - atan2(l2 + l3*s3, l3*c3);
s2 = sin(q2);
c2 = cos(q2);

s23 = sin(q2 + q3);
c23 = cos(q2 + q3);

% ===================================================

% Inverse Differential Kinematic Model
% calculation of joint velocities of derivatives of qi
ari1 = l2*l3*c1.*c3.*px_der + l2*l3*s1.*c3.*py_der;
para1 = l2*l3*c3.*(l3*c23-l2*s2);
q1_der = ari1./para1;

ari2 = -l3*c23.*(l1*c1-l2*s1.*s2+l3*s1.*c23).*px_der + l3*c23.*(-l1*s1-l2*c1.*s2+l3*c1.*c23).*py_der - l3*s23.*(l3*c23-l2*s2).*pz_der;
para2 = l2*l3*c3.*(l3*c23-l2*s2);
q2_der = ari2./para2;

ari3 = (l3*c23-l2*s2).*(l1*c1-l2*s1.*s2+l3*s1.*c23).*px_der - (l3*c23-l2*s2).*(-l1*s1-l2*c1.*s2+l3*c1.*c23).*py_der + (l3*c23-l2*s2).*(l2*c2+l3*s23).*pz_der;
para3 = l2*l3*c3.*(l3*c23-l2*s2);
q3_der = ari3./para3;

% ===================================================

% PLOT: angle q1
figure(8)
plot(t, q1*180/pi)
grid on
hold on
xlabel('Time (sec)')
ylabel('q_1 (deg)')
title('q_1(t)')

% ===================================================

% PLOT: angle q2
figure(9)
plot(t, q2*180/pi)
grid on
hold on
xlabel('Time (sec)')
ylabel('q_2 (deg)')
title('q_2(t)')

% ===================================================

% PLOT: angle q3
figure(10)
plot(t, q3*180/pi)
grid on
hold on
xlabel('Time (sec)')
ylabel('q_3 (deg)')
title('q_3(t)')

% ===================================================

% PLOT: derivative q1
figure(11)
plot(t, q1_der*180/pi)
grid on
hold on
xlabel('Time (sec)')
ylabel('derivative of q_1 (deg/sec)')
title('derivative of q_1(t)')

% ===================================================

% PLOT: derivative q2
figure(12)
plot(t, q2_der*180/pi)
grid on
hold on
xlabel('Time (sec)')
ylabel('derivative of q_2 (deg/sec)')
title('derivative of q_2(t)')

% ===================================================

% PLOT: derivative q3
figure(13)
plot(t, q3_der*180/pi)
grid on
hold on
xlabel('Time (sec)')
ylabel('derivative of q_3 (deg/sec)')
title('derivative of q_3(t)')

% ===================================================
% ===================================================

% Kinematic Simulation

% 3D Simulation
figure(14)
plot3(0, 0, 0, 'k*', 'MarkerFaceColor', 'k')
hold on
grid on
plot3([0, l0], [0, 0], [0, 0], 'k')
plot3(l0, 0, 0, 'k*', 'MarkerFaceColor', 'k')
plot3(pz, px, py, 'g')
plot3([-5 55], [0, 0], [0, 0], 'k--')
plot3([0, 0], [-15 45], [0, 0], 'k--')
plot3([0, 0], [0, 0], [-45 15], 'k--')
for tk = 1:200:2001
    % pause motion
    pause(0.2);

    % joint 1
    plot3([l0, l0], [0, l1*c1(tk)], [0, l1*s1(tk)], 'k');
    % link from joint 1 to joint 2
    plot3(l0, l1*c1(tk), l1*s1(tk), 'k*', 'MarkerFaceColor', 'k');
    % joint 2
    plot3([l0, l0+l2*c2(tk)], [l1*c1(tk), l1*c1(tk)-l2*s1(tk)*s2(tk)], [l1*s1(tk), l1*s1(tk)+l2*c1(tk)*s2(tk)], 'k');
    % link from joint 2 to joint 3
    plot3(l0+l2*c2(tk), l1*c1(tk)-l2*s1(tk)*s2(tk), l1*s1(tk)+l2*c1(tk)*s2(tk), 'k*', 'MarkerFaceColor', 'k');
    % joint 3
    plot3([l0+l2*c2(tk), l0+l2*c2(tk)+l3*s23(tk)], [l1*c1(tk)-l2*s1(tk)*s2(tk), l1*c1(tk)-l2*s1(tk)*s2(tk)+l3*s1(tk)*c23(tk)], [l1*s1(tk)+l2*c1(tk)*s2(tk), l1*s1(tk)+l2*c1(tk)*s2(tk)-l3*c1(tk)*c23(tk)], 'k');
    % link from joint 3 to end-effector
    plot3(l0+l2*c2(tk)+l3*s23(tk), l1*c1(tk)-l2*s1(tk)*s2(tk)+l3*s1(tk)*c23(tk), l1*s1(tk)+l2*c1(tk)*s2(tk)-l3*c1(tk)*c23(tk), 'm+');
    % end-effector
    plot3(pz(tk), px(tk), py(tk), 'rx');
end

title('3D Kinematic Simulation');
xlabel('z (cm)');
ylabel('x (cm)');
zlabel('y (cm)');

% ===================================================
