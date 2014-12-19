
A = [-0.57308,0.34419,0.74371,-1.13597; -0.20127,0.82061,-0.53487,-0.80367; -0.79440,-0.45622,-0.40100,0.93646; 0.00000,0.00000,0.00000,1.00000];

B = A; B(:,4) = B(:,4) * (47.68 / 2); B(4,4) = 1;

o = B * [0 0 0 1]'; o(4) = [];
o1 = B * [10 0 0 1]'; o1(4) = [];
o2 = B * [0 10 0 1]'; o2(4) = [];
o3 = B * [0 0 10 1]'; o3(4) = [];

LEDS = B^(-1) * [-10.850017,-13.898538,21.034423, 1; -8.963507,-4.099193,19.310233, 1; 10.938566,14.813514,23.273492, 1; 22.790962,4.919429,44.667622, 1; 21.678156,-3.967343,49.489478, 1; 2.332634,-19.547182,40.949912, 1]'; LEDS(4,:) = [];


c_c = [-12.40, 4.85, 45.74, 1];
p_c = [-10.25, 3.51, 44.73, 1];

ps_c = B^(-1) * p_c'; ps_c(4) = [];
cs_c = B^(-1) * c_c'; cs_c(4) = [];

figure;
hold on;
plot3([o(1); o1(1)], [o(2); o1(2)], [o(3); o1(3)]);
plot3([o(1); o2(1)], [o(2); o2(2)], [o(3); o2(3)]);
plot3([o(1); o3(1)], [o(2); o3(2)], [o(3); o3(3)]);

plot3(LEDS(1,:), LEDS(2,:), LEDS(3,:), 'x', 'color', 'blue');

plot3(ps_c(1), ps_c(2), ps_c(3), 'o', 'color', 'red');
plot3(cs_c(1), cs_c(2), cs_c(3), 'o', 'color', 'blue');
plot3([cs_c(1)' ps_c(1)'], [cs_c(2)' ps_c(2)'], [cs_c(3)' ps_c(3)'], '-');

