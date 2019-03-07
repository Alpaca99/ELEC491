clear
close all
D = zeros(800,1);
for k=1:13
    textFileName = [num2str(k) '.txt'];
    if exist(textFileName, 'file')
		fileID = fopen(textFileName);
		C = textscan(fileID,'%f');
        D = [D C{1}];
		fclose(fileID);
    else
    fprintf('File %s does not exist.\n', textFileName);
    end
end
D = D(:,2:14);
para=[];
for i=1:13
    X=D(:,i);
    T=1:1:800;
subplot(4,1,1);%orinigal signal
time=reshape(T,[800,1]);
plot(T,X*3.3/1023);
xlabel("Time(s)")
ylabel("Voltage(V)")
title("Microphone Reading:Ideal Environment")


subplot(3,1,2)%plot from the start of the signal
[yupper,ylower]=envelope(X*3.3/1023,10,'peak');
[val,index]=max(yupper);
px=X*3.3/1023;
px([1:index-1])=[];
%Tnew=1:1:800-index;
%t=reshape(Tnew,[Tnew-index,1]);
plot(px);
xlabel("Time(s)")
ylabel("Voltage(V)")
title("Microphone Reading:Ideal Environment")


subplot(3,1,3); %exponential fit of upper envelope
upper=yupper;
upper([1:index-1])=[];
y=1:1:800-index+1;
Y=reshape(y,[800-index+1,1]);
f2 = fit(Y,upper,'exp2');
plot(f2,Y,upper);
xlabel("Time(s)")
ylabel("Voltage(V)")
title("Exponential Decay of Upper Envelope");

parameters=coeffvalues(f2);
para=vertcat(para,parameters);

%outvalue=array2table(para,'VariableNames',{'a','b','c','d'});


end
filename='out_para.xls';
writetable(para,filename);

