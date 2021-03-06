#!/usr/bin/env python

import csv
import sys
import numpy as np
import scipy.stats
#import matplotlib.pyplot as plt

dimensions=[]

def readfile(filename):
	''' read file with coordinates of found spots and 
	convert it into a numpy-array'''
	global dimensions
	try:	# try to load cached numpy file format (faster)
		npzfilename = str(filename)
		if npzfilename[-4:] != '.npz':
			npzfilename += '.npz'
		data = np.load(npzfilename)
		dims = data['dims']
		dimensions = dims
		coords = data['coords']
		print "I read the cached file instead of parsing a .txt file"
		return dims, coords
	except IOError: # file could not be read
		pass

	# read csv coordinates list
	csvfile = open(filename, "rb")
	dialect = csv.Sniffer().sniff(csvfile.read(4096),delimiters=",; ")
	print "delim: '%s'" % dialect.delimiter
	csvfile.seek(0)
	reader = csv.reader(csvfile, dialect)
	
	# read dimensions
	dimline = reader.next()
	dimline = [float(i) for i in dimline] #convert to float
	coords = []
	try:
		for row in reader:
			coords.append(row)
	except csv.Error, e:
		sys.exit('file %s, line %d: %s' % (filename, reader.line_num, e))

	npcoords = np.asarray(coords, dtype='float') #numpy-array
	npdims = np.asarray(dimline[0:3])
	dimensions = npdims
	np.savez(str(filename), dims=npdims, coords=npcoords)
	return npdims, npcoords


def writefile(filename, shape, coords):
	'''write an array of coordinates found in an image of dimensions 
	shape back to disk.'''
	
	fp = open(filename, 'wb')
	csvwriter = csv.writer(fp, delimiter=' ', lineterminator='\n')
	shape2 = [int(shape[0]), int(shape[1])] # map tuple to array of ints
	shape2.append(int(np.max(coords[:,2]))+1) # number of frames
	coordsVisible = cropROI(coords, (0, shape2[0]-1, 0, shape2[1]-1)) # drop points outside visible area
	if len(coordsVisible) != len(coords):
		print "%i out of %i coordinates are outside of the field of view (dropping)" % \
				(len(coords)-len(coordsVisible), len(coords))
	csvwriter.writerow(shape2)
	csvwriter.writerows(coordsVisible)
	fp.close()


def cropROI(coords, roi):
	'''select only spots inside the roi=[xmin, xmax, ymin, ymax]'''

	xmin, xmax, ymin, ymax = roi
	idxs = (coords[:,0] >= xmin) & (coords[:,0] <  xmax) & (coords[:,1] >= ymin) & (coords[:,1] <  ymax)

	return coords[idxs,:]

def countDetections(cc,x,y,radius):
	roi = x-radius, x+radius, y-radius, y+radius
	cc = cropROI(cc, roi)
	number = 0
	intensity = 0.
	center = np.array([x,y])
	idxs = (np.sum((cc[:,0:2]-center)**2,axis=1)) < radius**2
	if(idxs!=[]):
		cc = cc[idxs]
		return len(cc), np.sum(cc[:,3])
	else: # empty set
		return 0, 0.

def coords2Image(dimension, coords, factor=8):
	im = np.zeros((dimension[1]*factor, dimension[0]*factor))
	cc = np.array(coords[:,:2]*factor, dtype=int) # integer indices
	for i in xrange(len(coords)):
		intensity = coords[i,3]
		im[cc[i,1],cc[i,0]] += intensity
	
	#limit maximum value
	mmx = scipy.stats.scoreatpercentile(im[np.where(im>0)].flat, 95.0)
	if mmx > 0:
		im[im>mmx] = mmx # crop maximum at above percentile
	print np.max(im)
	print mmx
	print np.mean(im)
	return im

def Image2coords(image, color):
	if color == (1,0,0):
		index = 2
	
	if color == (0,1,0):
		index = 1
		
	maxInt = np.max(image)
	retMatrix = [[image.shape[0],image.shape[1],1]]
	imagecol = np.zeros((image.shape[0],image.shape[1],4),dtype=np.uint8)
	
	if len(image.shape)==2:
		for i in range(image.shape[0]):
			for j in range(image.shape[1]):
				if image[i,j] != 0:
					retMatrix.append([i,j,0,image[i,j],1])
					imagecol[i,j,index] = int(image[i,j] * 255/float(maxInt))
	
	if len(image.shape)==3:
		for i in range(image.shape[0]):
			for j in range(image.shape[1]):
				if image[i,j,index] != 0:
					retMatrix.append([i,j,0,image[i,j,index],1])
					imagecol[i,j,index] = int(image[i,j,index] * 255/float(maxInt))
	
	
	
	return imagecol, retMatrix, retMatrix[0]
				
				

if __name__ == "__main__":
	'''read image coordinates, construct images
	save output as .png or .tiff ...'''
	if len(sys.argv) < 3:
		print ( "Usage: " + sys.argv[0] + "coordsfile.txt image.png")
		sys.exit(1)
	import matplotlib.pyplot as plt
	dims, coords = readfile(sys.argv[1])
	im = coords2Image(dims, coords, factor=10)
	plt.gray()
	plt.imsave(sys.argv[2], im)
	
"""def ImgCord2CenteredCord(x,y):
	x = x - dimensions[0]/2.
	y = y - dimensions[0]/2.
	returnArray=np.asarray([x,y])
	return returnArray.T

def CenteredCord2ImgCord(x,y):
	x = x + dimensions[0]/2.
	y = y + dimensions[0]/2.
	returnArray=np.asarray([x,y])
	return returnArray.T"""