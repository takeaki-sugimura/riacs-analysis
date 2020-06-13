This file explains how to install and execute the analysis software used in the manuscript "Raman image-activated cell sorting."


# System requirements

- Open source software R is required to run the analysis program. If you do not have one, download the base distribution of the softeware from the following website and install it on your computer. 
https://cran.r-project.org/

- The analysis software was tested on windows10 with R (ver4.0.0) installed, which may not work properly if the OS or version is different.


# Installation

Please follow the steps below to install. This process usually takes less than 5 minutes.

1. Save the unzipped folder to your computer. For a description of the contents of the folder, please refer to "Contents" below. Do not change the configuration or file names in the folder.

2. If you do not have R installed on your computer, please visit the CRAN website (https://cran.r-project.org/) to install it

3. Launch the R GUI software,and specify the absolute path of the unzipped folder as the working directory. Note if you close R and reopen it, you may need to do this again. You can specify the working directory by using the setwd() function in the R GUI console.An example is shown below.

> setwd("C:/RIACS/software_analysis_2")


# Contents
 
The unzipped folder contains the following files and folders.

- "Fig2c.R"
- "Fig2d.R"
- "Fig4b.R"
- "Fig4d.R"
- "Fig4f.R"

These files are the main R code used for graph drawing.

- "experiment_info_beads-analysis.csv"
- "experiment_info_beads-sort.csv"
- "experiment_info_3T3.csv"
- "experiment_info_chramydomonas.csv"
- "experiment_info_euglena.csv"

These files hold the condition settings for each experiment, the path to the data, etc. These files are used by the R code to access the data.

-"S5-functions_04s.R"

This file contains the functions referenced by the main R code.

- [Folder:"images"]

This folder contains the measurement data for each experiment.

- [Folder:"output"]

This folder shows examples of screenshots after the analysis software is executed.


# Execution

Please follow the steps below to execute. This process usually takes less than 5 minutes.

1. Find one a main R codes named "Fig**.R". Open it on the R GUI software.

2. Execute the R script on the R GUI console. You can execute it by typing [Ctrl]+[A] followed by [Ctrl]+[R]. 
If you get the error message that file "S5-functions_04s.R" is not found, please redo step 3 of the installation. 

3. If the code is successfully executed, you will get a result similar to the one illustrated in the "output" folder.

If you want to run this software with other data, it is necessary to arrange the csv file with the experimental data output from the IA node's software and the csv file with the experimental information appropriately.
