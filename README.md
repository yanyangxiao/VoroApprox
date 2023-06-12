# VoroApprox
This program is the implementation of paper "Zhonggui Chen, Yanyang Xiao, Juan Cao. Approximation by Piecewise Polynomials on Voronoi Tessellation. Graphical Models (Proc. GMP 2014), 76(5), 522-531, 2014"

## dependencies
CGAL: https://www.cgal.org/

## compile
Using CMake

## useage
(1) load image -> init -> optimize to run the algorithm;

(2) once Voronoi diagram is generated, click 'approximate' to compute an approximating image;

(3) scroll mouse middle button to zoom in and out; press mouse right button with motion to translate;

(4) left Alt + scroll mouse middle button to change point size; right Alt + scroll mouse middle button to change line width;

## examples
![](examples/elephant-init.png)
![](examples/elephant-result.png)
![](examples/elephant-output.png)
![](examples/lena-output.png)
![](examples/girl-output.png)

## cite

If you find our code or paper useful, please consider citing

```
@article{chen2014approximation,
   title={Approximation by piecewise polynomials on Voronoi tessellation},
   author={Chen, Zhonggui and Xiao, Yanyang and Cao, Juan},
   journal={Graphical Models},
   volume={76},
   number={5},
   pages={522-531},
   year={2014},
   publisher={Elsevier}
}
```
