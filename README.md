This file explains how to install and execute the sort/unsort decsion making software used in the manuscript Raman image-activated cell sorting" for the offline test.  

# System requirements

- Ubuntu Linux 16.04
- OpenCV 3.4 (https://opencv.org/opencv-3-4.html)

# Installation

Please follow the steps below to install. This process usually takes less than 5 minutes.

1. Make sure your computer environment meets the system requirements.
  For OpenCV, download the source code, build and install it for C/C++.


# Execution

Please follow the steps below to execute. This process usually takes less than 15 minutes.
 
1. Build the source codes in "src" subfolder with the make command.
 > $ make
     
2. Run the built executable. 
- 3T3-L1 sample
 > $ ./bin/srs_proc ./data/3t3l1 1
- Chlamydomonas sample
 > $ ./bin/srs_proc ./data/chlamydomonas 2
- Euglena sample
 > $ ./bin/srs_proc ./data/euglena 3

3. If the code is successfully executed, you will get a result similar to the one illustrated in the "data" folder.For each sample, the results of the sorting decision are recorded in a file named "result_s.csv". 

To run this code on your own data, store the image files in the srsimg folder in each experiment folder in the data folder. If you want to change gating conditions, etc., please edit the .cpp files.

