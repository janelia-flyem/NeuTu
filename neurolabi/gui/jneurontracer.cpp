#include "jneurontracer.h"
#include "zstackskeletonizer.h"
#include "zswcpositionadjuster.h"
#include "biocytin/swcprocessor.h"
#include "swc/zswcresampler.h"
#include "neutubeconfig.h"
#include "tz_stack_stat.h"
#include "tz_stack_math.h"

const double HUGE_NUMBER=1e30;
const double HUGE_NUMBER2=1e30;

JNeuronTracer::JNeuronTracer()
{
  m_splitNumber = 1;
  m_maskData = NULL;
  //  m_background = NeuTube::IMAGE_BACKGROUND_BRIGHT;
}

JNeuronTracer::~JNeuronTracer()
{
  if (m_maskData != NULL) {
    C_Stack::kill(m_maskData);
  }
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
  float *Is, *Ix, *Iy, *Ixy, *v, *u, *edge, dx=1.0, dy=1.0;
  unsigned int Wx, Wy;
  float thrSparse,lambdaRatioThr;
  int nthrs = 100, iter;
  float thrs[100];

  printf("Creating mask....\n");

  //PARAMETERS
  sigma = 1.0;			//Gaussian smoothing withs in pixel unit.
  Wx = 5; Wy = 5;			//window size for gaussian filter.
  mu = 10.0;				//width in gradient flow.
  dt = 0.02/mu;
  thrSparse = 0.1;		//sparseness of determining the threshold.
  expCompr = 0.1;			//compression exponent.
  lambdaRatioThr = 2.0;	//threshold for the ratio lambda1/lambda2 in Hessian to supress blob.
  iter = 200;				//levelset smoothing number of iterations.

  nxy = nx * ny;
  Is = (float *)malloc(nxy * sizeof(float));
  Ix = (float *)malloc(nxy * sizeof(float));
  Iy = (float *)malloc(nxy * sizeof(float));
  Ixy = (float *)malloc(nxy * sizeof(float));
  u = (float *)malloc(nxy * sizeof(float));
  v = (float *)malloc(nxy * sizeof(float));
  edge = (float *)malloc(nxy * sizeof(float));
  //createGrayTiffStackFromFloatArray(nx,ny,1,I,"temp.tif");
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

  for (i=0;i<nxy;i++) {
    edge[i] = v[i]*v[i] + u[i]*u[i];
  }

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
  //createGrayTiffStackFromFloatArray(nx,ny,1,v,"temp.tif");
  gaussianFilter2D(nx, ny, Wx, Wy, v, v, sigma);
  gaussianFilter2D(nx, ny, Wx, Wy, u, u, sigma);

  for (i=0;  i<nxy; i++) {
    if (v[i] > lambdaRatioThr*fabs(u[i])) {
      //bw[i] = pow(v[i],expComp) * exp(-5.0*abs(u[i])/(v[i]+1e-10));
      bw[i] = pow(v[i],expCompr);
    } else {
      bw[i] = 0.0;
    }
  }
  //createGrayTiffStackFromFloatArray(nx,ny,1,bw,"temp.tif");
#ifdef _DEBUG_2
  {
    Stack *checkStack = C_Stack::make(bw, GREY, ny, nx, 1);
    C_Stack::write(GET_TEST_DATA_DIR + "/test.tif", checkStack);
    C_Stack::kill(checkStack);
  }
#endif

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
  for (i=0;i<nxy;i++) {
    if (bw[i] > thrs[j]) {
      bw[i] = 1.0;
    } else {
      bw[i] = 0.0;
    }
  }

  levelsetSmoothing(nx, ny, bw, edge, iter);

  //createGrayTiffStackFromFloatArray(nx,ny,1,bw,"temp.tif");

  free(Is);
  free(Ix);
  free(Iy);
  free(u);
  free(v);
  free(Ixy);
  free(edge);
}

Stack* JNeuronTracer::makeMask(const Stack *stack)
{
  int nx,ny,nz,nchan,type,i,j,k,ntot,nxy,ii;
  float *im3d, *im3dG, *im3dB, *imFlat,maxI,minI;
  int nSplit,islab,z1,z2;
  float *bw,sigma;
  unsigned int Wx, Wy;

  //  if (argc > 1) {
  //      filename = argv[1];
  //      printf("Using  tiff image filename = %s\n",filename);
  //  } else {
  //      printf("ERROR: please sepcify the tiff stack filename as the first commandline argument.\n");
  //      return;
  //  }

  nSplit = 3;	//PARAMETER, number of sub slabs.

  //read image.
  //  getTiffAttribute(filename, tiffinfo);

#ifdef _DEBUG_2
  int width = C_Stack::width(stack);
  int height = C_Stack::height(stack);
  int depth = C_Stack::depth(stack);
#endif

  nx = C_Stack::height(stack);
  ny = C_Stack::width(stack);
  nz = C_Stack::depth(stack);
  nchan = 1;
  //  type = tiffinfo[4];
  printf("Image dimension=(%d %d %d) type=%d number of channels=%d\n",nx,ny,nz,type,nchan);
  ntot = nx * ny * nz;
  nxy = nx * ny;

  //  im3d = (float *) malloc(ntot*sizeof(float));
  imFlat = (float *) malloc(nxy * sizeof(float));
  bw = (float *) malloc(nxy * sizeof(float));


  Stack *floatStack = C_Stack::translate(const_cast<Stack*>(stack), FLOAT32, 0);


  im3d = C_Stack::guardedArrayFloat32(floatStack);

  Stack *result = C_Stack::make(GREY, C_Stack::width(stack),
                                C_Stack::height(stack), 1);
  Zero_Stack(result);

  if (nchan == 1) {
    printf("One channel stack, assume dark field...\n");
    //      readTiffStack(filename, tiffinfo, im3d, NULL, NULL);
    //reschale.
    maxI = -HUGE_NUMBER2;
    minI = HUGE_NUMBER2;
    for (i=0; i<ntot; i++) {
      if (maxI < im3d[i]) maxI = im3d[i];
      if (minI > im3d[i]) minI = im3d[i];
    }
    for (i=0; i<ntot; i++) {
      im3d[i] = (maxI - im3d[i])/(maxI - minI + 1e-10); //invert to bright field.
    }
  } else if (nchan == 3) {
    printf("Color stack, assuming biocytin bright field...\n");
    im3dG = (float *) malloc(ntot*sizeof(float));
    im3dB = (float *) malloc(ntot*sizeof(float));
    //      readTiffStack(filename, tiffinfo, im3d, im3dG, im3dB);
    maxI = -HUGE_NUMBER2;
    minI = HUGE_NUMBER2;
    for (i=0; i<ntot; i++) {
      im3d[i] = 0.2126 * im3d[i] + 0.7152 * im3dG[i] + 0.0722 * im3dB[i];
      if (maxI < im3d[i]) maxI = im3d[i];
      if (minI > im3d[i]) minI = im3d[i];
    }
    for (i=0; i<ntot; i++) {
      im3d[i] = (im3d[i] - minI)/(maxI - minI + 1e-10); //invert to dark field.
    }
    free(im3dG);
    free(im3dB);
  } else {
    printf("ERROR: The number of channels in the image is currently assumed to be 1 or 3. \n");
    return NULL;
    //      return;
  }

  //smooth each plane in the tiff stack
  sigma = 1.0;
  Wx = 5; Wy = 5;
  for (k=0; k<nz; k++) {
    for (i=0;i<nxy;i++) imFlat[i] = im3d[k*nxy+i];
    gaussianFilter2D(nx, ny, Wx, Wy, im3d+i, im3d+i, sigma);
  }

#ifdef _DEBUG_2
  {
    Stack *checkStack = C_Stack::make(im3d, GREY, width, height, depth);
    C_Stack::write(GET_TEST_DATA_DIR + "/test.tif", checkStack);
    C_Stack::kill(checkStack);
  }
#endif

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
    //      sprintf(outName,"%s.imFlat%d.tif",filename,islab);
    //      printf("Saving maximum projection for slab %d to %s...\n",islab,outName);
    //      createGrayTiffStackFromFloatArray(nx,ny,1,imFlat,outName);

#if 0
    Stack *tmpStack = Scale_Float_Stack(
          imFlat, C_Stack::width(stack), C_Stack::height(stack), 1, GREY);
    Stack_Invert_Value(tmpStack);
    int commonIntensity = Stack_Common_Intensity(tmpStack, 0, 65535);
    Stack_Subc(tmpStack, commonIntensity);

    size_t area = C_Stack::area(tmpStack);
    for (size_t i = 0; i < area; ++i) {
      imFlat[i] = tmpStack->array[i];
    }

#ifdef _DEBUG_2
  C_Stack::write(GET_TEST_DATA_DIR + "/test.tif", tmpStack);
#endif

    C_Stack::kill(tmpStack);
#endif

    //create mask.
    createMask2D(nx,ny,imFlat,bw);

#ifdef _DEBUG_2
    {
      Stack *checkStack = C_Stack::make(
            bw, GREY, C_Stack::width(stack), C_Stack::height(stack), 1);
      C_Stack::write(GET_TEST_DATA_DIR + "/test.tif", checkStack);
      C_Stack::kill(checkStack);
    }
#endif

    size_t voxelNumber = C_Stack::voxelNumber(result);
    for (size_t i = 0; i < voxelNumber; ++i) {
      if (bw[i] > 0.5) {
        result->array[i] = 1;
      }
    }

    //      sprintf(outName,"%s.imFlat%d.Mask.tif",filename,islab);
    //      printf("Saving mask for slab %d to %s...\n",islab,outName);
    //      createGrayTiffStackFromFloatArray(nx,ny,1,bw,outName);

  }

  //  free(im3d);
  free(imFlat);
  free(bw);

  C_Stack::kill(floatStack);

  return result;

#if 0
  int nx,ny,nz,i,j,k,ntot,nxy,ii;
  float *imFlat,maxI,minI;
  int nSplit,islab,z1,z2;
  float *bw,sigma;
  unsigned int Wx, Wy;

  nSplit = m_splitNumber;

  Stack *floatStack = C_Stack::translate(const_cast<Stack*>(stack), FLOAT32, 0);

  float* im3d = C_Stack::guardedArrayFloat32(floatStack);

  ny = C_Stack::width(stack);
  nx = C_Stack::height(stack);
  nz = C_Stack::depth(stack);
  //  im3d = (float *) malloc(nx*ny*nz*sizeof(float));
  //      readTiffStack(filename, tiffinfo, im3d, NULL, NULL);
  ntot = nx*ny*nz;
  nxy = nx * ny;

  Stack *originalStack = C_Stack::translate(const_cast<Stack*>(stack), FLOAT32, 0);
  float* original = C_Stack::guardedArrayFloat32(originalStack);

  //smooth each plane in the tiff stack
  sigma = 1.0;
  Wx = 5; Wy = 5;
  for (k=0; k<nz; k++) {
    i = k*nxy;
    gaussianFilter2D(nx, ny, Wx, Wy, im3d+i, im3d+i, sigma);
  }

  C_Stack::kill(originalStack);

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

#ifdef _DEBUG_
  {
    Stack *checkStack = C_Stack::make(im3d, GREY, ny, nx, nz);
    C_Stack::write(GET_TEST_DATA_DIR + "/test.tif", checkStack);
    C_Stack::kill(checkStack);
  }
#endif

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

#ifdef _DEBUG_
  Stack *checkStack = C_Stack::make(imFlat, GREY, ny, nx, 1);
  C_Stack::write(GET_TEST_DATA_DIR + "/test.tif", checkStack);
  C_Stack::kill(checkStack);
#endif

  //      free(im3d);
  free(imFlat);
  free(bw);

  C_Stack::kill(floatStack);

  return result;
#endif

}

void JNeuronTracer::gaussianFilter2D(
    int nx, int ny, unsigned int Wx, unsigned int Wy, float *imgIn, float *img, float sigma)
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
      *(extIter++) = img[ix*ny + iy]; //copy img to extension_bufferX
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
    resIter = &(img[iy]);
    while (extIter < extStop) {
      sum = 0.;
      weightIter = WeightsX;
      End = WeightsX + Wx;
      arrIter = extIter;
      while (weightIter < End)
        sum += *(weightIter++) * 1.0 * (*(arrIter++));
      extIter++;
      *(resIter) = sum;
      resIter += ny;

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
      *(extIter++) = img[ix*ny + iy];
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

    resIter = &(img[ix*ny]);

    while (extIter < extStop){
      sum = 0.;
      weightIter = WeightsY;
      End = WeightsY + Wy;
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
  free(WeightsY); WeightsY=0;
  free(extension_bufferY); extension_bufferY=0;
}

//levelset method.
void NeumannBoundCond(int nrow, int ncol, float *g)
{
  // Make a function satisfy Neumann boundary condition
  int i;

  g[getIndex(0,0,nrow,ncol)] = g[getIndex(2,2,nrow,ncol)];
  g[getIndex(0,ncol-1,nrow,ncol)] = g[getIndex(2,ncol-3,nrow,ncol)];
  g[getIndex(nrow-1,0,nrow,ncol)] = g[getIndex(nrow-3,2,nrow,ncol)];
  g[getIndex(nrow-1,ncol-1,nrow,ncol)] = g[getIndex(nrow-3,ncol-3,nrow,ncol)];

  for (i=1;i<ncol-1;i++) {
    g[getIndex(0,i,nrow,ncol)] = g[getIndex(2,i,nrow,ncol)];
    g[getIndex(nrow-1,i,nrow,ncol)] = g[getIndex(nrow-3,i,nrow,ncol)];
  }
  for (i=1;i<nrow;i++) {
    g[getIndex(i,0,nrow,ncol)] = g[getIndex(i, 2,nrow,ncol)];
    g[getIndex(i,ncol-1,nrow,ncol)] = g[getIndex(i,ncol-3,nrow,ncol)];
  }
}

float Dirac(float x, float sigma)
{
  float f;
  if (x <= sigma && x >= -sigma) {
    f =(1./2./sigma)*(1.+cos(3.14156*x/sigma));
  } else {
    f = 0.0;
  }
  return f;
}

void gradient2d(int nx, int ny, float* g, float* vx, float *vy)
{
  int i,j,ii,jj,kk;
  for (i=1;i<nx-1;i++) {
    for (j=0; j< ny; j++) {
      ii = getIndex(i,j,nx,ny);
      jj = getIndex(i+1,j,nx,ny);
      kk = getIndex(i-1,j,nx,ny);
      vx[ii] = (g[jj] - g[kk])/2.0;
    }
  }
  for (j=0; j<ny; j++) {
    ii = getIndex(0,j,nx,ny);
    jj = getIndex(1,j,nx,ny);
    vx[ii] = g[jj] - g[ii];
    ii = getIndex(nx-1,j,nx,ny);
    jj = getIndex(nx-2,j,nx,ny);
    vx[ii] = g[ii] - g[jj];
  }
  for (j=1;j<ny-1;j++) {
    for (i=0; i< nx; i++) {
      ii = getIndex(i,j,nx,ny);
      jj = getIndex(i,j+1,nx,ny);
      kk = getIndex(i,j-1,nx,ny);
      vy[ii] = (g[jj] - g[kk])/2.0;
    }
  }
  for (i=0; i<nx; i++) {
    ii = getIndex(i,0,nx,ny);
    jj = getIndex(i,1,nx,ny);
    vy[ii] = g[jj] - g[ii];
    ii = getIndex(i,ny-1,nx,ny);
    jj = getIndex(i,ny-2,nx,ny);
    vy[ii] = g[ii] - g[jj];
  }
}

void laplacian2d(int nx, int ny, float *u, float *lap)
{
  int i, j,ii,i1,i2,j1,j2;
  for (i=1; i<nx-1; i++) {
    for (j=1; j<ny-1; j++) {
      ii = getIndex(i,j,nx,ny);
      i1 = getIndex(i-1,j,nx,ny);
      i2 = getIndex(i+1,j,nx,ny);
      j1 = getIndex(i,j-1,nx,ny);
      j2 = getIndex(i,j+1,nx,ny);
      lap[ii] = u[i1] + u[i2] + u[j1] + u[j2] - 4.0*u[ii];
    }
  }
}

void curvature_central(int nx,int ny, float *ux, float *uy, float *K)
{
  int i, j, ii, jj, kk;
  float uxx, uyy;
  for (i=1; i<nx-1; i++) {
    for (j=1; j<ny-1; j++) {
      ii = getIndex(i,j,nx,ny);
      jj = getIndex(i,j+1,nx,ny);
      kk = getIndex(i,j-1,nx,ny);
      uyy = (uy[jj] - uy[kk])/2.0;
      jj = getIndex(i+1,j,nx,ny);
      kk = getIndex(i-1,j,nx,ny);
      uxx = (ux[jj] - ux[kk])/2.0;
      K[ii] = uxx + uyy;
    }
  }
}

//u is the level set function, g is the gradient function.
void levelSetEvolution(int nx, int ny, float *u, float *g, float lam, float mu, float alf, float epsilon, float delt, int numIter)
{
  int i, k, nxy;
  float *vx, *vy, *ux, *uy, *K, *Nx, *Ny, *lap;
  float normDu, diracU, terms, weightedLengthTerm,penalizingTerm,weightedAreaTerm;
  nxy = nx * ny;
  vx = (float *) malloc(nxy * sizeof(float));
  vy = (float *) malloc(nxy * sizeof(float));
  ux = (float *) malloc(nxy * sizeof(float));
  uy = (float *) malloc(nxy * sizeof(float));
  Nx = (float *) malloc(nxy * sizeof(float));
  Ny = (float *) malloc(nxy * sizeof(float));
  K = (float *) malloc(nxy * sizeof(float));
  lap = (float *) malloc(nxy * sizeof(float));

  gradient2d(nx, ny, g, vx, vy);

  for (k=0; k<numIter; k++){
    NeumannBoundCond(nx,ny,u);
    gradient2d(nx,ny,u,ux,uy);

    for (i=0; i<nxy; i++) {
      normDu=sqrt(ux[i]*ux[i] + uy[i]*uy[i] + 1e-10);
      Nx[i]=ux[i]/normDu;
      Ny[i]=uy[i]/normDu;
    }
    //compute curvature.
    curvature_central(nx,ny,Nx,Ny,K);
    //compute laplacian
    laplacian2d(nx, ny, u, lap);
    for (i=0; i<nxy; i++) {
      diracU=Dirac(u[i],epsilon);
      weightedLengthTerm=lam*diracU*(vx[i]*Nx[i] + vy[i]*Ny[i] + g[i]*K[i]);
      weightedAreaTerm=alf*diracU*g[i];
      penalizingTerm=mu*(lap[i]-K[i]);
      terms = weightedLengthTerm + weightedAreaTerm + penalizingTerm;
      u[i] = u[i] +delt*terms;  //update the level set function
    }
  }
  free(vx);
  free(vy);
  free(ux);
  free(uy);
  free(K);
  free(Nx);
  free(Ny);
  free(lap);
}

void JNeuronTracer::levelsetSmoothing(int nx, int ny, float *bw, float *edge,int iter)
{
  int i, nxy;
  float *u, *g;
  float mu, alf, epsilon, lam, delt, timestep, mmax, mmin;

  printf("Level set smoothing...\n");

  nxy = nx*ny;
  u = (float *)malloc(nxy * sizeof(float));
  g = (float *)malloc(nxy * sizeof(float));

  for (i=0; i<nxy; i++) u[i] = (bw[i] - 0.5) * 2.0;

  mmax = -HUGE_NUMBER;
  mmin = HUGE_NUMBER;
  for (i=0; i<nxy; i++) {
    if (mmax < edge[i]) mmax = edge[i];
    if (mmin > edge[i]) mmin = edge[i];
  }
  for (i=0; i<nxy; i++) {
    g[i]=1. / (1.e-5+pow((edge[i] - mmin)/(mmax - mmin),0.1));  	// edge indicator function.
  }
  epsilon = 0.5;				// the papramater in the definition of smoothed Dirac function
  timestep = 5;  		 		// time step
  mu=0.1/timestep;  			// coefficient for keeping u a signed function.
  // Note: the product timestep*mu must be less
  // than 0.25 for stability!
  lam=0.2; 					// coefficient of the weighted length term Lg(\phi)
  alf=0.0; 					// coefficient of the weighted area term Ag(\phi);
  // Note: Choose a positive(negative) alf if the
  // initial contour is outside(inside) the object.
  delt = 1.0;					// coefficient in updating u in each step.

  levelSetEvolution(nx, ny, u, g, lam, mu, alf, epsilon, delt, iter);

  for (i=0; i<nxy; i++) {
    if (u[i] > 0) {
      bw[i] = 1.0;
    } else {
      bw[i] = 0.0;
    }
  }

  free(u);
  free(g);
}

void JNeuronTracer::setResolution(const ZResolution &resolution)
{
  m_resolution = resolution;
}

ZSwcTree* JNeuronTracer::trace(const Stack *stack, bool doResampleAfterTracing)
{
  Stack *maskData = m_maskData;

  bool newMask = false;
  if (maskData == NULL) {
    maskData = makeMask(stack);
    newMask = true;
  }


  ZStackSkeletonizer skeletonizer;
  skeletonizer.setMinObjSize(10);

  ZSwcTree *wholeTree = skeletonizer.makeSkeleton(maskData);

  if (newMask) {
    C_Stack::kill(maskData);
  }

  if (wholeTree != NULL) {
    ZSwcPositionAdjuster adjuster;
    adjuster.setSignal(const_cast<Stack*>(stack), NeuTube::IMAGE_BACKGROUND_DARK);
    adjuster.adjustPosition(*wholeTree);

    Biocytin::SwcProcessor swcProcessor;
    swcProcessor.setResolution(m_resolution);
    swcProcessor.setMaxVLRatio(2.0);

    swcProcessor.breakZJump(wholeTree);
//    Biocytin::SwcProcessor::BreakZJump(wholeTree, 2.0);
    Biocytin::SwcProcessor::RemoveOrphan(wholeTree);
    Biocytin::SwcProcessor::SmoothZ(wholeTree);

    skeletonizer.setConnectingBranch(true);
    skeletonizer.reconnect(wholeTree);

    //    swcFrame->document()->estimateSwcRadius(wholeTree);

    //    Biocytin::SwcProcessor::smoothRadius(wholeTree);


    if (doResampleAfterTracing) {
      ZSwcResampler resampler;
      resampler.optimalDownsample(wholeTree);
    }

    Biocytin::SwcProcessor::BreakZJump(wholeTree, 2.0);

    //    if (stackFrame != NULL) {
    //      wholeTree->translate(stackFrame->document()->getStackOffset());
    //    }

  }

  return wholeTree;
}
