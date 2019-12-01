import math
import glob
import numpy as np
import TiffFile

def open_swc(file_name):
	return SWC(file_name)
	
def open_tif(file_name):
	'''
		return numpy array
		z,y,x
	'''
	return TiffFile.imread(file_name)
	
def save_tif(np_array_like,file_name):
	'''
		save numpy array like to tiff 
		z,y,x
	'''
	TiffFile.imsave(file_name,np_array_like.astype(np.uint8))
	
def save_swc2tif(swc_file_name,tif_file_name,shape=None):
	swc=open_swc(swc_file_name)
	bound_box=swc.get_bound_box()
	width=math.ceil(bound_box[3]-bound_box[0])
	height=math.ceil(bound_box[4]-bound_box[1])
	depth=math.ceil(bound_box[5]-bound_box[2])
	if shape:
	    depth,height,width=shape
	tif=np.zeros(shape=(depth,height,width))
	for node in swc.nodes:
		_sphere(tif,node.x,node.y,node.z,node.r,255)
	for edge in swc.edges:
		if edge[0]!=-1 and edge[1]!=-1:
			a,b=swc.get_node(edge[0]),swc.get_node(edge[1])
			_cone(tif,a.x,a.y,a.z,a.r,b.x,b.y,b.z,b.r,255)
	save_tif(tif,tif_file_name)
	
class SWCNode(object):
	
	def __init__(self,id,type,x,y,z,r,pid):
		self.id=id
		self.type=type
		self.x=x
		self.y=y
		self.z=z
		self.r=r
		self.pid=pid
		
class SWC(object):
	
	def __init__(self,file_name=None):
		self.__nodes={}
		self.__edges=[]
		self.bound_box=[0,0,0,0,0,0]# x0,y0,z0,x1,y1,z1
		if file_name:
			self.open(file_name)
			
	def open(self,file_name):
		with open(file_name) as f:
			for line in f.readlines():
				if line.startswith('#'):
					continue
				id,type,x,y,z,r,pid=map(float,line.split())
				self.__nodes[id]=SWCNode(id,type,x,y,z,r,pid)
				self.__edges.append((id,pid))
				if x<self.bound_box[0]:
					self.bound_box[0]=x
				if x>self.bound_box[3]:
					self.bound_box[3]=x
				if y<self.bound_box[1]:
					self.bound_box[1]=y
				if y>self.bound_box[4]:
					self.bound_box[4]=y
				if z<self.bound_box[2]:
					self.bound_box[2]=z
				if z>self.bound_box[5]:
					self.bound_box[5]=z
					
	def get_node(self,id):
		return self.__nodes[id]
		
	def get_bound_box(self):
		return self.bound_box
		
	@property
	def nodes(self):
		return self.__nodes.values()
	
	@property
	def edges(self):
		return self.__edges
	
def _sphere(stack,x,y,z,r,v=1):
	d,h,w=stack.shape
	for k in range(int(max(0,z-r)),int(min(d,z+r))):
		for j in range(int(max(0,y-r)),int(min(h,y+r))):
			for i in range(int(max(0,x-r)),int(min(w,x+r))):
				if (k-z)**2+(j-y)**2+(i-x)**2<=r**2:
					stack[k,j,i]=v
	
def _cone(stack,x0,y0,z0,r0,x1,y1,z1,r1,v=1):
	d,h,w=stack.shape
	'''if r0<r1:
		x0,x1=x1,x0
		y0,y1=y1,y0
		z0,z1=z1,z0
		r0,r1=r1,r0'''
	a,b,c=x1-x0,y1-y0,z1-z0
	r=math.sqrt(a**2+b**2+c**2)+1e-6
	tan_theta=(r0-r1)/r
	points=[]
	for k in range(math.ceil(r)):
		rk=tan_theta*(r-k)+r1
		for j in range(-int(rk)-1,int(rk)+1):
			for i in range(-int(rk)-1,int(rk)+1):
				if j**2+i**2<rk**2:
					points.append((i,j,k))
	n1=math.sqrt(b**2+c**2)
	if n1==0:
		for pt in points:
			x=int(pt[2]+x0)
			y=int(pt[1]+y0)
			z=int(pt[0]+z0)
			if x>=0 and x<w and y>=0 and y<h and z>=0 and z<d:
				stack[z,y,x]=v
	else:	
		n2=math.sqrt((b**2+c**2)**2+(a*b)**2+(a*c)**2)
		Tr=np.array(((0,c/n1,-b/n1),(-(b**2+c**2)/n2,a*b/n2,a*c/n2),(a/r,b/r,c/r)))
		Tr=Tr.T
		for pt in points:
			coord=np.matmul(Tr,pt)
			coord=coord+(x0,y0,z0)
			for x in range(math.floor(coord[0]),math.ceil(coord[0])+1):
				for y in range(math.floor(coord[1]),math.ceil(coord[1])+1):
					for z in range(math.floor(coord[2]),math.ceil(coord[2])+1):
						if x>=0 and x<w and y>=0 and y<h and z>=0 and z<d:
							stack[z,y,x]=v
	
if __name__=='__main__':
    for swc_file in glob.glob('./*.swc'):
	    save_swc2tif(swc_file,swc_file+'.tif',(85,2048,2048))
