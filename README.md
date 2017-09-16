

# HopeFOAM:  High Order Parallel Extensible CFD software

       ---------------------------------------------------------------------------------------
       |     __  ______  ____  ______ |                                                       |
       |    / / / / __ \/ __ \/ ____/ | HopeFOAM: High Order Parallel Extensible CFD Software |
       |   / /_/ / / / / /_/ / __/    |                                                       |
       |  / __  / /_/ / ____/ /___    |                                                       |
       | /_/ /_/\____/_/   /_____/    | Copyright(c) 2017-2017 The Exercise Group, AMS, China.|
       |                              |                                                       |
       ---------------------------------------------------------------------------------------
        
        TITLE:     README for [[ https://github.com/HopeFOAM/HopeFOAM [HopeFOAM-0.1]]
        AUTHOR:               The Exercise Group
        DATE:                 15th September 2017
        LINK:         https://github.com/HopeFOAM/HopeFOAM

  **Copyright (c) 2017-2017 The Exercise Group, AMS, China.**

* **About HopeFOAM**

HopeFOAM is a major extension of OpenFOAM to variable higher order finite element method and other numerical methods for computational mechanics. It is developed by Exercise Group, Innovation Institute for Defence Science and Technology, the Academy of Military Science (AMS), China. The Group aims at developing open source software packages for large scale computational science and engineering.

HopeFOAM has following features:
  
    * **High Order**
    
      In addition to the finite volume discretisation method used in OpenFOAM, HopeFOAM aims at incorporating high-order discretisation methods into the computational mechanics Toolbox, among which DGM(Discontinuous Galerkin Method) is the first one.
       
    * **Parallel** 
    
      In order to improve the performance and scalability of parallel computing, parallel computational toolkits/software are integrated into HopeFOAM to accelerate the discretization and computational procedures.
  
    * **Extensible**
    
      By incorporating with the high order discretisation and efficient parallel computing, HopeFOAM provides an extensible software framework for further development of application module and easy-to-use interfaces for developers
  
    * **FOAM**
    
      The current version of HopeFOAM is a major extension of the OpenFOAM-4.0 released by the OpenFOAM Foundation on the 28th of June, 2016.

* **Release Note**

HopeFOAM-0.1 is the first publicly released version of HopeFOAM and developed by a major extension of OpenFOAM-4.0. The well-known high-order discretization method, Discontinuous Galerkin Method (DGM) is implemented in HopeFOAM-0.x. There are copious references about the method in the book:[Hesthaven J S, Warburton T. Nodal discontinuous Galerkin methods: algorithms, analysis, and applications[M]. Springer Science & Business Media, 2007.]
HopeFOAM-0.1 provides 2D-DGM and related support. The major components include data structure, DGM discretization, solvers and related tools. PETSc is used for solving of linear systems of equations. Details information can be found in HopeFOAM-0.1_Programmers_Guide and HopeFOAM-0.1_User_Guide. 3D applications will be supported in a new release in a few months. 
The guiding principle in the development of HopeFOAM-0.1 is to reuse the primitive data structure of OpenFOAM-4.0 as much as possible and keep it consistent with the user interfaces. Thus, users of OpenFOAM could implement and adopt corresponding high order DGM solvers in a relatively straightforward way. 

  
* **Copyright**

  HopeFOAM is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your
  option) any later version.
  
  HopeFOAM is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.
  
  You should have received a copy of the GNU General Public License
 ?along with HopeFOAM.  If not, see <http://www.gnu.org/licenses/>.

* **Contact us**

  If you have any questions, please contact us by email <hopefoam@163.com>

* **Installation** 

  Please refer to the file Linux_Install_Guide located at root dir of HopeFOAM 
  
* **Document**

  Please see the document files under the source code Dir SOUCEDE_DIR/doc

* **Download**

  [https://github.com/HopeFOAM/HopeFOAM [Source Code]]  
  
  [https://codeload.github.com/HopeFOAM/HopeFOAM/zip/master  [Source Code zip download]]
  
  
  [https://github.com/HopeFOAM/HopeFOAM/tree/master/HopeFOAM-0.1/doc/Guides/HopeFOAM-0.1_Programmers_Guide.pdf [Programmers Guide]]  
  
  [https://github.com/HopeFOAM/HopeFOAM/tree/master/HopeFOAM-0.1/doc/Guides/HopeFOAM-0.1_User_Guide.pdf  [User Guide]]
  
