#include <QtCore/QCoreApplication>
#include <sys/types.h>  // For stat().
#include <sys/stat.h>   // For stat().
#include <QDir>
#include <omp.h>

#include "ImageParser.h"

#define PrintError {printf("Need 3 params: regime (file/folder), count of clasters and path to file. For example: file 4 \"ololo.png\"\n");\
	return EXIT_SUCCESS;}


void MakeAnalysis(const char* sPath, int iCount);

int main(int argc, char *argv[])
{

	//checking some stuff
	if(argc!=4)
		PrintError;
	int iCount = atoi(argv[2]);
	if (!iCount)
		PrintError;
	string sRegime = argv[1];
	string sFrom = argv[3];
	char* buf = (char*)sRegime.c_str();
	sRegime = strlwr(buf);
	if (sRegime!="file"&&sRegime!="folder")
		PrintError;

	if (-1==access(sFrom.c_str(),0))
		PrintError;

	struct stat mystatus;
	::stat( sFrom.c_str(), &mystatus );

	if (sRegime=="file" && mystatus.st_mode & S_IFDIR||sRegime=="folder"&&!(mystatus.st_mode & S_IFDIR))
		PrintError;

	//simple file
	if (sRegime == "file")
	{
		MakeAnalysis(sFrom.c_str(),iCount);
	}
	//or directory and files in it
	else
	{
		
		omp_set_dynamic(0);      // deny to openmp to change count of threads while working
		omp_set_num_threads(10); // set count of thread equal 10

		sFrom+="\\";
		QDir dir(sFrom.c_str(),"*.*",QDir::Size | QDir::Reversed,QDir::Files | QDir::Hidden | QDir::NoSymLinks);
		QFileInfoList list = dir.entryInfoList();
		int i;
		#pragma omp parallel shared(list) private(i)		//declare arguments
		{
			#pragma omp for									//declare parallel cycle
			for ( i = 0; i < list.size(); ++i)  
			{
				QFileInfo fileInfo = list.at(i);
				string sPath = fileInfo.filePath().toStdString();
				MakeAnalysis(sPath.c_str(),iCount);
				cout<<sPath<<endl;
			}
		}
	}
	
	return EXIT_SUCCESS;
}

//working with files and analysing them
void MakeAnalysis(const char* sPath, int iCount)
{
	int i = 10;
	while (i-- > 0)
	{
		try
		{
			ImageParser prs(sPath, iCount);
			prs.Calc();
			break;
		}
		catch (exception& e)
		{
			cout<<e.what()<<endl<<"Trying Again"<<endl;
		}
	}
}