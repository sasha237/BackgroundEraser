#include "ImageParser.h"
//simple fitter - gives info, that "white" is color, in which r,g and blue always > WHITE_FITTER
#define WHITE_FITTER 255
//initialising
ImageParser::ImageParser(string sInPath, int iCount):
m_img(sInPath.c_str())
{
	//we are creating linear arrays, because it can give us more speed in calculations, and creating cashes for it
	m_sInPath = sInPath;
	m_iCount = iCount;
 	m_Clusters = new int[m_iCount*3];
	m_iWidth = m_img.width();
	m_iHeight = m_img.height();
	m_iTotalBits = m_iWidth*m_iHeight*3;
 	m_Map = new int[m_iWidth*m_iHeight];
	m_iBufferColors = new int[m_iWidth*m_iHeight*3];
	int iPix = 0;
	for (int i=0;i<m_iWidth;i++)
	{
		for (int j=0;j<m_iHeight;j++)
		{
			QRgb p = m_img.pixel(i,j);
			int r = qRed(p);
			int g = qGreen(p);
			int b = qBlue(p);
			//checking if color is "white" or "semi-white"
			if (r>WHITE_FITTER&&g>WHITE_FITTER&b>WHITE_FITTER)
				r=g=b=255;
			m_iBufferColors[iPix++] = r;
			m_iBufferColors[iPix++] = g;
			m_iBufferColors[iPix++] = b;
		}
	}

	m_dSummRGB = new double[m_iCount*3];
	m_dSum = new double[m_iCount];
	memset(m_Map,0,sizeof(int)*m_iWidth*m_iHeight);

	InitClusters();
}

ImageParser::~ImageParser(void)
{
	delete[] m_Clusters;
	delete[] m_Map;
	delete[] m_iBufferColors;
	delete[] m_dSummRGB;
	delete[] m_dSum;
}

				
void ImageParser::SaveToFile()
{
	int ipos = m_sInPath.find(".");
	m_sInPath = m_sInPath.insert(ipos,"_new");
	m_img.save(m_sInPath.c_str());
}
void ImageParser::InitClusters()
{
	srand( (unsigned)time( NULL ) );
	
	double dSumm = 0;
	m_dR = m_dG = m_dB = 0;
	int i;
	int j;
	int k;
	int iSubCount = m_iCount*3;
	//random initialization of kernel for k-means
	for (i = 0; i < iSubCount;i++ )
		m_Clusters[i] = rand()%256;
	//calculation total sum of red, green and blue color layers of all picture
	//sometimes it can be bad initialized - it is not bug, it is feature of algorithm =), really, so it generates exception and reruns
	//probably we can use no random, but special magical numbers, 
	//for example we divide color scale into some pieces and use them for initial values
	for (j = 0; j < m_iTotalBits; j+=3)
	{
		m_dR += m_iBufferColors[j]/255.;
		m_dG += m_iBufferColors[j+1]/255.;
		m_dB += m_iBufferColors[j+2]/255.;
	}
	//calculation of norm for each color layer
	dSumm = m_dR + m_dG + m_dB;
	m_dR /= dSumm;
	m_dG /= dSumm;
	m_dB /= dSumm;

}

//simple Manhattan distance
double ImageParser::Distance1(int* c1, int* c2)
{
	int dr = abs(*(c1) - *(c2));
	int dg = abs(*(c1+1) - *(c2+1));
	int db = abs(*(c1+2) - *(c2+2));
	return (double)mymax(mymax(dr, dg), db);
}
//normed Manhattan distance
double ImageParser::Distance1Scale(int* c1, int* c2)
{
	double dr = abs(*(c1) - *(c2)) / m_dR;
	double dg = abs(*(c1+1) - *(c2+1)) / m_dG;
	double db = abs(*(c1+2) - *(c2+2)) / m_dB;
	return (double)mymax(mymax(dr, dg), db);
}
//simple Euclidean distance
double ImageParser::Distance2(int* c1, int* c2)
{
	return sqrt(pow((*(c1) - *(c2)), 2.0) + pow((*(c1+1) - *(c2+1)), 2.0) + pow((*(c1+2) - *(c2+2)), 2.0));
}
//normed Euclidean distance
double ImageParser::Distance2Scale(int* c1, int* c2)
{
	return sqrt(pow((*(c1) - *(c2)) / m_dR, 2.0) + pow((*(c1+1) - *(c2+1)) / m_dG, 2.0) + pow((*(c1+2) - *(c2+2)) / m_dB, 2.0));
}

//testfunc for detecting, if save works correct
void ImageParser::Calc1()
{
	QImage newBmp = m_img.copy();
	int ipos = m_sInPath.rfind("\\");
	m_sInPath = m_sInPath.insert(ipos + 1, "new_");
	newBmp.save(m_sInPath.c_str());

}
//debugfunc for save each step of our program
void ImageParser::Calc2()
{
	for (int i = 0; i < 1000; i++)
	{
		cout<<i<<endl;
		//one iteration, if successful, then we need one more operation
		bool bFlag = !CalcStep();
		//createing bufferimage
		QImage bmp(QSize(m_iWidth,m_iHeight),m_img.format());

		//filling buffer image with modified bits
		for (int j = 0; j < m_iWidth; j++)
		{
			for (int k = 0; k < m_iHeight; k++)
			{
				int iInd = m_Map[j*m_iHeight+k];
				bmp.setPixel(j,k,qRgb(m_Clusters[iInd*3],m_Clusters[iInd*3+1],m_Clusters[iInd*3+2]));
			}
		}

		//saving buffer image
		int ipos = m_sInPath.rfind("\\");
		
		string sInPath;
		string sBufPath;
		char buffer[33];
		sInPath = "new_";
		sInPath+=itoa(i,buffer,10);
		sInPath+="_";
		sBufPath = m_sInPath;
		sBufPath.insert(ipos + 1, sInPath);
		bmp.save(sBufPath.c_str());
		if (bFlag)
			break;
	}
	//after all clearing background
	EraseFirstCluster();
	//and saving final image
	SaveToFile();
}
//main calc function
void ImageParser::Calc()
{
	for (int i=0; i < 1000; i++)
	{
		cout<<i<<endl;
		//calc each step
		if (!CalcStep())
			break;
	}
	//after all clearing background
	EraseFirstCluster();
	//and saving final image
	SaveToFile();
}
void ImageParser::EraseFirstCluster()
{
	//max possible value of double
	double dMax = numeric_limits<double>::min();
	int iInd = -1;
	//detection biggest Cluster
	for (int i = 0; i < m_iCount;i++ )
	{
		//if not "white" and "semi-white"
		if (dMax < m_dSum[i] && m_Clusters[i] < WHITE_FITTER && m_Clusters[i+1] < WHITE_FITTER && m_Clusters[i+2] < WHITE_FITTER)
		{
			dMax = m_dSum[i];
			iInd = i;
		}
	}
	//erasing biggest Cluster by replacing its bits by white
	for (int j = 0; j < m_iWidth; j++)
	{
		for (int k = 0; k < m_iHeight; k++)
		{
			if (m_Map[j*m_iHeight+k] != iInd)
				continue;
			m_img.setPixel(j, k, qRgb(255,255,255));
		}
	}
}
bool ImageParser::CalcStep()
{
	//initializing
	bool bChange = false;
	double dMax = numeric_limits<double>::max();
	memset(m_dSummRGB,0,sizeof(double)*m_iCount*3);
	memset(m_dSum,0,sizeof(double)*m_iCount);
	int i=0;
	int j;
	int iInd;
	double dDiff;
	int iSubCount = m_iCount*3;
	double dBufDiff;
	//going through all bits
	while(i<m_iTotalBits)
	{
		iInd = -1;
		dDiff = dMax;
		for(j=0;j<iSubCount;j+=3)
		{
			if(m_Clusters[j]>255)
				continue;
			//calculating distance between each bit and each cluster's kernel and finding closest
			dBufDiff = Distance2Scale(m_iBufferColors+i,m_Clusters+j);
			if (dDiff > dBufDiff)
			{
				dDiff = dBufDiff;
				iInd = j/3;
			}
		}
		if (iInd==-1)
		{
			continue;
			i+=3;
		}
		//associating bit with Cluster if it changed
		if (m_Map[i/3]!=iInd)
		{
			bChange = true;
			m_Map[i/3] = iInd;
		}
		//calculating summ weight of each cluster on this iteration
		m_dSummRGB[iInd*3] += (double)(m_iBufferColors[i])/255.0;
		m_dSummRGB[iInd*3+1] += (double)(m_iBufferColors[i+1])/255.0;
		m_dSummRGB[iInd*3+2] += (double)(m_iBufferColors[i+2])/255.0;
		m_dSum[iInd]++;
		i+=3;
	}
	//of we have no bits, which changes its cluster, when we stops iterations
	if (!bChange)
	{
		return false;
	}
	// calculating new cluster's kernel color coordinates
	// if we have cluster with no element then it will be eliminated
	for (i = 0; i < iSubCount; i++)
	{
		if(m_dSum[i/3]>0)
			m_Clusters[i] = m_dSummRGB[i] / m_dSum[i/3]*255.0;
		else
			m_Clusters[i] = 1000;
	}

	return true;
}
		
				
		