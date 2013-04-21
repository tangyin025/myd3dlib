# find -name *.vcproj | xargs sed 's/Version="9.00"/Version="8.00"/' -i -b
find -name *.vcproj | xargs sed 's/E:\\Games\\boost_1_34_1/E:\\Downloads\\boost_1_43_0/' -i -b
# find -name *.sln | xargs sed 's/Microsoft Visual Studio Solution File, Format Version 10.00/Microsoft Visual Studio Solution File, Format Version 9.00/' -i -b
# find -name *.sln | xargs sed 's/# Visual Studio 2008/# Visual Studio 2005/' -i -b