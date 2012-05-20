#include <io.h>
#include <Qt/qImage.h>
#include <Qt/qrgb.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <limits>
#include <iostream>
#define mymax(a,b) (a)<(b)?(b):(a)

using namespace std;

#pragma once

/************************************************************************/
/* THIS ClusS IS TRYING TO AUTODETECT AND ERASE BACKGROUND IN PICTURES  */
/************************************************************************/
// we are using modification of k-means algorithm for this http://en.wikipedia.org/wiki/K-means_clustering
class ImageParser
{
	//file to open
	string m_sInPath;
	QImage m_img;
	//total count of color bits in image
	int m_iTotalBits;
	//converted bits of image, can be replaced by, because image->getpixelcolor is too slow
	int* m_iBufferColors;
	//sizes of image
	int m_iWidth;
	int m_iHeight;

	//array for Clusters, which contains 3 colors for each Cluster
 	int* m_Clusters;
	//count of kernels for k-means
 	int m_iCount;
	//map of bits, which belongs to Clusters
 	int* m_Map;
	//total summ for norm
	double* m_dSum;
	//norm of color layers, red, green and blue
	double m_dR;
	double m_dG;
	double m_dB;
	//not normed summ of colors for Clusters
	double* m_dSummRGB;

	//save new file
	void SaveToFile();
	//initialisig of Clusters
	void InitClusters();
	//different functions for detecting color distances
	double Distance1(int* c1, int* c2);
	double Distance2(int* c1, int* c2);
	double Distance1Scale(int* c1, int* c2);
	double Distance2Scale(int* c1, int* c2);
	//funtion for clearing most frequent color
	void EraseFirstCluster();
	//one iteration function for k-means
	bool CalcStep();
public:
	//costructor
	ImageParser(string sInPath, int iCount);
	//destructor
	~ImageParser(void);
	//main calc fuction
	void Calc();
	//test function for detecting: "Is programm working and saving images"
	void Calc1();
	//debug calculations which give a picture after each iteration
	void Calc2();
	
};
