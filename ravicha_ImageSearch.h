#include <iostream>
#include <algorithm>
#include <string>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <iterator>
#include "PNG.h"
#include <omp.h>

using namespace std;

class ImageSearch {
	
	public:
	
	ImageSearch(); //default constructor

	ImageSearch( char **ImageDetails, int percentageMatch = 75, int colorTolerance = 32) : bigImageFile (*( ImageDetails+ 1)), smallImageFile (*( ImageDetails+ 2)), outputImageFile (*( ImageDetails+ 3)), mask ( (string)"true" == *(ImageDetails + 4) ? true : false), percentagePixelMatch (percentageMatch), pixelColorTolerance(colorTolerance) {}
	
	void outputImageGeneration();
	
	void display() const;
						  
	void ImageMatcher();

	bool matchFound(int x,int y) const;
	
	bool isMatchFound(int i, int j) const;
	
	inline bool isPixelMatching(int x, int y, int z, int w) const;
	
	inline int getPixelIndex(int x, int y, int width) const;
	
	void redBoxes(int x,int y);
	
	void maskInvoke();
	
	void maskMatch();
	
	bool isBlack(int x, int y);
	
	std::vector<unsigned char> computeAverage(int x, int y);
	
	std::vector<unsigned char> blackMatchFound(const std::vector<unsigned char>& largeImageBuffer, int bigIndex, const std::vector<unsigned char>& subImageBuffer, int smallIndex) const;

	bool maskImageFound(int x,int y, std::vector<unsigned char> averageColor);
	
	bool isMaskedPixelMatching(int x, int y, int z, int w, std::vector<unsigned char> averageColor);
	
	std::vector< unsigned char> smallImageRotate90( std::vector< unsigned char > smallImageBuffer );
	
	std::vector< unsigned char> smallImageRotate180( std::vector< unsigned char > smallImageBuffer );
	
	std::vector< unsigned char> smallImageRotate270( std::vector< unsigned char > smallImageBuffer );
	
	bool isMaskedPixelMatchRotate90(int x, int y, int z, int w, std::vector<unsigned char> averageColor);

	bool isMaskedPixelMatchRotate180(int x, int y, int z, int w, std::vector<unsigned char> averageColor);
	
	bool isMaskedPixelMatchRotate270(int x, int y, int z, int w, std::vector<unsigned char> averageColor);	
	
	//void maskDisplay() const;
	
	
	
	
	private:
	
	int count;
	int bigImageHeight;
	int bigImageWidth;
	
	//int width = bigImageWidth - smallImageWidth;
	//int height = bigImageHeight - smallImageHeight;
	
	int smallImageHeight;
	int smallImageWidth;

	std::string bigImageFile;
	std::string smallImageFile;
	std::string outputImageFile;
	
	bool mask;
	//bool matchFound;
	
	int percentagePixelMatch;
	int pixelColorTolerance;
	
	PNG bigImage;
	PNG smallImage;
	PNG outputImg;
	
	std::vector<unsigned char> bigImageBuffer;
	std::vector<unsigned char> smallImageBuffer;
	//std::vector<unsigned char> temporaryImageBuffer;
	//std::vector<unsigned char> averageColor;
	
	std::vector<int> matcher;
	std::vector<int> maskMatcher;
};
