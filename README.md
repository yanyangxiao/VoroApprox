# VoroApprox
Implementation of paper "Zhonggui Chen, Yanyang Xiao, Juan Cao. Approximation by Piecewise Polynomials on Voronoi Tessellation. Graphical Models (Proc. GMP 2014), 76(5), 522-531, 2014"

if you use the code, please cite

@article{chen2014approximation, <br>
   title={Approximation by piecewise polynomials on Voronoi tessellation}, <br>
   author={Chen, Zhonggui and Xiao, Yanyang and Cao, Juan}, <br>
   journal={Graphical models}, <br>
   volume={76}, <br>
   number={5}, <br>
   pages={522-531}, <br>
   year={2014}, <br>
   publisher={Elsevier} <br>
}

# Examples
![](examples/elephant-init.png)
![](examples/elephant-result.png)
![](examples/elephant-output.png)
![](examples/lena-output.png)
![](examples/girl-output.png)

# Requirements
CGAL

# Compile
Just build the project by using CMake

# Useage
(1) load image -> init -> optimize to run the algorithm;

(2) after running algorithm, click 'approximate' button to compute an approximating image;

(3) scroll mouse middle button to zoom in and out; press mouse right button with motion to translate;

(4) left Alt + scroll mouse middle button to change point size; right Alt + scroll mouse middle button to change line width;
