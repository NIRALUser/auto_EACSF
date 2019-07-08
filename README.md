# What is Auto_EACSF?
Automatic Extra-Axial Cerebrospinal Fluid (Auto EACSF) is an open-source, interactive tool for automatic computation of brain extra-axial cerebrospinal fluid (EA-CSF) in magnetic resonance image (MRI) scans of infants. Elevated extra-axial fluid volume is a possible biomarker for Autism Spectrum Disorder (ASD). Auto EACSF aims to automatically calculate the volume of EA-CSF and could therefore be used for early diagnosis of Autism. Auto EACSF is a user-friendly application that generates a Qt application to calculate the volume of EA-CSF. The application is run through a GUI, but also provides an advanced use mode that allows execution of different steps by themselves via Python and XML scripts. 

# Build instructions
Requirements: Qt5, Python3 (with packages: itk, numpy)

Build:
1. Get files from Github
```
$ git clone https://github.com/NIRALUser/auto_EACSF
```
2. Make an out-of-source build directory
```
$ mkdir ./auto_EACSF-bin
$ cd auto_EACSF-bin
```
3. Configure the build
```
$ ccmake ../auto_EACSF
$ make
```
4. Get the executable
```
$ ./bin/auto_EACSF
```
5. Open the application by typing the line below (do not click to open). 
```
$ ./Auto_EACSF
```

Installation Notes:
Auto_EACSF generates and then calls a python3 script. For that purpose, a python 3 installation, as well as the python packages itk and numpy, is needed.

# How to fix an error?
You can fix an error by utilizing Qtcreator
1. Open Qtcreator
2. Open auto_EACSF source file within Qtcreator
3. Build the application
4. Run on Qtcreator 
5. Check console for detailed error message


