%clear all
%clc
%a=arduino();

interv=1000;
init_time=1;
x=0;
y=0;
z=0;
while (init_time<interv)
    vx=readVoltage(a,'A3')*(1023/3.3);
    vy=readVoltage(a,'A2')*(1023/3.3);
    vz=readVoltage(a,'A1')*(1023/3.3);
    x=[x,vx];
    y=[y,vy];
    z=[z,vz];
    plot(x,'r');
    hold on
    plot(y,'b');
    plot(z,'g');
    legend('x','y','z');
    ylim([500 800]);
    hold off
    grid ON
    init_time=init_time+1;
    drawnow
end
