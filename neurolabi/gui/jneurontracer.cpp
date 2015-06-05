#include "jneurontracer.h"
#include "zstackskeletonizer.h"
#include "zswcpositionadjuster.h"
#include "biocytin/swcprocessor.h"
#include "swc/zswcresampler.h"
#include "neutubeconfig.h"

const double HUGE_NUMBER=1e30;
const double HUGE_NUMBER2=1e30;

JNeuronTracer::JNeuronTracer()
{
  m_splitNumber = 1;
//  m_background = NeuTube::IMAGE_BACKGROUND_BRIGHT;
}

static int getIndex3DZfirst(int i, int j, int k, int nx, int ny, int /*nz*/)
{
    return k * nx * ny + i * ny + j;
}

//1d index of 2d index (i,j)
static int getIndex(int i, int j, int /*nx*/, int ny)
{
    return i * ny + j;
}

//compar function for sorting, for inverse sorting
static int comp (const void * elem1, const void * elem2)
{
    float f = *((float*)elem1);
    float s = *((float*)elem2);
    if (f > s) return  -1;
    if (f < s) return 1;
    return 0;
}

void JNeuronTracer::createMask2D(int nx, int ny, float *I, float *bw)
{
    int i,j,nxy,ii,jj,kk;
    float mu, sigma,dt,expCompr;
    float *Is, *Ix, *Iy, *Ixy, *v, *u, dx=1.0, dy=1.0;
    unsigned int Wx, Wy;
    float thrSparse,lambdaRatioThr;
    int nthrs =  100;
    float thrs[100];

    printf("Creating mask....\n");

    //PARAMETERS
    mu = 10.0;				//width in gradient flow.
    sigma = 1.0;			//Gaussian smoothing withs in pixel unit,
    dt = 0.02/mu;
    Wx = 10; Wy = 10;		//window size for gaussian filter.
    thrSparse = 0.01;		//sparseness of determining the threshold.
    expCompr = 0.2;			//compression exponent.
    lambdaRatioThr = 2.0;	//threshold for the ratio lambda1/lambda2 in Hessian to supress blob.

    nxy = nx * ny;
    Is = (float *)malloc(nxy * sizeof(float));
    Ix = (float *)malloc(nxy * sizeof(float));
    Iy = (float *)malloc(nxy * sizeof(float));
    Ixy = (float *)malloc(nxy * sizeof(float));
    u = (float *)malloc(nxy * sizeof(float));
    v = (float *)malloc(nxy * sizeof(float));

    //gaussian_filter(nx,ny,I,sigma,Is);
    gaussianFilter2D(nx, ny, Wx, Wy, I, Is, sigma);

    // compute Ix and Iy.
    for (i=1; i<nx-1; i++) {
        for (j=1; j<ny-1; j++) {
            ii = getIndex(i,j,nx,ny);
            jj = getIndex(i+1,j,nx,ny);
            kk = getIndex(i-1,j,nx,ny);
            Ix[ii] = (Is[jj] - Is[kk])/(2.0*dx);
            jj = getIndex(i,j+1,nx,ny);
            kk = getIndex(i,j-1,nx,ny);
            Iy[ii] = (Is[jj] - Is[kk])/(2.0*dy);
        }
    }
    for (j=0; j<ny; j++){
        ii = getIndex(0,j,nx,ny);
        jj = getIndex(1,j,nx,ny);
        kk = getIndex(0,j,nx,ny);
        Ix[ii] = (Is[jj] - Is[kk])/(1.0*dx);
        ii = getIndex(nx-1,j,nx,ny);
        jj = getIndex(nx-1,j,nx,ny);
        kk = getIndex(nx-2,j,nx,ny);
        Ix[ii] = (Is[jj] - Is[kk])/(1.0*dx);
    }
    for (i=0; i<nx; i++) {
        ii = getIndex(i,0,nx,ny);
        jj = getIndex(i,1,nx,ny);
        kk = getIndex(i,0,nx,ny);
        Iy[ii] = (Is[jj] - Is[kk])/(1.0*dy);
        ii = getIndex(i,ny-1,nx,ny);
        jj = getIndex(i,ny-2,nx,ny);
        kk = getIndex(i,0,nx,ny);
        Iy[ii] = (Is[jj] - Is[kk])/(1.0*dy);
    }
    //gvf2d(nx, ny, v, u, Ix, Iy, mu, dt, dx, dy, maxIter);
    gaussianFilter2D(nx, ny, Wx, Wy, Ix, v, sigma);
    gaussianFilter2D(nx, ny, Wx, Wy, Iy, u, sigma);

    // calculate the Hessian matrix
    for (i=1; i<nx-1; i++) {
        for (j=1; j<ny-1; j++){
            ii = getIndex(i,j,nx,ny);
            jj = getIndex(i+1,j,nx,ny);
            kk = getIndex(i-1,j,nx,ny);
            Ix[ii] = (v[jj] - v[kk])/(2.0*dx);
            Ixy[ii] =(u[jj] - u[kk])/(2.0*dx)/2.0;
            jj = getIndex(i,j+1,nx,ny);
            kk = getIndex(i,j-1,nx,ny);
            Iy[ii] = (u[jj] - u[kk])/(2.0*dy);
            Ixy[ii] += (v[jj] - v[kk])/(2.0*dy)/2.0;
        }
    }
    gaussianFilter2D(nx, ny, Wx, Wy, Ix, Ix, sigma);
    gaussianFilter2D(nx, ny, Wx, Wy, Iy, Iy, sigma);
    gaussianFilter2D(nx, ny, Wx, Wy, Ixy,Ixy, sigma);

    for (j=0; j<ny; j++){
        ii = getIndex(0,j,nx,ny);
        Ix[ii] = 0.0;
        Ixy[ii] = 0.0;
        ii = getIndex(nx-1,j,nx,ny);
        Ix[ii] = 0.0;
        Ixy[ii] = 0.0;
    }
    for (i=0; i<nx; i++) {
        ii = getIndex(i,0,nx,ny);
        Iy[ii] = 0.0;
        Ixy[ii] = 0.0;
        ii = getIndex(i,ny-1,nx,ny);
        Iy[ii] = 0.0;
        Ixy[ii] = 0.0;
    }

    // compute the eigen values of the Hessian matrix.
    for (i=0; i<nxy; i++) {
        v[i] = 0.5 * (Ix[i] + Iy[i] + sqrt((Ix[i] - Iy[i])*(Ix[i] - Iy[i]) + 4 * Ixy[i] * Ixy[i])); //lambda1
        u[i] = 0.5 * (Ix[i] + Iy[i] - sqrt((Ix[i] - Iy[i])*(Ix[i] - Iy[i]) + 4 * Ixy[i] * Ixy[i]));			//lambda2
    }

    for (i=0;  i<nxy; i++) {
        if (v[i] > lambdaRatioThr*abs(u[i])) {
            //bw[i] = pow(v[i],expComp) * exp(-5.0*abs(u[i])/(v[i]+1e-10));
            bw[i] = pow(v[i] - abs(u[i]),3)/(v[i] + 1e-10);
            bw[i] = pow(bw[i],expCompr);
        } else {
            bw[i] = 0.0;
        }
    }

    //get the threshold using the sparse critiria.
    for (i=0; i<nxy; i++) Is[i] = bw[i];
    qsort (Is, nxy, sizeof(float), comp);
    for (i=0;i<nthrs;i++) {
        thrs[i] = Is[0] - Is[0]/nthrs*(i+1);
    }
    j = 0;
    for (i=0; i<nxy; i++) {
        if (Is[i] < thrs[j]) {
            if (i*1.0/nxy > thrSparse) {
                break;
            }
            j++;
        }
    }

    Stack *stackData = Scale_Float_Stack(bw, nx, ny, 1, GREY);
    C_Stack::write(GET_TEST_DATA_DIR + "/test.tif", stackData);
    C_Stack::kill(stackData);

    for (i=0;i<nxy;i++) {
        if (bw[i] > thrs[j]) {
            bw[i] = 1.0;
        } else {
            bw[i] = 0.0;
        }
    }

    //createGrayTiffStackFromFloatArray(nx,ny,1,bw,"temp.tif");

    free(Is);
    free(Ix);
    free(Iy);
    free(u);
    free(v);
    free(Ixy);
}

Stack* JNeuronTracer::makeMask(const Stack *stack)
{
  int nx,ny,nz,i,j,k,ntot,nxy,ii;
  float *imFlat,maxI,minI;
  int nSplit,islab,z1,z2;
  float *bw,sigma;
  unsigned int Wx, Wy;

  nSplit = m_splitNumber;

  Stack *floatStack = C_Stack::translate(const_cast<Stack*>(stack), FLOAT32, 0);

  float* im3d = C_Stack::guardedArrayFloat32(floatStack);

  nx = C_Stack::width(stack);
  ny = C_Stack::height(stack);
  nz = C_Stack::depth(stack);
  //  im3d = (float *) malloc(nx*ny*nz*sizeof(float));
  //      readTiffStack(filename, tiffinfo, im3d, NULL, NULL);
  ntot = nx*ny*nz;
  nxy = nx * ny;

  //smooth each plane in the tiff stack
  sigma = 1.0;
  Wx = 5; Wy = 5;
  for (k=0; k<nz; k++) {
    i = k*nxy;
    gaussianFilter2D(nx, ny, Wx, Wy, im3d+i, im3d+i, sigma);
  }

  //reschale.
  maxI = -HUGE_NUMBER2;
  minI = HUGE_NUMBER2;
  for (i=0; i<ntot; i++) {
    if (maxI < im3d[i]) maxI = im3d[i];
    if (minI > im3d[i]) minI = im3d[i];
  }
  for (i=0; i<ntot; i++) {
    im3d[i] = (maxI - im3d[i])/(maxI - minI + 1e-10); //invert to dark field.
  }

  imFlat = (float *) malloc(nxy * sizeof(float));
  bw = (float *) malloc(nxy * sizeof(float));


  Stack *result = C_Stack::make(GREY, C_Stack::width(stack),
                                C_Stack::height(stack), 1);
  Zero_Stack(result);

  for (islab=0;islab <nSplit;islab++) {
    z1 = nz/nSplit * islab;
    z2 = (int) fmin(nz,nz/nSplit * (islab+1));	//work on sub-slab

    //maximum intensity projection.
    for (i=0; i<nx; i++) {
      for (j=0; j<ny; j++) {
        minI = HUGE_NUMBER2;
        for (k=z1; k<z2; k++) {
          ii = getIndex3DZfirst(i,j,k,nx,ny,nz);
          if (minI > im3d[ii]) minI = im3d[ii];
        }
        ii = getIndex(i,j,nx,ny);
        imFlat[ii] = minI;
      }
    }
    //create mask.
    createMask2D(nx,ny,imFlat,bw);

    size_t voxelNumber = C_Stack::voxelNumber(result);
    for (size_t i = 0; i < voxelNumber; ++i) {
      if (bw[i] > 0.5) {
        result->array[i] = 1;
      }
    }
  }

  //      free(im3d);
  free(imFlat);
  free(bw);

  C_Stack::kill(floatStack);

  return result;
}

void JNeuronTracer::gaussianFilter2D(
    int nx, int ny, unsigned int Wx, unsigned int Wy,
    float *imgIn, float *img, float sigma)
{
  // for filter kernel
  float sigma_s2 = 0.5/(sigma*sigma); // 1/(2*sigma*sigma)
  float pi_sigma = 1.0/(sqrt(2*3.1415926)*sigma); // 1.0/(sqrt(2*pi)*sigma)
  float min_val = HUGE_NUMBER, max_val = 0;
  int i,Weight,ix,iy;
  float  *WeightsX = 0,*WeightsY=0;
  float Half,x,y,k,sum;
  float  *extension_bufferX = 0,*extension_bufferY = 0;
  float  *extStop,*extIter,*stop_line,*arrLeft,*extLeft,*extRight,*arrRight,*resIter;
  float  *weightIter, *End,*arrIter;
  unsigned int offset;

  for (i=0; i<nx*ny; i++) img[i] = imgIn[i];

  //create Gaussian kernel
  WeightsX = (float *) malloc(Wx*sizeof(float));

  // Gaussian filter equation:
  // http://en.wikipedia.org/wiki/Gaussian_blur
  //   for (unsigned int Weight = 0; Weight < Half; ++Weight)
  //   {
  //        const float  x = Half* float (Weight) / float (Half);
  //         WeightsX[(int)Half - Weight] = WeightsX[(int)Half + Weight] = pi_sigma * exp(-x * x *sigma_s2); // Corresponding symmetric WeightsX
  //    }
  Half = (float)(Wx-1)/2.0;
  for (Weight = 0; Weight <= Half; ++Weight){
    x = Weight -Half;
    WeightsX[Weight]= pi_sigma * exp(-(x * x *sigma_s2)); // Corresponding symmetric WeightsX
    WeightsX[Wx-Weight-1] = WeightsX[Weight];
  }

  k = 0.;
  for (Weight = 0; Weight < (int) Wx; ++Weight) k += WeightsX[Weight];
  for (Weight = 0; Weight < (int) Wx; ++Weight) WeightsX[Weight] /= k;

  //   Allocate 1-D extension array
  extension_bufferX = (float *) malloc((nx + (Wx<<1))*sizeof(float));
  offset = Wx>>1;

  //	along x
  extStop = extension_bufferX + nx + offset;

  for(iy = 0; iy < ny; iy++) {
    extIter = extension_bufferX + Wx;
    for(ix = 0; ix < nx; ix++) {
      *(extIter++) = img[iy*nx + ix];
    }

    //   Extend image
    stop_line = extension_bufferX - 1;
    extLeft = extension_bufferX + Wx - 1;
    arrLeft = extLeft + 2;
    extRight = extLeft + nx + 1;
    arrRight = extRight - 2;

    while (extLeft > stop_line){
      *(extLeft--) = *(arrLeft++);
      *(extRight++) = *(arrRight--);
    }

    //	Filtering
    extIter = extension_bufferX + offset;

    resIter = &(img[iy*nx]);

    while (extIter < extStop) {
      sum = 0.;
      weightIter = WeightsX;
      End = WeightsX + Wx;
      arrIter = extIter;
      while (weightIter < End)
        sum += *(weightIter++) * 1.0 * (*(arrIter++));
      extIter++;
      *(resIter++) = sum;

      //for rescale
      if(max_val<*arrIter) max_val = *arrIter;
      if(min_val>*arrIter) min_val = *arrIter;
    }

  }
  //de-alloc
  free(WeightsX); WeightsX=0;
  free(extension_bufferX); extension_bufferX=0;

  //create Gaussian kernel
  WeightsY = (float *)malloc(Wy * sizeof(float));
  Half = (float)(Wy-1)/2.0;
  for (Weight = 0; Weight <= Half; ++Weight) {
    y = Weight-Half;
    WeightsY[Weight] =  pi_sigma * exp(-(y * y *sigma_s2)); // Corresponding symmetric WeightsY
    WeightsY[Wy-Weight-1] = WeightsY[Weight];
  }

  k = 0.;
  for (Weight = 0; Weight < (int) Wy; ++Weight) k += WeightsY[Weight];
  for (Weight = 0; Weight < (int) Wy; ++Weight) WeightsY[Weight] /= k;

  //	along y
  extension_bufferY = (float *) malloc((ny + (Wy<<1))*sizeof(float));
  offset = Wy>>1;
  extStop = extension_bufferY + ny + offset;

  for(ix = 0; ix < nx; ix++) {
    extIter = extension_bufferY + Wy;
    for(iy = 0; iy < ny; iy++) {
      *(extIter++) = img[iy*nx + ix];
    }

    //   Extend image
    stop_line = extension_bufferY - 1;
    extLeft = extension_bufferY + Wy - 1;
    arrLeft = extLeft + 2;
    extRight = extLeft + ny + 1;
    arrRight = extRight - 2;

    while (extLeft > stop_line) {
      *(extLeft--) = *(arrLeft++);
      *(extRight++) = *(arrRight--);
    }

    //	Filtering
    extIter = extension_bufferY + offset;

    resIter = &(img[ix]);

    while (extIter < extStop){
      sum = 0.;
      weightIter = WeightsY;
      End = WeightsY + Wy;
      arrIter = extIter;
      while (weightIter < End)
        sum += *(weightIter++) * 1.0 * (*(arrIter++));
      extIter++;
      *resIter = sum;
      resIter += nx;

      //for rescale
      if(max_val<*arrIter) max_val = *arrIter;
      if(min_val>*arrIter) min_val = *arrIter;
    }

  }
  //de-alloc
  free(WeightsY); WeightsY=0;
  free(extension_bufferY); extension_bufferY=0;
}

ZSwcTree* JNeuronTracer::trace(const Stack *stack, bool doResampleAfterTracing)
{
  Stack *maskData = makeMask(stack);

  ZStackSkeletonizer skeletonizer;

  ZSwcTree *wholeTree = skeletonizer.makeSkeleton(maskData);

  C_Stack::kill(maskData);

  if (wholeTree != NULL) {
    ZSwcPositionAdjuster adjuster;
    adjuster.setSignal(const_cast<Stack*>(stack), NeuTube::IMAGE_BACKGROUND_DARK);
    adjuster.adjustPosition(*wholeTree);

    Biocytin::SwcProcessor::breakZJump(wholeTree, 2.0);
    Biocytin::SwcProcessor::removeOrphan(wholeTree);
    Biocytin::SwcProcessor::smoothZ(wholeTree);

    skeletonizer.setConnectingBranch(true);
    skeletonizer.reconnect(wholeTree);

//    swcFrame->document()->estimateSwcRadius(wholeTree);

//    Biocytin::SwcProcessor::smoothRadius(wholeTree);


    if (doResampleAfterTracing) {
      ZSwcResampler resampler;
      resampler.optimalDownsample(wholeTree);
    }

//    if (stackFrame != NULL) {
//      wholeTree->translate(stackFrame->document()->getStackOffset());
//    }

  }

  return wholeTree;
}
