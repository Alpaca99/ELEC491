fileID = fopen("75cm90deg_inside.txt");
C = textscan(fileID,'%f %f %f');
fclose(fileID);
X=C{1}
Y=C{2}
Z=C{3}
n=10000;
t_end=3;
t=0:t_end/n:t_end*(1-1/n);
%data_float=data* 0.004882814;
%plot(t,data);
plot(X,'r');
hold on
plot(Y,'b');
plot(Z,'g');
hold on


fileID = fopen("75cm90deg_outside.txt");
D = textscan(fileID,'%f %f %f');
fclose(fileID);
a=D{1}
b=D{2}
c=D{3}
n=10000;
p_end=3;
p=0:p_end/n:p_end*(1-1/n);
%data_float=data* 0.004882814;
%plot(t,data);
plot(a);
plot(b);
plot(c);
