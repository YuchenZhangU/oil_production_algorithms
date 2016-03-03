Pi = 6000;      %inital reservoir pressure: 6000 psi
P_low = 1000;   %the low bound for BHP
dropSlope = 80; %maximum BHP drop rate: 80 psi/day
dropInt = 5;    %drop the pressure for 5 days
holdInt = 10;    %hold the pressure for 5 days
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

buildInt =150;

linspace(bhp(end),4000,length(tstep)-length(bhp));
bhp2 = [bhp linspace(bhp(end),4000,buildInt)];
bhp2 = [bhp2 4000*ones(1,length(tstep)-length(bhp2))]

fileT = fopen('tstep.dat','w');
fileSch = fopen('sch.dat','w');
fprintf(fileT,'%d ',tstep);
fprintf(fileT,'\n');
fprintf(fileT,'%d ',tstep);

fprintf(fileSch,'%d ',bhp1);
fprintf(fileSch,'\n');
fprintf(fileSch,'%d ',bhp2);

fclose(fileT);
fclose(fileSch);

 plot(cumsum(tstep),bhp2)