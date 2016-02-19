#from Tkinter import *
import PIL
import numpy
from numpy import newaxis
from scipy import *
from pylab import *
from skimage.morphology import label
from skimage.measure import regionprops	
from scipy.ndimage.morphology import binary_fill_holes
from RSF import *
from subprocess import call
import time
import ctypes
from scipy.misc import imresize
from matplotlib.patches import Rectangle
from llist import dllist	# np.double linked list
import statsmodels.api as sm			# for robust linear regression
from scipy.interpolate import interp1d
import os.path
from collections import OrderedDict
import glob
import random
from skimage.morphology import skeletonize

# specify the image filename common and overlap list

idataSet = 3
if idataSet == 1:
	filenameCommon = 'MC0509C3-'
	#overlapList = [(1,2),   (2,3),   (3,4),   (1,5),    (5,6),    (6,7),   (1,8),   (8,12),   (12,13), (8,9), (8,14), (14,17), (14,15), (15,16), (14,18)]
	overlapList = [(1,2), (2,3), (1,4), (4,5), (1,6), (6,7), (6,8),(6,10),(8,11),(1,12),(12,13),(13,14),(14,15),(12,16),(12,17),(17,18),(17,19),	(17,20),(20,21),(20,22),(22,23),(23,24),(24,25),(25,26),(23,27),(27,28),(28,29),(29,30),(30,31),(28,32),(32,33),(27,34),(34,35),(35,36),(34,37),(36,38)]
elif idataSet == 2:
	filenameCommon = 'DH070613-1-'
	overlapList = [(1,2),(2,3),(2,4),(4,5),(2,6),(6,7),(1,8),(8,9),(9,10),(10,11),(10,12),(12,13),(12,14),(14,15),(15,16),(16,17),(14,18),(18,19),(18,20)]
elif idataSet == 3:
	filenameCommon = 'DH070313-'
	overlapList = [(1,2),(1,3),(3,4),(3,5),(5,6),(6,7),(1,8),(8,9),(9,10)]

ddir = '/groups/visitors/home/jind/Dropbox/NeuralTracingImages/'	# directory for mask and flat images. 
# parameters 
minBranchLen = 10		# if the branch length is smaller than this value they are eliminated. 
maxZ = 3				# points with z jumping more than this value are disconnected.
radiusSmall = 3.0 		# points with radius smaller than this value are eliminated. 
somaPointRadius = 20	# Points with radius larger than this are assumed to be in the soma. 
minDistDisConnect = 20 	# connections longer than this are broke in the final stitched swc. 
maxDistConnect = 10		# loose ends close by within this this are connected. 
mag = 100				# microscope magnification
if mag == 60:
	xyDist = 195.44/1600.0	# pixel distance in microns in x y plane
	zDist = 1.0		# pixel distance in mcirons in z
elif mag == 100:
	xyDist = 118.11/1600.0
	zDist = 1.0
elif mag == 40:
	xyDist = 293.47/1600.0
	zDist = 1.0
saveDir = '/groups/visitors/home/jind/Dropbox/NeuralTracingImages/'

# These are typical routines.
def processOneStack(filenameBase): 
	#filenameBase = 'Pyramidal19'
	mag = 100	# magnification.
	im3d,imFlat=loadStacks(filenameBase+'.tif')
	#imGrad3d,maxGrad,zMaxGrad = getEdges(im3d)
	#bw = getBoundaries(imFlat,mag,iplot=1,filenameBase=filenameBase)
	bw = getMask(imFlat)
	bwz,bws = findZ(bw,im3d)
	# save the mask. 
	filenameSave = filenameBase+'.ana.npz'
	print 'Saving the segmentation to file '+filenameSave,
	numpy.savez(filenameSave,bw=bw,bwz=bwz)
	#bwz = findZ2(bw,zMaxGrad)
	createSWC(bw,bwz,bws,filenameBase)
	scaleSWC(filenameBase,mag)

def deleteIsolatedPoints(img,thr,minLen):
	nx,ny = img.shape
	iflag = 0
	for ii in range(nx):
		for jj in range(ny):
			if iflag == 0 and img[ii,jj] < thr:
				iflag = 1
				jj1 = jj
				num = 1
			elif iflag == 1 and  img[ii,jj] > thr:
				iflag = 0
				if num < minLen:
					img[ii,jj1:jj] = 1.0
			elif iflag == 1:
				num += 1
	iflag = 0
	for jj in range(ny):
		for ii in range(nx):
			if iflag == 0 and img[ii,jj] < thr:
				iflag = 1
				ii1 = ii
				num = 1
			elif iflag == 1 and  img[ii,jj] > thr:
				iflag = 0
				if num < minLen:
					img[ii1:ii,jj] = 1.0
			elif iflag == 1:
				num += 1



def getMask(imFlat,im3d):
	nx,ny = imFlat.shape
	img = imFlat.copy()
	img = subtractBackground(img)

	minLen = 5
	thr = percentile(img[where(img) < 0.95],10)
	deleteIsolatedPoints(img,thr,minLen)

	img = (1-img)**0.2	
	thr = percentile(img[where(img > 0.05)],80)
	bw = img > thr

	bw = 1 - bw
	deleteIsolatedPoints(bw,0.5,minLen)
	bw = 1 - bw
	bw_s = skeletonize(bw)
	bwz,bws = findZ(bw_s,im3d)

	print 'Saving the initialMask and flattened image...'
	img = PIL.Image.fromarray(((1-bw_s) * 255.0).astype('uint8'))
	img = img.convert("RGBA")
	datas = img.getdata()
	newData = []
	for item in datas:
		if item[0] == 255 and item[1] == 255 and item[2] == 255:
			newData.append((255, 255, 255, 0))
		else:
			newData.append((255, 0, 0, 255))
	img.putdata(newData)
	img.save(saveDir+filenameBase+'.mask.png')
	imsave(saveDir+filenameBase+'.flat.png',(imFlat * 255.0).astype('uint8'),cmap=cm.gray)

	tempFile1 = filenameBase+'.bw.tif'
	PIL.Image.fromarray((255*bw_s).astype(uint8)).convert('1').save(tempFile1)
	# call Ting's neutube program. 
	tempFile2 = filenameBase+'.bw.swc'
	call(['/groups/visitors/home/jind/neutube/neurolabi/cpp/build-skeletonize-Desktop-Debug/skeletonize',
		tempFile1,'-o', tempFile2,'--minlen','20', '--maxdist', '8', '--rebase', '--minobj', '20', '--keep_short'])
	# get rid of small points
	#call(['/groups/visitors/home/jind/neutube/neurolabi/c/bin/edswc','-prune_small','5',tempFile2,'-o',tempFile2])

	# get the linked lists of branches from swc file. Find Z. 
	LinkedPoints = getLinkedPointsFromSWC(tempFile2,nx,ny,bw=bw_s,bwz=bwz,bws=bws)
	call(['rm',tempFile1,tempFile2])

	# disconnect points with large jumps in z direction
	pids = []
	for PP in LinkedPoints:
		pids.append(PP.ID)
	for ii in range(len(LinkedPoints)):
		ll = []
		for pd in LinkedPoints[ii].conn:
			jj = pids.index(pd)
			if abs(LinkedPoints[ii].z - LinkedPoints[jj].z) > maxZ:
				ll.append(jj)
		for jj in ll:
			LinkedPoints[ii].delConn(pids[jj])
			LinkedPoints[jj].delConn(pids[ii])

	# create a branches structure from the linked points. 
	branches,branchConnIDs = createBranchesFromLinkedPoints(LinkedPoints)

	# delete small length branches
	minLen = 5 	
	for br in branches:
		if len(br) < minBranchLen:
			br.clear()
	branchConnIDs = repairBranches(branches,branchConnIDs)
	    
	saveSWC(filenameBase+'.swc',branches)
	
	return bw

def preprocessAllFiles(filenameCommon):
	files = glob.glob(filenameCommon+'*.tif')
	for fn in files:
		filenameBase = fn.split('.')[0]
		# see if this file has been processed. Check .npz file. 
		if os.path.exists(filenameBase+'.npz'):
			continue
		preprocessOneStack(filenameBase)

def processAllImages(filenameCommon=filenameCommon):
	files = glob.glob(filenameCommon+'*.tif')
	for fn in files:
		filenameBase = fn.split('.')[0]
		# see if this file has been processed. Check .npz file. 
		if os.path.exists(filenameBase+'.npz'):
			continue
		else:
			im3d,imFlat=loadStacks(filenameBase+'.tif')
			savez(filenameBase+'.npz',im3d=im3d)
			imsave(saveDir+filenameBase+'.flat.png',(imFlat * 255.0).astype('uint8'),cmap=cm.gray)
			imFlat = PIL.Image.fromarray((imFlat*255).astype('uint8'))
			imFlat.convert('L')
			imFlat.save(filenameBase+'.Flatten.tif')

def processImage(filenameBase):
	im3d,imFlat=loadStacks(filenameBase+'.tif')
	savez(filenameBase+'.npz',im3d=im3d)
	imsave(saveDir+filenameBase+'.flat.png',(imFlat * 255.0).astype('uint8'),cmap=cm.gray)
	imFlat = PIL.Image.fromarray((imFlat*255).astype('uint8'))
	imFlat.convert('L')
	imFlat.save(filenameBase+'.Flatten.tif')

def preprocessOneStack(filenameBase):
	# creat the .npz file to signal that this file is being processed. 
	savez(filenameBase+'.npz',mag=mag)
	im3d,imFlat=loadStacks(filenameBase+'.tif')
	# save im3d
	savez(filenameBase+'.npz',im3d=im3d)
	#imGrad3d,maxGrad,zMaxGrad = getEdges(im3d)
	bw = getBoundaries(imFlat,mag,iplot=0,filenameBase=filenameBase)
	print 'Saving the initialMask and flattened image...'
	img = PIL.Image.fromarray(((1-bw) * 255.0).astype('uint8'))
	img = img.convert("RGBA")
	datas = img.getdata()
	newData = []
	for item in datas:
		if item[0] == 255 and item[1] == 255 and item[2] == 255:
			newData.append((255, 255, 255, 0))
		else:
			newData.append((255, 0, 0, 255))
	img.putdata(newData)
	img.save(saveDir+filenameBase+'.mask.png')
	#imsave(saveDir+filenameBase+'.mask.png',(bw * 255.0).astype('uint8'))
	imsave(saveDir+filenameBase+'.flat.png',(imFlat * 255.0).astype('uint8'),cmap=cm.gray)
	imFlat = PIL.Image.fromarray((imFlat*255).astype('uint8'))
	imFlat.convert('L')
	imFlat.save(filenameBase+'.Flatten.tif')

def postProcessOneStack(filenameBase):
	im3d,imFlat=loadStacks(filenameBase+'.tif')
	bw=loadMaskFromFile(filenameBase)
	bwz,bws = findZ(bw,im3d)
	#imGrad3d,maxGrad,zMaxGrad = getEdges(im3d)
	#bwz = findZ2(bw,zMaxGrad)
	# save the mask. 
	filenameSave = filenameBase+'.ana.npz'
	print 'Saving the segmentation to file '+filenameSave,
	numpy.savez(filenameSave,bw=bw,bwz=bwz)
	createSWC(bw,bwz,bws,filenameBase)

def loadMaskFromFile(filenameBase):
	print 'Loading the initial mask from file...'
	saveDir = '/groups/visitors/home/jind/Dropbox/NeuralTracingImages/'
	bw = imread(saveDir+filenameBase+'.mask.Edit.png')
	bw= dot(bw[...,:3], [0.299, 0.587, 0.144])
	bw = (bw - bw.min())/(bw.max() - bw.min())
	bw = bw < 0.99
	bw = binary_fill_holes(bw)
	return bw

def subtractBackground(img):
	sigmaBackground = mag * 1.0 #60 # 100	# get the smooth background	
	img = ndimage.gaussian_filter(img,sigma=sigmaBackground) - img
	img[where(img < 0)] = 0
	img = img ** 0.3
	img = 1 - img/img.max()
	return img

def loadStacks(tiffFilename):
	print 'Loading stack image from ',tiffFilename

	sigmaSmooth = 1
	# load the image
	im = PIL.Image.open(tiffFilename)
	img = im.convert("L")
	imgarray = ndimage.gaussian_filter(array(img).astype(float),sigma=sigmaSmooth)
	image3d = array(imgarray)[...,newaxis]
	im.seek(0)
	try:
		while 1:
			im.seek(im.tell()+1)
			img = im.convert("L")	# convert to greyscale.
			imgarray = ndimage.gaussian_filter(array(img).astype(float),sigma=sigmaSmooth)
			image3d=append(image3d,imgarray[...,newaxis],2)		
	except EOFError:
		pass

	imFlat = amin(image3d,2)
	mmin = imFlat.min()
	mmax = imFlat.max()
	imFlat = (imFlat - mmin)/(mmax - mmin)

	return image3d,imFlat

def loadStacks2(tiffFilename):
	print 'Loading stack image from ',tiffFilename

	sigmaSmooth = 1
	# load the image
	im = PIL.Image.open(tiffFilename)
	img = im.convert("L")
	imgarray = array(img)
	image3d = array(imgarray)[...,newaxis]
	im.seek(0)
	try:
		while 1:
			im.seek(im.tell()+1)
			img = im.convert("L")	# convert to greyscale.
			#imgarray = ndimage.gaussian_filter(array(img).astype(float),sigma=sigmaSmooth)
			imgarray = array(img).astype(float)
			image3d=append(image3d,imgarray[...,newaxis],2)		
	except EOFError:
		pass

	imFlat = zeros(imgarray.shape)
	nx,ny = imgarray.shape
	for i in range(nx):
		for j in range(ny):
			smoothed = ndimage.gaussian_filter(image3d[i,j,:], sigma=2)
			imFlat[i,j] = smoothed.min()			

	mmin = imFlat.min()
	mmax = imFlat.max()
	imFlat = (imFlat - mmin)/(mmax - mmin)

	return image3d,imFlat
		
# Get edges using Sobel filter.
def getEdges(image3d):
	sigmaSmooth = 2
	imageGrad3d = zeros(image3d.shape)
	for i in range(image3d.shape[2]):
		sl = ndimage.gaussian_filter(image3d[:,:,i].astype(float),sigma=sigmaSmooth)
		dx = ndimage.sobel(sl, axis=0, mode='constant')
		dy = ndimage.sobel(sl, axis=1, mode='constant')
		grad = hypot(dx, dy)
		imageGrad3d[:,:,i]=grad
	zMaxGrad = argmax(imageGrad3d,axis=2)	
	maxGrad = amax(imageGrad3d,axis=2)
	#imshow(maxGrad)	
	return imageGrad3d,maxGrad,zMaxGrad

# Get the boundaries of the neuites using the flatterned intensity and maximum gradient images. 	
# Returns a bindary image with neurites labeled as 1.
def getBoundaries(imFlat,mag,iplot=1,filenameBase='pyramidalNeuron'):
	print 'Segmenting the image...'
	# parameters for extracting initial boundaries
	sigmaBackground = mag * 1.0 #60 # 100	# get the smooth background
	thrFracMax = 0.6	# threshold for getting the initial boundary
	
	# parameters for cleaning up the boundaries
	minNumPixel = 20	# regions smaller than 10 pixels are deleted as noise.

	# parameters for level set
	# initial iteration for finding the segment. Down scale and run for long iterations. 
	maxit = 30000
	lam = 0.05
	fract = 1.0	# fraction of shrinkage for the first pass. 
 
	sl = double(imFlat.copy())

	# get the initial boundary
	print 'Getting the initial boundary....'
	sls1 = ndimage.gaussian_filter(sl,sigma=sigmaBackground)
	sls = sls1 - sl
	sls[where(sls < 0)] = 0
	sls = 1 - sls/sls.max()

	init = sls < thrFracMax
	init = binary_fill_holes(init)

	# now get the boundary to the maximum gradient boundary using level set method.	
	# load the C program
	sfmLib = ctypes.CDLL('/groups/visitors/home/jind/NeuronMorphology/Codes/sparseFieldChanVeseC/sparseFieldChanVese.so')
	# set parameter types
	sfmLib.sparseFieldChanVese.argtypes = [ctypes.c_long, ctypes.c_long, ctypes.POINTER(ctypes.c_double), \
		ctypes.POINTER(ctypes.c_double), ctypes.POINTER(ctypes.c_double), ctypes.c_int, ctypes.c_double]
	sfmLib.sparseFieldChanVese.restype = None
	
	img = sls
	# resize
	imgT = np.double(imresize(img,fract))
	init2 = np.double(imresize(init,imgT.shape))
	init2 = init2/init2.max()
	# make sure there are no holes! watch out the type, need to convert to double!
	init2=np.double(binary_fill_holes(init2))
	nx,ny = imgT.shape
	phi = np.zeros(imgT.shape)
	# call the C function.
	sfmLib.sparseFieldChanVese(ctypes.c_long(nx), ctypes.c_long(ny), imgT.ctypes.data_as(ctypes.POINTER(ctypes.c_double)), \
			init2.ctypes.data_as(ctypes.POINTER(ctypes.c_double)), phi.ctypes.data_as(ctypes.POINTER(ctypes.c_double)), \
			ctypes.c_int(maxit), ctypes.c_double(lam))
	
	#print 'Getting rid of small regions...'
	bw = phi < 0
	bw_l = label(bw)
	for region in regionprops(bw_l, ['Area', 'Centroid','Label']):
		x,y = where(bw_l == region['Label'])
		if region['Area'] < minNumPixel:
			bw[x,y] = 0
	
	if iplot == 1:
		subplot(2,2,1); cla(); imshow(sls,cm.gray)
		subplot(2,2,2); cla(); imshow(init)
		subplot(2,2,3); cla(); imshow(sl,cm.gray); contour(phi,0,colors='r')
		subplot(2,2,4); cla(); imshow(bw)
		time.sleep(1)
		savefig(filenameBase+'.pdf')	
	return bw

def getBoundaries3(imFlat,maxGrad):
	# parameters for extracting initial boundaries
	sigmaBackground = 100	# get the smooth background
	sigmaForground = 2	# forground smoothing
	expFlattenTop = 0.2	# exponent for flattening the top of forground - backgorund
	thrFracMax = 0.85	# threshold for getting the initial boundary

	# parameters for level set method, Lu et al, IEEE Trans. Image Processing, vol. 17 (10), pp.1940-1949, 2008.
	iterNum =5
	lambda1 = 1.0
	lambda2 = 2.0
	nu = 0.0 #0.001*255*255	# coefficient of the length term
	timestep = .1 	# time step
	mu = 1.0 		# coefficient of the level set (distance) regularization term P(\phi)
	epsilon = 1.0 	# the papramater in the definition of smoothed Dirac function
	sigma=5.0    	# scale parameter in Gaussian kernel
			# Note: A larger scale parameter sigma, such as sigma=10, would make the LBF algorithm more robust 
			#       to initialization, but the segmentation result may not be as accurate as using
			#       a small sigma when there is severe intensity inhomogeneity in the image. If the intensity
			#       inhomogeneity is not severe, a relatively larger sigma can be used to increase the robustness of the LBF
			#       algorithm.
	
	# parameters for cleaning up the boundaries
	minNumPixel = 20	# regions smaller than 10 pixels are deleted as noise. 

	sl = double(imFlat.copy())
	sld = double(maxGrad.copy())
	sl = double(imFlat.copy())
	# make sure the boundary artfact is removed. 
	nx,ny = sld.shape
	sld[0,:] = 0; sld[nx-1,:] = 0; sld[:,0] = 0; sld[:,ny-1] = 0
	# smooth the gradient lanscape. 
	slds = sld

	# get the initial boundary
	print 'Getting the initial boundary....'
	sls1 = ndimage.gaussian_filter(sl,sigma=sigmaBackground)
	sls2 = ndimage.gaussian_filter(sl, sigma=sigmaForground) 
	sls = sls1 - sls2
	sls[where(sls < 0)] = 0
	sls = sls**expFlattenTop	# flatten the top. 
	u = sls > sls.max() * thrFracMax
	u = binary_fill_holes(u)
	u = (0.5 - u) * 4
	subplot(2,2,1); cla(); imshow(slds,cm.gray); contour(u,0, colors='r') 

	# now get the boundary to the maximum gradient boundary using level set method.	
	Img = sl.max() - sl
	K=gauss_kern(np.round(2*sigma)*2+1,sigma)     # the Gaussian kernel
	I = Img
	KI=conv2(Img,K,mode='same')     	# compute the convolution of the image with the Gaussian kernel outside the iteration
						# See Section IV-A in the above IEEE TIP paper for implementation.
	KONE=conv2(np.ones(Img.shape),K,mode='same') 	# compute the convolution of Gassian kernel and constant 1 outside the iteration
							# See Section IV-A in the above IEEE TIP paper for implementation.

	# start level set evolution
	print 'Compuiting the boundaries...'
	for n in range(iterNum):
		u=LSF(u,I,K,KI,KONE, nu,timestep,mu,lambda1,lambda2,epsilon,1)
		if np.mod(n,5)==0:
			print 'n=',n

	subplot(2,2,2); cla(); imshow(sl,cm.gray); contour(u,0, colors='r') 

	print 'Getting rid of small regions...'
	bw = u > 0
	bw_l = label(bw)
	for region in regionprops(bw_l, ['Area', 'Centroid','Label']):
		x,y = where(bw_l == region['Label'])
		if region['Area'] < minNumPixel:
			bw[x,y] = 0
	subplot(2,2,3); cla(); imshow(bw)
	return bw

#from phasecongmono import *
def getBoundaries2(imFlat,maxGrad):
	# parameters for extracting initial boundaries
	sigmaBackground = 100	# get the smooth background
	sigmaForground = 2	# forground smoothing
	expFlattenTop = 0.2	# exponent for flattening the top of forground - backgorund
	thrFracMax = 0.85	# threshold for getting the initial boundary

	# parameters for the level set method of Chuming Li et al, IEEE Trans Image Process 2010
	epsilon=1.5 	# the papramater in the definition of smoothed Dirac function
	timestep=0.5  	# time step
	mu=0.2/timestep # coefficient of the internal (penalizing) 
                  	# energy term P(\phi)
                  	# Note: the product timestep*mu must be less 
                  	# than 0.25 for stability!

	lam=10 		# coefficient of the weighted length term Lg(\phi)
	alf=1 		# coefficient of the weighted area term Ag(\phi);
      			# Note: Choose a positive(negative) alf if the 
      			# initial contour is outside(inside) the object.

	# parameters for cleaning up the boundaries
	minNumPixel = 20	# regions smaller than 10 pixels are deleted as noise. 

	sld = double(maxGrad.copy())
	sl = double(imFlat.copy())
	# make sure the boundary artfact is removed. 
	nx,ny = sld.shape
	sld[0,:] = 0; sld[nx-1,:] = 0; sld[:,0] = 0; sld[:,ny-1] = 0
	# smooth the gradient lanscape. 
	slds = sld
	#slds = ndimage.gaussian_filter(sld, sigma=sigmaForground)

	# get the initial boundary
	print 'Getting the initial boundary....'
	sls1 = ndimage.gaussian_filter(sl,sigma=sigmaBackground)
	sls2 = ndimage.gaussian_filter(sl, sigma=sigmaForground) 
	sls = sls1 - sls2
	sls[where(sls < 0)] = 0
	sls = sls**expFlattenTop	# flatten the top. 
	u = sls > sls.max() * thrFracMax
	u = binary_fill_holes(u)
	u = (u - 0.5) * 4
	subplot(2,2,1); cla(); imshow(slds,cm.gray); contour(u,0, colors='r') 

	# now get the boundary to the maximum gradient boundary using level set method.	
	M,ori,ft,T=phasecongmono(sl,nscale=6,minWaveLength=10) 
	g= 1- M**0.5  # edge indicator function.
	g = 1/(1+sld**0.5)
	g = M * sin(ft)
	for n in range(100):   
		u=evolution(u, g ,lam, mu, alf, epsilon, timestep, 1)
		if mod(n,20) == 0:
			print 'At level set step ',n
	subplot(2,2,2); cla(); imshow(sl,cm.gray); contour(u,0, colors='r') 

	print 'Getting rid of small regions...'
	bw = u > 0
	#bw_l = label(bw)
	#for region in regionprops(bw_l, ['Area', 'Centroid','Label']):
	#	x,y = where(bw_l == region['Label'])
	#	if region['Area'] < minNumPixel:
	#		bw[x,y] = 0
	subplot(2,2,3); cla(); imshow(bw)
	return bw

def findZ2(bw,zMaxGrad):
	bwd,indices = ndimage.distance_transform_edt(bw,return_indices=True)
	bwz = zeros(bw.shape)
	x,y = where(bw ==1)
	for px,py in zip(x,y):
		z = zMaxGrad[indices[0,px,py],indices[1,px,py]]
		bwz[px,py] = z
	return bwz

def detectPeak(y,thr):
	# detect peaks lower than thr. discard boundaries. If none exist, return sig = 0 and minimum. 
	ps = argmax(-y)
	pv = y.min()
	ii1 = 0
	ii2 = len(y)
	if ps == 0:	# this is a boundary maximum.
		for kk in range(1,len(y)):
			if y[kk] < y[kk-1]:
				ii1 = kk
			break
		if ii1 == 0:
			ii1 = len(y)
	elif ps == ii2-1:	# this is boundary maximum. 
		for kk in range(len(y)-1,0,-1):
			if y[kk] > y[kk-1]:
				ii2 = kk
			break
		if ii2 == len(y):
			ii2 = 0
	iid = where(y[ii1:ii2] < thr[ii1:ii2])[0]
	if len(iid) == 0:
		sig = 0
		return ps,pv,sig

	ps = argmax(-y[ii1:ii2]) + ii1
	pv = y[ii1:ii2].min()
	sig = 1
	return	ps,pv,sig


def findZ(bw,im3d):
	# paramerer. 
	thrPer = 2	# threshold for determing the significance of the peak. 
	smoothLen = 10	# length for smoothing 
	print 'Computing z...'
	bg = []
	x,y = where(bw ==0)
	nx,ny = bw.shape
	for ii in range(4000):
		iid = int(len(x) * random.random())
		smoothed = smooth(im3d[x[iid],y[iid],:],window_len=smoothLen)
		bg.append(smoothed)
	bg = array(bg) 
	thresholds = []
	bgmean = mean(bg,axis=0)
	for ii in range(bg.shape[1]):
		thresholds.append(percentile(bg[:,ii],thrPer))	# this is the threshold of significance for dark pixel. 
	thresholds = array(thresholds)

	bwz = zeros(bw.shape)	
	bws = zeros(bw.shape)	# significance of the peak. 
	x,y = where(bw ==1)
	for px,py in zip(x,y):
		smoothed = smooth(im3d[px,py,:],window_len=smoothLen)
		z,value,sig = detectPeak(smoothed-bgmean,thresholds-bgmean)
		#subplot(111); cla(); plot(thresholds-bgmean); plot(smoothed-bgmean); plot(z,value,'o')
		#raw_input('Press...')
		bwz[px,py] = z	
		bws[px,py] = sig
	return bwz,bws

class SWCPoint:
	def __init__(self,PP):
		self.ID, self.Type, self.x, self.y, self.z, self.r, self.parentID, self.zSig = PP

		
def smooth(x,window_len=11,window='hanning'):
    """smooth the data using a window with requested size.
    
    This method is based on the convolution of a scaled window with the signal.
    The signal is prepared by introducing reflected copies of the signal 
    (with the window size) in both ends so that transient parts are minimized
    in the begining and end part of the output signal.
    
    input:
        x: the input signal 
        window_len: the dimension of the smoothing window; should be an odd integer
        window: the type of window from 'flat', 'hanning', 'hamming', 'bartlett', 'blackman'
            flat window will produce a moving average smoothing.

    output:
        the smoothed signal
        
    example:

    t=linspace(-2,2,0.1)
    x=sin(t)+randn(len(t))*0.1
    y=smooth(x)
    
    see also: 
    
    numpy.hanning, numpy.hamming, numpy.bartlett, numpy.blackman, numpy.convolve
    scipy.signal.lfilter
 
    TODO: the window parameter could be the window itself if an array instead of a string
    NOTE: length(output) != length(input), to correct this: return y[(window_len/2-1):-(window_len/2)] instead of just y.
    """

    if x.ndim != 1:
        raise ValueError, "smooth only accepts 1 dimension arrays."

    if x.size < window_len:
        raise ValueError, "Input vector needs to be bigger than window size."


    if window_len<3:
        return x


    if not window in ['flat', 'hanning', 'hamming', 'bartlett', 'blackman']:
        raise ValueError, "Window is on of 'flat', 'hanning', 'hamming', 'bartlett', 'blackman'"


    s=numpy.r_[x[window_len-1:0:-1],x,x[-1:-window_len:-1]]
    #print(len(s))
    if window == 'flat': #moving average
        w=numpy.ones(window_len,'d')
    else:
        w=eval('numpy.'+window+'(window_len)')

    y=numpy.convolve(w/w.sum(),s,mode='valid')
    return y[(window_len/2-1):-(window_len/2)]

def extrap1d(interpolator):
    xs = interpolator.x
    ys = interpolator.y

    def pointwise(x):
        if x < xs[0]:
            return ys[0]+(x-xs[0])*(ys[1]-ys[0])/(xs[1]-xs[0])
        elif x > xs[-1]:
            return ys[-1]+(x-xs[-1])*(ys[-1]-ys[-2])/(xs[-1]-xs[-2])
        else:
            return interpolator(x)

    def ufunclike(xs):
        return array(map(pointwise, array(xs)))

    return ufunclike


def saveSWC(filenameSave,branches):
	print 'Saving SWC to file '+filenameSave 
	f = open(filenameSave,'w')
	for br in branches:
		for PP in br:
			ll = str(int(round(PP.ID)))+' ' \
				+str(int(round(PP.Type)))+' '+str(PP.y)+' '+str(PP.x)+' ' \
				+str(PP.z)+' '+str(PP.r)+' '+str(int(round(PP.parentID)))+'\n'
			f.write(ll)
	f.close()


def smoothBranchesTemp():
		zsmooth = zeros(len(indSel))
		rsmooth = zeros(len(indSel))
		ii1 = 0; ii2 = nlinfit	
		yy = zz[indSel[ii1:ii2]]
		xx = bind[indSel[ii1:ii2]]
		X = sm.add_constant(column_stack((xx**0,xx)))
		res1 = sm.RLM(yy,X).fit()
		yy = rr[indSel[ii1:ii2]]
		res2 = sm.RLM(yy,X).fit()
		zsmooth[0:nf+1] = res1.fittedvalues[0:nf+1]
		rsmooth[0:nf+1] = res2.fittedvalues[0:nf+1]
		for ii in range(nf+1,len(indSel)-nf):
			ii1 = ii - nf -1
			ii2 = ii + nf +1
			yy = zz[indSel[ii1:ii2]]
			xx = bind[indSel[ii1:ii2]]
			X = sm.add_constant(column_stack((xx**0,xx)))
			res1 = sm.RLM(yy,X).fit()
			yy = rr[indSel[ii1:ii2]]
			res2 = sm.RLM(yy,X).fit()
			zsmooth[ii] = res1.fittedvalues[nf+1]
			rsmooth[ii] = res2.fittedvalues[nf+1]
		ii1 = len(indSel)-nlinfit; ii2 = len(indSel)	
		yy = zz[indSel[ii1:ii2]]
		xx = bind[indSel[ii1:ii2]]
		X = sm.add_constant(column_stack((xx**0,xx)))
		res1 = sm.RLM(yy,X).fit()
		yy = rr[indSel[ii1:ii2]]
		res2 = sm.RLM(yy,X).fit()
		zsmooth[len(indSel)-nf:len(indSel)] = res1.fittedvalues[nf:-1]
		rsmooth[len(indSel)-nf:len(indSel)] = res2.fittedvalues[nf:-1]
		
		zsmooth = ndimage.gaussian_filter(zsmooth,sigma=1)
		rsmooth = ndimage.gaussian_filter(rsmooth,sigma=1)


def smoothBranches(branches,branchConnIDs):
	print 'Smoothing z positions of the branches...'
	# smooth z, radius
	minLen = 6
	print 'Smoothing the branches...'
	for ibr in range(len(branches)):
		if len(branches[ibr]) < 3:
			continue
		# find if there is a parent branch. 
		ipr = -1
		for pbr in range(len(branchConnIDs)):
			if ibr in branchConnIDs[pbr]:
				ipr = pbr
				break
		# find a the longest daughter branch.
		ll = []
		for jj in branchConnIDs[ibr]:
			ll.append(len(branches))
		
		idr = -1
		if len(ll) > 0:
			idr = argmax(ll)
		# construct the z array across the connected branches.		
		zz = []
		zs = []
		rr = []
		bind = []
		i1 = 0
		iid1 = 0
		if ipr != -1:
			br = branches[ipr]
			for ii in range(len(br)):
				PP = br[ii]
				if PP.r > somaPointRadius:
					continue
				bind.append(ii)
				zz.append(PP.z)
				zs.append(PP.zSig)
				rr.append(PP.r)
				i1 += 1
			iid1 = len(br)
		i2 = i1
		br = branches[ibr]
		for ii in range(len(br)):
			PP = br[ii]
			if PP.r > somaPointRadius:
				continue
			bind.append(ii+iid1)
			zz.append(PP.z)
			zs.append(PP.zSig)
			rr.append(PP.r)
			i2 += 1
		iid2 = iid1 + len(br)
		if idr != -1:
			br = branches[idr]
			for ii in range(len(br)):
				PP = br[ii]
				if PP.r > somaPointRadius:
					continue
				bind.append(ii+iid2)
				zz.append(PP.z)
				zs.append(PP.zSig)
				rr.append(PP.r)

		zz = array(zz)
		zs = array(zs)
		rr = array(rr)
		bind = array(bind)
		if len(zz) ==0:
			continue

		indSel = where(zs == 1)[0]
		zzs = ndimage.gaussian_filter(zz[indSel],sigma=1)
		rrs = ndimage.gaussian_filter(rr[indSel],sigma=1)
		divs = diff(zzs/rrs)/diff(bind[indSel])
		indSel = indSel[where(abs(divs) < 0.5)[0]+1]
				
		if len(indSel) < minLen:
			indSel = array(range(len(zz)))

		yy = zz[indSel]
		xx = bind[indSel]
		X = sm.add_constant(column_stack((xx**0,xx,xx**2,xx**3)))
		res1 = sm.RLM(yy,X).fit()
		yy = rr[indSel]
		#res2 = sm.RLM(yy,X).fit()
		zsmooth = res1.fittedvalues
		#rsmooth = res2.fittedvalues

		fz = interp1d(bind[indSel], zsmooth)
		znew = zeros(len(bind))
		ii1 = indSel[0]
		ii2 = indSel[-1]
		znew[ii1:ii2+1] = fz(bind[ii1:ii2+1])
		for ii in range(0,ii1):
			znew[ii] = znew[ii1]
		for ii in range(ii2+1,len(znew)):
			znew[ii] = znew[ii2]
		#fz2 = extrap1d(fz)
		#znew = fz2(bind)			
		#fr = interp1d(bind[indSel], rsmooth)
		#rnew = zeros(len(bind))
		#rnew[ii1:ii2+1] = fr(bind[ii1:ii2+1])
		#for ii in range(0,ii1):
		#	rnew[ii] = rnew[ii1]
		#for ii in range(ii2+1,len(rnew)):
		#	rnew[ii] = rnew[ii2]
		#fr2 = extrap1d(fr)
		#rnew = fr2(bind)
		for ii in range(i1,i2):
			branches[ibr][bind[ii]-iid1].z = znew[ii]
		#	branches[ibr][bind[ii]-iid1].r = rnew[ii] 

		#subplot(1,1,1); cla()
		#plot(bind,zz)
		#plot(bind,znew)
		#raw_input('Press any key...')		

def createSWC(bw,bwz,bws,filenameBase):
	# this function uses Ting Zhao's neutube program to turn the binary image bw into swc file. 
	# z-information is found from im3d.
	
	# parameters

	tempFile1 = filenameBase+'.bw.tif'
	PIL.Image.fromarray((255*bw).astype(uint8)).convert('1').save(tempFile1)
	# call Ting's neutube program. 
	tempFile2 = filenameBase+'.bw.swc'
	call(['/groups/visitors/home/jind/neutube/neurolabi/cpp/build-skeletonize-Desktop-Debug/skeletonize',
		tempFile1,'-o', tempFile2,'--minlen','10', '--maxdist', '8', '--rebase', '--minobj', '20', '--keep_short'])
	# get rid of small points
	#call(['/groups/visitors/home/jind/neutube/neurolabi/c/bin/edswc','-prune_small','5',tempFile2,'-o',tempFile2])

	# get the linked lists of branches from swc file. Find Z. 
	nx,ny=bw.shape
	LinkedPoints = getLinkedPointsFromSWC(tempFile2,nx,ny,bw=bw,bwz=bwz,bws=bws)
	call(['rm',tempFile1,tempFile2])

	# get rid of small radius points.
	pids = []
	rr = []
	for PP in LinkedPoints:
		pids.append(PP.ID)
		rr.append(PP.r)
	rr = array(rr)
	iid = where(rr < radiusSmall)[0]
	pidsRemove =[]	
	for ii in iid:
		pidsRemove.append(pids[ii])
		editList = []
		for pd in LinkedPoints[ii].conn:
			editList.append(pids.index(pd))
		for jj in editList:
			LinkedPoints[jj].conn.update(LinkedPoints[ii].conn)
	for PP in LinkedPoints:
		PP.conn.difference_update(pidsRemove)
		PP.conn.discard(PP.ID)
	LinkedPoints = [LinkedPoints[ii] for ii in range(len(LinkedPoints)) if not (ii in iid)]
		
	# disconnect points with large jumps in z direction
	#pids = []
	#for PP in LinkedPoints:
	#	pids.append(PP.ID)
	#for ii in range(len(LinkedPoints)):
	#	ll = []
	#	for pd in LinkedPoints[ii].conn:
	#		jj = pids.index(pd)
	#		if abs(LinkedPoints[ii].z - LinkedPoints[jj].z) > max([maxZ,LinkedPoints[jj].r,LinkedPoints[ii].r]):
	#			ll.append(jj)
	#	for jj in ll:
	#		LinkedPoints[ii].conn.discard(pids[jj])
	#		LinkedPoints[jj].conn.discard(pids[ii])

	# create a branches structure from the linked points. 
	branches,branchConnIDs = createBranchesFromLinkedPoints(LinkedPoints)

	# smooth z, radius
	smoothBranches(branches,branchConnIDs)

	# delete small length branches with small radius. 	
	for br in branches:
		maxr = 0
		for PP in br:
			maxr = max(maxr,PP.r)
		if len(br) < minBranchLen and maxr < 10:
			br.clear()

	branchConnIDs = repairBranches(branches,branchConnIDs)

	# save the swc file.		
	saveSWC(filenameBase+'.swc',branches)

def scaleSWC(filenameBase,mag):
	# this function rescales the SWC file with the real distances. 
	f = open(filenameBase+'.swc')
	lines = f.readlines()
	f.close()
	Points = [];
	for line in lines:
		ll = line.split(' ')
		nn = int(float(ll[0]))	# label of the point
		tp = int(float(ll[1]))  # point type
		py = float(ll[2])	# note the inversion of x, y.
		px = float(ll[3])	
		z  = float(ll[4])	# z
		r = float(ll[5])	# radius of the sphere. 
		np = int(float(ll[6]))	# parent point id. 			
		# get the length in micron
		py *= xyDist; px *= xyDist; r = r*xyDist; z *= zDist
		Points.append([nn,tp,py,px,z,r,np])
		
	print 'Saving SWC to file '+filenameBase+'.scaled.swc'
	f = open(filenameBase+'.scaled.swc','w')
	for [nn,tp,py,px,z,r,np] in Points:
		ll = str(int(nn))+' '+str(int(tp))+' '+str(py)+' '+str(px)+' '+str(z)+' '+str(r)+' '+str(int(np))+'\n'
		f.write(ll)
	f.close()


def sticthComputeOffSetOnePair(filenameCommon,overlapList):
	# this computes the stictching offset of one pair in the overlapList if the offset if not computed already. 
	# this is useful for running simtaneously across several processors. 
	import os.path
	from subprocess import call
	ddir = '/groups/visitors/home/jind/Dropbox/NeuralTracingImages/'
	filenameCommon = 'MC0509C3X100-'
	overlapList = [(1,2),   (2,3),   (3,4),   (1,5),    (5,6),    (6,7),   (1,8),   (8,12),   (12,13), (8,9),   (9,10),   (10,11), (8,14), (14,17), (14,15), (15,16), (14,18)]
	relPos = [     (0,1,0), (0,1,0), (-1,0,0),(0,-1,0), (0,-1,0), (0,-1,0),(1,0,0), (0,1,0),  (0,1,0), (0,-1,0),(0,-1,0), (0,-1,0),(1,0,0),(0,1,0), (0,-1,0),(0,-1,0),(1,0,0)]   
	for kk in range(len(overlapList)):
		(i,j) = overlapList[kk]
		filenameBase1 = filenameCommon+str(i)
		filenameBase2 = filenameCommon+str(j)
		outfile = filenameCommon+str(i)+'-'+str(j)+'.txt'
		if os.path.exists(outfile):
			continue
		print 'Stitching '+filenameBase1+'.tif'+' and '+filenameBase2+'.tif'
		connfile = outfile+'.conn.txt'
		f = open(connfile,'w')
		ox,oy,oz = relPos[kk]
		f.write('2\n')
		f.write('1 2 '+str(ox)+' '+str(oy)+' '+str(oz)+'\n')
		f.close()
		call(['/groups/visitors/home/jind/neutube/neurolabi/c/bin/stitchstack','-conn',connfile,filenameBase1+'.tif',filenameBase2+'.tif','-o',outfile])


def findConnectedXYZ(kk,sids,sidsAppear,overlapList,offsets,flags,X,Y,Z):
	# find a connected image that has been placed.
	for jj in sidsAppear[kk]:
		(m,n) = overlapList[jj]
		if m == sids[kk]:
			sgn = 1
		else:
			n = m
			sgn = -1
		nind = sids.index(n)
		if flags[nind] == 1:
			continue
		else:
			flags[nind] = 1
			X[nind] = X[kk] + sgn * offsets[jj][0]
			Y[nind] = Y[kk] + sgn * offsets[jj][1]
			Z[nind] = Z[kk] + sgn * offsets[jj][2]
			findConnectedXYZ(nind,sids,sidsAppear,overlapList,offsets,flags,X,Y,Z)
	return


def stitchImages(filenameCommon=filenameCommon,overlapList=overlapList):
	# This file sticthes the images together, and creates a summary flattened image of the neuron from the stacks.
	sids = []
	for (i,j) in overlapList:
		sids.append(i)
		sids.append(j)
	sids=OrderedDict.fromkeys(sids).keys()
	sidsAppear = [[] for i in range(len(sids))]
	k =0
	offsets = []
	for (i,j) in overlapList:
		sidsAppear[sids.index(i)].append(k); sidsAppear[sids.index(j)].append(k); k += 1
		filenameBase1 = filenameCommon+str(i)
		filenameBase2 = filenameCommon+str(j)
		outfile = filenameCommon+str(i)+'.'+filenameCommon+str(j)+'.txt'
		if os.path.exists(outfile):
			print 'Loading previous stitching result from '+outfile
			# load the result
			f = open(outfile)
			lines = f.readlines()
			f.close()
			dx,dy,dz = lines[0].split(',')
			dx = float(dx); dy = float(dy); dz = float(dz)
		else:
			print 'Computing the offset by stitching the tif files...'	
			dx,dy,dz=PairwiseStitch(filenameBase1,filenameBase2)
		offsets.append(array([dx,dy,dz]))
	for ss in sidsAppear:
		ss = OrderedDict.fromkeys(ss).keys()
	# construct the coordinates of the images. 
	X = zeros(len(sids))
	Y = zeros(len(sids)) 
	Z = zeros(len(sids))
	shapes = []
	flags = zeros(len(sids))
	flags[0] = 1
	findConnectedXYZ(0,sids,sidsAppear,overlapList,offsets,flags,X,Y,Z)
	X = X - X.min()
	Y = Y - Y.min()
	Z = Z - Z.min()

	# get the dimensions of the stacks. 
	shapes = []
	for ii in sids:
		im = PIL.Image.open(filenameCommon+str(ii)+'.tif')
		ny,nx = im.size
		nz = 0
		im.seek(0)
		try:
			while 1:
				im.seek(im.tell()+1)
				nz += 1
		except:
			pass
		shapes.append((nx,ny,nz))
	
	# construct a stiched 2D image
	for kk in range(len(sids)):
		ii = sids[kk]
		imFlat = imread(ddir+filenameCommon+str(ii)+'.flat.png')
		if kk == 0:
			nx,ny,nc = imFlat.shape
			imFlatCombined = zeros((int(X.max())+nx,int(Y.max())+ny,nc))
		imFlatCombined[int(X[kk]):int(X[kk])+nx,int(Y[kk]):int(Y[kk])+ny,:] = imFlat		
	cla(); subplot(1,1,1); imshow(imFlatCombined)
	print 'Saving the combined images to ',filenameCommon+'.flatCombined.png'
	im = PIL.Image.fromarray((array(imFlatCombined)*255).astype('uint8'))
	im.save(filenameCommon+'.flatCombined.png')
	
	outFile = filenameCommon+'StitchCoord.npz'
	print 'Saving stitch coordinates to ',outFile
	savez(outFile,X=X,Y=Y,Z=Z,IDs=sids,shapes=shapes,overlapList=overlapList,filenameCommon=filenameCommon)
 
def PairwiseStitch(filenameBase1,filenameBase2, iplot=0):

	try:
		im3d1 = numpy.load(filenameBase1+'.npz')['im3d']
	except:
		im3d1,imFlat1=loadStacks(filenameBase1+'.tif')
		savez(filenameBase1+'.npz',im3d=im3d1)
		imsave(saveDir+filenameBase1+'.flat.png',(imFlat1 * 255.0).astype('uint8'),cmap=cm.gray)
		imFlat1 = PIL.Image.fromarray((imFlat1*255).astype('uint8'))
		imFlat1.convert('L')
		imFlat1.save(filenameBase1+'.Flatten.tif')
	try:
		im3d2 = numpy.load(filenameBase2+'.npz')['im3d']
	except:
		im3d2,imFlat2=loadStacks(filenameBase2+'.tif')
		savez(filenameBase2+'.npz',im3d=im3d2)
		imsave(saveDir+filenameBase2+'.flat.png',(imFlat2 * 255.0).astype('uint8'),cmap=cm.gray)
		imFlat2 = PIL.Image.fromarray((imFlat2*255).astype('uint8'))
		imFlat2.convert('L')
		imFlat2.save(filenameBase2+'.Flatten.tif')

	# compute the X,Y shifts.
	print 'Computing x,y shift throght fiji stitch plug in...'
	fn1 = filenameBase1+'.Flatten.tif'
	fn2 = filenameBase2+'.Flatten.tif'

	# use ImageJ stitch
	filenameMarco = filenameBase1+'.'+filenameBase2+'.ijm'	
	ddir2 = os.getcwd()+'/'
	command = 'open("'+ddir2+fn1+'");open("'+ddir2+fn2+'");run("Pairwise stitching", "first_image='+fn1+' second_image='+fn2+ \
		' fusion_method=[Do not fuse images] check_peaks=5 compute_overlap subpixel_accuracy x=0.0000 y=0.0000 registration_channel_image_1=[Average all channels]' + \
		' registration_channel_image_2=[Average all channels]"); run("Quit")'
	f = open(filenameMarco,'w')
	f.write(command)
	f.close()
	fout = open(filenameMarco+'.txt','w')
	call(['/groups/visitors/home/jind/Fiji.app/ImageJ-linux64','--headless',filenameMarco],stdout=fout)
	fout.close()
	f = open(filenameMarco+'.txt','r')
	lines = f.readlines()
	f.close()
	# delete intermeidate files.
	call(['rm',filenameMarco+'.txt',filenameMarco])
	LL=lines[0].split(':')[1].split(' ')
	dy = int(round(float(LL[1][1:-2])))
	dx = int(round(float(LL[2][0:-1])))
	crr = float(LL[4][4:-1])
	print 'ImageJ stitching returns with dx=',dx,' dy=',dy,' crr=',crr

	# Use Ting's program 
	#outfile=filenameBase1+filenameBase2+'.tmp.txt'
	#call(['/groups/visitors/home/jind/neutube/neurolabi/c/bin/stitchstack',filenameBase1+'.Flatten.tif',filenameBase2+'.Flatten.tif','-o',outfile])
	#f = open(outfile,'r')
	#lines = f.readlines()
	#f.close()
	#call(['rm',outfile])
	#dy,dx,dz=lines[1].split(' ')[1][1:-1].split(',')
	#dx = float(dx)
	#dy = float(dy)	

	X = array([0,dx])
	Y = array([0,dy])
	X = X - X.min()
	Y = Y - Y.min()
	# construct a stiched 2D image
	if iplot == 1:
		figure(1)
		imFlat1 = imread(ddir+filenameBase1+'.flat.png')
		imFlat2 = imread(ddir+filenameBase2+'.flat.png')
		nx,ny,nc = imFlat1.shape
		imFlatCombined = ones((int(X.max()+nx),int(Y.max()+ny),nc))
		imFlatCombined[int(X[0]):int(X[0]+nx),int(Y[0]):int(Y[0]+ny),:] = imFlat1
		imFlatCombined[int(X[1]):int(X[1]+nx),int(Y[1]):int(Y[1]+ny),:] = imFlat2
		figure(1)
		cla(); imshow(imFlatCombined,cm.gray)

	# regions of overlap.
	#figure(2); clf()
	nx,ny,nz = im3d1.shape
	if dx < 0:
		x11 = 0; x21 = -dx 
		x12 = int(nx+dx); x22 = nx
	else:
		x21 = 0; x11 = dx 
		x22 = nx- dx; x12 = nx
	if dy < 0:
		y11 = 0; y21 = -dy 
		y12 = ny+dy; y22 = ny
	else:
		y21 = 0; y11 = int(dy); 
		y22 = int(ny - dy); y12 = ny
	if iplot == 1:
		figure(2)
		subplot(2,1,1); imshow(imFlat1[x11:x12,y11:y12],cm.gray)
		subplot(2,1,2); imshow(imFlat2[x21:x22,y21:y22],cm.gray)

	# find Z shift. 
	print 'Computing the z shift...',
	dn = 5	# subsample x, y
	z1 = im3d1[x11:x12:dn,y11:y12:dn,:]
	z2 = im3d2[x21:x22:dn,y21:y22:dn,:]
	nx,ny,nz1 = z1.shape
	nx,ny,nz2 = z2.shape
	
	# brutal force computing correlzation for each shift, 
	iflagUseBrutalForce = 1
	if iflagUseBrutalForce == 1:
		zscan = range(-nz2+1,nz1-1)
		crrs = []
		for iz in zscan:
			if iz < 0:
				zp = int(min(nz1,nz2 + iz))
				z11 = 0; z12= z11+zp
				z21 = -iz; z22 = z21+zp
			else:
				zp = int(min(nz1 - iz,nz2))
				z11 = iz; z12 = z11 + zp
				z21 = 0; z22 = zp
			zr1 = z1[:,:,z11:z12]
			a,b,c = zr1.shape; zr1 = reshape(zr1,a*b*c)
			zr2 = z2[:,:,z21:z22]
			a,b,c = zr2.shape; zr2 = reshape(zr2,a*b*c)
			crrs.append(corrcoef(zr1,zr2)[0][1])
		crrs = array(crrs)
		if iplot == 1:
			figure(3); subplot(2,1,1);cla()
			plot(crrs)	
		dz = zscan[argmax(crrs)]
		print 'Brutal force method returned dz=',dz,' corr=',crrs.max()
	else:		
		d1 = nx*ny*nz1
		d2 = nx*ny*nz2
		if nz1 < nz2:
			nz = nz2
			z1d = zeros(d2)
			z1d[0:d1] = reshape(z1.swapaxes(0,2),d1)
			z2d = reshape(z2.swapaxes(0,2),nx*ny*nz2)	
		else:
			nz = nz1
			z2d = zeros(d1)
			z2d[0:d2] = reshape(z2.swapaxes(0,2),d2)
			z1d = reshape(z1.swapaxes(0,2),nx*ny*nz1)
		crrs = (ifftn(fftn(z1d)*ifftn(z2d))).real
		dz = argmax(crrs)/(nx*ny)
		if dz > nz/2:
			dz = dz - nz
		if iplot == 1:
			subplot(1,1,1); cla();
			plot(crrs)
		print 'Phase correlatiion returns dz =',dz
	outfile = filenameBase1+'.'+filenameBase2+'.txt'
	print 'Saving the offsets to ',outfile
	f = open(outfile,'w')
	f.write(str(dx)+','+str(dy)+','+str(dz))
	f.close()
	return dx,dy,dz

def nearPointID(x,y,z,Points):
	dists = (xyDist*(Points[:,0]-x))**2 + (xyDist*(Points[:,1]-y))**2 + (zDist*(Points[:,2] - z))**2
	jjs = argmax(-dists)
	return jjs
	
class LinkedPoint:
	def __init__(self,x,y,z,r,ID,Type,zSig):
		self.x = x; self.y = y; self.z = z; self.r = r; self.ID = ID; self.Type = Type; self.zSig = zSig; self.conn = set()
	def addConn(self,ID):
		self.conn.add(ID)
	def delConn(self,ID):
		self.conn.discard(ID)	
	def numConn(self):
		return len(self.conn)
		
def getLinkedPointsFromSWC(swcfilename,nx,ny,bw=None,bwz=None,bws=None,idOffset=0,xOffset=0,yOffset=0,zOffset=0):
	# reload the swc file
	f = open(swcfilename)
	lines = f.readlines()
	f.close()

	# parse the swc file
	LinkedPoints = []
	pIDs = []
	IDs = []
	for line in lines:
		ll = line.split(' ')
		nn = float(ll[0]) + idOffset	# label of the point
		tp = float(ll[1])  	# point type
		py = float(ll[2]) + yOffset	# note the inversion of x, y.
		px = float(ll[3]) + xOffset
		r = float(ll[5])	# radius of the sphere. 
		zsig = 1
		z = float(ll[4])
		if bw!= None and bwz != None and bws !=None:
			# find z. 
			zAveSizeMin = 2		# minimum size of pixels for averaging z around each point. 
			zz = []
			zs = []
			zAveSize = min(zAveSizeMin,int(round(r * 0.3)))
			for ii in range(max(0,int(px)-zAveSize),min(nx,int(px)+zAveSize+1)):
				for jj in range(max(0,int(py)-zAveSize),min(ny,int(py)+zAveSize+1)):
					if bw[ii,jj] == False:
						continue;
					zz.append(bwz[ii,jj])
					zs.append(bws[ii,jj])
			if len(zz) > 0:
				z = mean(zz)
				zsig = sum(zs) == len(zs)
		z  += zOffset	# z
		np = float(ll[6]) 
		if np != -1:
			np += idOffset	# parent point id. 			
		pIDs.append(np)
		IDs.append(nn)
		LinkedPoints.append(LinkedPoint(px,py,z,r,nn,tp,zsig))

	for ii in range(len(pIDs)):
		pid = pIDs[ii]
		if pid == -1:
			continue
		iid=IDs.index(pid)
		LinkedPoints[ii].addConn(pid)
		LinkedPoints[iid].addConn(IDs[ii])	
	return LinkedPoints

def getCoordinates(LinkedPoints):
	Points = []
	IDs = []
	for PP in LinkedPoints:
		Points.append([PP.x,PP.y,PP.z])
		IDs.append(PP.ID)
	return array(Points),array(IDs)

def getAllChildBranchesLinkedPoints(jj,LinkedPointsAll,IDs,flags,branches,branchConnIDs):
	# this function gets all branches in the linked points starting from jj.
	br = dllist()
	brIDs = []
	while 1:
		PP = LinkedPointsAll[jj]
		if len(br) > 0:
			pid = br.last.value.ID
		else:
			pid = -1
		br.append(SWCPoint(array([PP.ID,PP.Type,PP.x,PP.y,PP.z,PP.r,pid,PP.zSig])))
		flags[jj] = 1
		# find the next point.
		kids = []
		for kk in PP.conn:
			iid = IDs.index(kk)
			if flags[iid] == 0:
				kids.append(kk)
		if len(kids) == 0:
			break
		elif len(kids) == 1:
			jj = IDs.index(kids[0])
		else:
			for kk in kids:
				iid = IDs.index(kk)
				bID  = getAllChildBranchesLinkedPoints(iid,LinkedPointsAll,IDs,flags,branches,branchConnIDs)
				brIDs.append(bID)	
			break		
	branches.append(br)
	branchConnIDs.append(brIDs)
	return len(branches)-1

def createBranchesFromLinkedPoints(LinkedPointsAll):
	IDs = []
	numConn = []
	for PP in LinkedPointsAll:
		IDs.append(PP.ID)
		numConn.append(len(PP.conn))
	flags = zeros(len(LinkedPointsAll))
	numConn = array(numConn)
	
	branches = []		# linked lists of the branches.
	branchConnIDs = []	# IDs of the branches connected to this branch. 
	
	while 1:
		ind = where(flags == 0)[0]
		if len(ind) == 0:
			break
		ii = argmax(-numConn[ind])
		jj = ind[ii]	
		print ii, jj
		getAllChildBranchesLinkedPoints(jj,LinkedPointsAll,IDs,flags,branches,branchConnIDs)

	# connect branches
	for ii in range(len(branches)):
		br = branches[ii]
		for jj in branchConnIDs[ii]:
			if len(branches[jj]) == 0:
				continue
			branches[jj].first.value.parentID = br.last.value.ID
	# smooth z, radius
	#smoothBranches(branches)
	return branches, branchConnIDs

def distanceTwoPoints(Point1,Point2):
		dist =  ((Point1[0] - Point2[0])*xyDist)**2 + \
				((Point1[1] - Point2[1])*xyDist)**2 + \
				((Point1[2] - Point1[2])*zDist )**2
		return sqrt(dist)

def getCoordinatesFromBranches(branches):
		Points = []
		IDs = []
		PIDs = []
		for br in branches:
			for PP in br:
					Points.append([PP.x,PP.y,PP.z])
					IDs.append(PP.ID)
					PIDs.append(PP.parentID)
		return array(Points),array(IDs),array(PIDs)

def getLinkedPointsFromBranches(branches):
	LinkedPoints = []
	pIDs = []
	IDs = []
	for br in branches:
		for PP in br:
			pIDs.append(PP.parentID)
			IDs.append(PP.ID)
			LinkedPoints.append(LinkedPoint(PP.x,PP.y,PP.z,PP.r,PP.ID,PP.Type,PP.zSig))

	for ii in range(len(pIDs)):
		pid = pIDs[ii]
		if pid == -1:
			continue
		iid=IDs.index(pid)
		LinkedPoints[ii].addConn(pid)
		LinkedPoints[iid].addConn(IDs[ii])	
	return LinkedPoints

def updateConnIDs(branches):
	pIDs = []
	for ii in range(len(branches)):
		br = branches[ii]
		if len(br) == 0:
			continue
		pIDs.append((ii,br.first.value.parentID))
	pIDs = array(pIDs)
	branchConnIDs = []
	for ii in range(len(branches)):
		br = branches[ii]
		if len(br) == 0:
			branchConnIDs.append([])
			continue
		iid = where(pIDs[:,1] == br.last.value.ID)[0]
		branchConnIDs.append(list(pIDs[iid,0].astype(int)))
	return branchConnIDs

def insertReverseOrder(indRemove,ii):
	if len(indRemove) == 0:
		indRemove.append(ii)
	else:
		flag = 0
		for kk in range(len(indRemove)):
			if ii > indRemove[kk]:
				indRemove.insert(kk,ii)
				flag = 1
				break
		if flag == 0:
			indRemove.append(ii)


def repairBranches(branches,branchConnIDs):
	# re-assign the IDs of the Points to make sure they are unique.
	#kk = 0 
	#for br in branches:
	#	if len(br) == 0:
	#		continue
	#	for PP in br:
	#		PP.ID = kk
	#		kk += 1

	# make sure the connectivity of the nodes are correct
	for ii in range(len(branches)):
		node = branches[ii].first
		if node == None:
			continue
		while 1:
			node = node.next
			if node == None:
				break
			if node.value.parentID != -1:
				node.value.parentID = node.prev.value.ID
		for bid in branchConnIDs[ii]:
			if len(branches[bid]) == 0:
				continue
			if branches[bid].first.value.parentID != -1: 
				branches[bid].first.value.parentID = branches[ii].last.value.ID 
	# check the possibility that the parent was delete for each branch.
	LID = []
	for br in branches:
		if len(br) == 0:
			continue;
		LID.append(br.last.value.ID)
	for br in branches:
		if len(br) == 0:
			continue;
		if not (br.first.value.parentID in LID):
			br.first.value.parentID = -1		
	# delete empty branches and update connctions. 
	indRemove = []
	for ii in range(len(branches)):
		if len(branches[ii]) == 0:	# remove empty branches
			insertReverseOrder(indRemove,ii)
	for ii in indRemove:
		branches.pop(ii)
	# recompute the connection IDs. 
	branchConnIDs = updateConnIDs(branches)					

	# merge non branching "branches"
	indRemove = []
	for ii in range(len(branches)):
		if ii in indRemove:
			continue
		if len(branchConnIDs[ii]) != 1:
			continue
		br = branches[ii]
		jj = branchConnIDs[ii][0]
		while 1:
			if jj in indRemove:
				break
			insertReverseOrder(indRemove,jj)
			node = branches[jj].first
			while 1:
				br.append(node)
				node = node.next
				if node == None:
					break
			branches[jj].clear()		
			if len(branchConnIDs[jj]) == 1:
				jj = branchConnIDs[jj][0]
			else:
				break
	for ii in indRemove:
		branches.pop(ii)
	# recompute the connection IDs. 
	branchConnIDs = updateConnIDs(branches)	
	return branchConnIDs			

def stitchSWCs(filenameCommon=filenameCommon,mag=mag):

	# load the coordinates genrated from stitchImages
	outFile = filenameCommon+'StitchCoord.npz'
	print 'Loading stitch coordinates from ',outFile
	try:
		res=numpy.load(outFile)
		X = res['X']
		Y = res['Y']
		Z = res['Z']
		fileIDs = res['IDs']
		shapes = res['shapes']
	except:
		print 'ERROR: No results from image stitching found. First run stitchImages.'
		return

	# create list of branches from the swc file, shift the coordinates using the stitch results. 
	LinkedPointsAll = []
	LeftFileIDs = range(len(fileIDs)-1,-1,-1)
	#LeftFileIDs = range(len(fileIDs))
	#LeftFileIDs = [15,11]
	npTot = 0
	while 1:
		ii = -1
		for jj in LeftFileIDs:
			try:
				filename = filenameCommon+str(fileIDs[jj])+'.Edit.swc'
				nx,ny,nz = shapes[jj]
				# get the linked points.
				LinkedPoints = getLinkedPointsFromSWC(filename,nx,ny,idOffset=npTot+1,xOffset=X[jj],yOffset=Y[jj],zOffset=Z[jj])
				ii = jj
				break
			except:
				continue
		if ii == -1:
			break
		else:
			LeftFileIDs.remove(ii)

		print 'Adding ',filename		
		npTot += len(LinkedPoints)
		
		if len(LinkedPointsAll) == 0:
			LinkedPointsAll = list(LinkedPoints)
			continue
		
		# find out which points are in the range of the tile. 
		x1 = X[ii]; x2 = X[ii] + nx
		y1 = Y[ii]; y2 = Y[ii] + ny
		z1 = Z[ii]; z2 = Z[ii] + nz
		
		PointsAll, IDsAll = getCoordinates(LinkedPointsAll)
		Points, IDs = getCoordinates(LinkedPoints)
		
		indIn = where((PointsAll[:,0] >= x1) & (PointsAll[:,0] < x2)		
				 &  (PointsAll[:,1] >= y1) & (PointsAll[:,1] < y2)    
				 &  (PointsAll[:,2] >= z1) & (PointsAll[:,2] < z2))[0]
		indOut = array([k for k in range(len(PointsAll)) if not (k in indIn)])
		
		# find boundary points and make connections. 
		IDsIn = IDsAll[indIn]
		for ii2 in indOut:
			delIDs=[]
			addIDs=[]
			for kk in LinkedPointsAll[ii2].conn:
				jj = where(IDsIn == kk)[0]
				if len(jj) > 0:
					x,y,z = PointsAll[indIn[jj[0]],:]
					jjs = nearPointID(x,y,z,Points)
					delIDs.append(kk)
					addIDs.append(IDs[jjs])
			for kk in delIDs:
				LinkedPointsAll[ii2].delConn(kk)
			for kk in addIDs:
				LinkedPointsAll[ii2].addConn(kk)
				iid = where(IDs == kk)[0]
				LinkedPoints[iid].addConn(IDsAll[ii2])
		# merged points outside and in the new tile. 	
		LinkedPointsAllNew = []
		for kk in indOut:		
			P1 = LinkedPointsAll[kk]
			PP = LinkedPoint(P1.x,P1.y,P1.z,P1.r,P1.ID,P1.Type,P1.zSig)
			PP.conn = set(P1.conn)
			LinkedPointsAllNew.append(PP)
		for P1 in LinkedPoints:
			PP = LinkedPoint(P1.x,P1.y,P1.z,P1.r,P1.ID,P1.Type,P1.zSig)
			PP.conn = set(P1.conn)
			LinkedPointsAllNew.append(PP)			
		LinkedPointsAll = list(LinkedPointsAllNew)
	
	# repair the connections. 
	# disconnection long connections that are artifacts of stitching process. 
	print 'Repairing stitches....'
	print 'Delete connections with distances larger than ',minDistDisConnect
	PointsAll, IDsAll = getCoordinates(LinkedPointsAll)
	for ii in range(len(LinkedPointsAll)):
			delIDs = []
			for kk in LinkedPointsAll[ii].conn:
				jj = where(IDsAll == kk)[0][0]
				dist = distanceTwoPoints(PointsAll[ii,:],PointsAll[jj,:])
				if dist > minDistDisConnect:
					delIDs.append(jj)
			for jj in delIDs:
				LinkedPointsAll[ii].delConn(IDsAll[jj])
				LinkedPointsAll[jj].delConn(IDsAll[ii])
																		
	# create the branches 
	branches, branchConnIDs = createBranchesFromLinkedPoints(LinkedPointsAll) 
	# delete small branches. 
	print 'Deleteing small terminal branches with nuber of points smaller than ', minBranchLen
	for ii in range(len(branches)):
		br = branches[ii]
		if len(br) == 0:
			continue
		if len(branchConnIDs[ii]) == 0 and len(br) < minBranchLen:
			br.clear()		
	branchConnIDs = repairBranches(branches,branchConnIDs)
	# connect nearby endpoints of branch points. 
	print 'Connecting end points of isolated branches within distance ',maxDistConnect
	indIsos = []
	indEnds = [] 
	for ii in range(len(branches)):
		br = branches[ii]
		if len(br) == 0:
			continue
		if len(branchConnIDs[ii]) > 1: 
			continue
		if len(branchConnIDs[ii]) == 0:
			indEnds.append(ii)	# this branch has a terminal ending. 		
		if br.first.value.parentID != -1:
			continue
		if len(branchConnIDs[ii]) == 1:	# chase the branch connections see it ends with no connections. 
			jj = branchConnIDs[ii][0]
			while 1:
				if len(branchConnIDs[jj]) == 0:
					flag = 1
					break
				elif len(branchConnIDs[jj]) > 1:
					flag = 0 
					break
				else:
					jj = branchConnIDs[jj][0]
			if flag == 0:
				continue						
		indIsos.append(ii)
	for ii in indIsos:
		P1S = branches[ii].first.value
		Point1S = array([P1S.x,P1S.y,P1S.z])
		P1E = branches[ii].last.value
		Point1E = array([P1E.x,P1E.y,P1E.z])	
		for jj in indEnds:
			if ii == jj:
				continue
			P2 = branches[jj].last.value
			Point2 = array([P2.x,P2.y,P2.z])
			dist1 = distanceTwoPoints(Point1S,Point2)
			dist2 = distanceTwoPoints(Point1E,Point2)	
			if dist1 < maxDistConnect:
				#print ii, dist1, 
				P1S.pranetID = P2.ID
				break
			elif dist2 < maxDistConnect:
				#print ii, dist2,
				# reverse the order of the branch. 
				brnew = dllist()
				node = branches[ii].last
				id = P2.ID
				while 1:
					node.value.parentID = id
					id = node.value.ID
					brnew.append(node)
					node = node.prev
					if node == None:
						break
				branches[ii] = brnew		
				break
								
	# smooth z and r.
	print 'Smoothing z and r...'
	for br in branches:
		if len(br) ==0:
			continue
		zz = []
		rr = []
		for PP in br:
			zz.append(PP.z)
			rr.append(PP.r)
		zz = array(zz)
		rr = array(rr)
		zsmooth = ndimage.gaussian_filter(zz,sigma=1)
		rsmooth = ndimage.gaussian_filter(rr,sigma=1)
		for ii in range(len(br)):
			br[ii].z = zsmooth[ii]
			br[ii].r = rsmooth[ii]	
		
	# save the swc file.		
	saveSWC(filenameCommon+'.swc',branches)
	scaleSWC(filenameCommon,mag	)

#def backupCodes():
	#LinkedPointsAll = getLinkedPointsFromBranches(branches)
	#PointsAll, IDsAll = getCoordinates(LinkedPointsAll)
	#indEnds = []
	#for ii in range(len(LinkedPointsAll)):
	#	PP = LinkedPointsAll[ii]
	#	if len(PP.conn) == 1:
	#		indEnds.append(ii)
	#for ii in range(len(indEnds)):
	#	i1 = indEnds[ii]
	#	if len(LinkedPointsAll[i1].conn) > 1:
	#		continue
	#	for jj in range(ii+1,len(indEnds)):
	#		j1 = indEnds[jj]
	#		if len(LinkedPointsAll[j1].conn) > 1:
	#			continue
	#		dist = distanceTwoPoints(PointsAll[i1,:],PointsAll[j1,:])
	#		if dist < maxDistConnect:
	#			LinkedPointsAll[i1].addConn(IDsAll[j1])
	#			LinkedPointsAll[j1].addConn(IDsAll[i1])
	#branches, branchConnIDs = createBranchesFromLinkedPoints(LinkedPointsAll) 
