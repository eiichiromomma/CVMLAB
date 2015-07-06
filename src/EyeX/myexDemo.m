%% 1. compile
% - Only needs to be run on first usage.
% - Must be run in a directory containing:
%       ./eyex (subdirectory containing EyeX.h, EyeXActions.h, etc.)
%       myex.c
%       Tobii.EyeX.Client.dll
%       Tobii.EyeX.Client.lib
% - Note:
%       This *did* work with: Microsoft Software Development Kit (SDK) 7.1 in C:\Program Files (x86)\Microsoft Visual Studio 10.0
%       This did *not* work with: Lcc-win32 C 2.4.1 in C:\PROGRA~2\MATLAB\R2012b\sys\lcc
%       (i.e., since lcc does not permit variable definition/initialisation on same line)
%       - You can change compiler using mex -setup
%       - You can download the visual studio compiler as part of the
%         Microsoft .Net dev kit (if my memory serves)
mex myex.c % compile to generate myex.mexw32, myex.mexw64, or whatever
pause(.1);
 
%% 2. run
% connect to EyeX Engine
myex('connect')
% allow to track for N seconds
pause(5);
% poll myex.c for any data in buffer (N.B. will return empty if eyes were
% not tracked)
x = myex('getdata');
% disconnect from EyeX Engine
pause(.1);
myex('disconnect')
 
%% 3. show results
close all
plot(x(:,1:2))

% mod. by Eiichiro Momma 2015/07/06 
figure,plot(x(:,1),x(:,2))
set(gca,'YDir','rev')

% print data to console
fprintf('%6.2f  %6.2f  %7.2f     %i %i  %6.2f %6.2f %6.2f  %6.2f %6.2f %6.2f  %7.2f\n',x')
 
% Example console output:
%     ...
%     833.48  476.04  2795213.47     1 1  -35.46 -69.89 606.63   25.79 -61.06 618.22  2795206.13
%     833.38  481.44  2795230.65     1 1  -35.86 -69.89 606.43   25.32 -60.86 618.23  2795223.11
%     827.62  486.93  2795265.50     1 1  -36.31 -69.77 605.84   24.87 -60.45 618.16  2795258.28
%     824.11  490.15  2795281.87     1 1  -36.46 -69.66 605.70   24.67 -60.23 618.15  2795274.20
%     823.07  494.03  2795296.70     1 1  -36.50 -69.40 606.18   24.67 -59.98 618.15  2795289.09
%     821.80  494.31  2795315.55     1 1  -36.48 -69.01 607.07   23.92 -62.15 605.15  2795308.19
%     ...