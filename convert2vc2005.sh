# find -name *.vcproj | xargs sed 's/Version="9.00"/Version="8.00"/' -i -b
# find -name *.sln | xargs sed 's/Microsoft Visual Studio Solution File, Format Version 10.00/Microsoft Visual Studio Solution File, Format Version 9.00/' -i -b
# find -name *.sln | xargs sed 's/# Visual Studio 2008/# Visual Studio 2005/' -i -b
find -name *.vcproj | xargs sed 's/D:\\Works\\OpenSource\\boost_1_34_1/E:\\新建文件夹\\boost_1_44_0/g' -i -b
find -name *.vcproj | xargs sed 's/D:\\Works\\OpenSource\\PhysX-3.2.3_PC_SDK_Core/E:\\新建文件夹\\PhysX-3.2.3_PC_SDK_Core/g' -i -b
find -name *.vcproj | xargs sed 's/D:\\Works\\OpenSource\\APEXSDK-1.2.3-Build4-CL15214552-PhysX_3.2.3-WIN-VC10-BIN/E:\\新建文件夹\\APEXSDK-1.2.3-Build4-CL15214552-PhysX_3.2.3-WIN-VC10-BIN/g' -i -b