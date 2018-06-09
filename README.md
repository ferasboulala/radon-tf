# radon-tf
Simple opencv radon transform implementation (no Fourier slice theorem or filtering is applied). Example of usage is found in `main.cpp`. The `sinogram()` function can be speeded up by giving it a thread count. Tests have shown that one can get up to the number of cores of this computer in speed up factor.

![example](pic/phantom_sinogram.png)

![inverse](pic/inverse.png)
