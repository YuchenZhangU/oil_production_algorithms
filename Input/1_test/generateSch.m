Pi = 6000;      %inital reservoir pressure: 6000 psi
P_low = 1000;   %the low bound for BHP
dropSlope = 80; %maximum BHP drop rate: 80 psi/day
dropInt = 5;    %drop the pressure for 5 days
holdInt = 20;    %hold the pressure for 5 days
tstep = ones(1,360);

pInt = Pi:-dropSlope*dropInt:P_low;
pInt = [pInt P_low];

dt = 1;
bhp = [];
for i=1:(length(pInt)-1)
   bhp = [bhp (pInt(i)-dt*dropSlope):-dt*dropSlope:pInt(i+1)];
   bhp = [bhp pInt(i+1)*ones(1,holdInt/dt)];
end
bhp1 = [bhp P_low*ones(1,length(tstep)-length(bhp))];

fileT = fopen('tstep.dat','w');
fileSch = fopen('sch.dat','w');
fprintf(fileT,'%d ',tstep);
fprintf(fileSch,'%d ',bhp1);
% --------------- print case(2) - case(N) --------------

buildInt =10;
buildGoal = 6000;   %shut the well to keep BHP 6000 psi
buildAt = [4000 3000 2000 1000]
for i = 1:length(buildAt)
    P_low = buildAt(i);
    pInt = Pi:-dropSlope*dropInt:P_low;
    if(pInt(end) ~= P_low)
        pInt = [pInt P_low];
    end
    
    dt = 1;
    bhp = [];
    for i=1:(length(pInt)-1)
        bhp = [bhp (pInt(i)-dt*dropSlope):-dt*dropSlope:pInt(i+1)];
        bhp = [bhp pInt(i+1)*ones(1,holdInt/dt)];
    end
    bhp2 = [bhp linspace(bhp(end),buildGoal,buildInt)];
    bhp2 = [bhp2 buildGoal*ones(1,length(tstep)-length(bhp2))]
    
    fprintf(fileT,'\n');
    fprintf(fileT,'%d ',tstep);

    fprintf(fileSch,'\n');
    fprintf(fileSch,'%d ',bhp2);
    
    
    plot(cumsum(tstep),bhp2,'LineWidth',2)
end

fclose(fileT);
fclose(fileSch);

% bhp2 = [bhp linspace(bhp(end),buildGoal,buildInt)];
% bhp2 = [bhp2 buildGoal*ones(1,length(tstep)-length(bhp2))]
% 
% 
% 
% fprintf(fileT,'\n');
% fprintf(fileT,'%d ',tstep);
% 
% 
% fprintf(fileSch,'\n');
% fprintf(fileSch,'%d ',bhp2);
% 
% plot(cumsum(tstep),bhp1,'LineWidth',2)
% titleStr1 = ['pressure drop time=' num2str(dropInt) ' days          pressure hold time=' num2str(holdInt) ' days']
% title(titleStr1);
% 
% plot(cumsum(tstep),bhp2,'LineWidth',2)