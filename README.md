# TSBK03_Project
Source code of my project for the course TSBK03 Teknik för avancerade datorspel taught at Linköping Univeristy. Check within the "OpenGLApplication" folder for source-code (I know, amazing name).

## Notes before starting
There is descriptive comments at the start of each code-file the sums up the purpose of each class. Read those to get an introduction to the structure of the project. 

There are also alot of "debugging-prints" that has been commented out left in the code. Uncomment these for deeper insight of certian structures.

There is 3 visualization-modes using 3 shaders. You can also enable/disable "fake physics" which was a placeholder that was left in. Ctrl + f "mode" to find these. 

The most relevant files are:
*  `Main.cpp`: Contains the application entry point and overall program flow.
*  `skeleton.h`: Defines the core data structures used to represent a skeletal hierarchy.
*  `physicsBone.h`: Is a lightweight physics-oriented representation of bones.

## Current issues
* All things `Bullet`-related (used for physics-sim) doesn't work. Can't get bullet to generate dll-files, and therefore, it cant be used in the project either. 
    * *Update*: Bullet has been deleted from project to reduce size.
* No functional ragdoll-sim due to the previous issue and time-constraint.

## Possible issues
* I've pushed some *.vs* files to github before creating a **gitignore**
