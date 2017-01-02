#include "ravicha_ImageSearch.h"

using namespace std;


//function that tries to find a match
void ImageSearch::ImageMatcher() {

	bigImage.load(bigImageFile);
	smallImage.load(smallImageFile);
	//outputImg.load(outputImageFile);
	
	smallImageHeight = smallImage.getHeight();
	smallImageWidth = smallImage.getWidth();
	
	bigImageHeight = bigImage.getHeight();
	bigImageWidth = bigImage.getWidth();
	
	int width = bigImageWidth - smallImageWidth;
	int height = bigImageHeight - smallImageHeight;
	
	//cout << height << width;
	
	bigImageBuffer = bigImage.getBuffer(); //gets a reference to the flat image buffer.
	smallImageBuffer = smallImage.getBuffer(); //gets a reference to the flat image buffer.
		
		
#pragma omp parallel default(shared) 
	{
	#pragma omp for ordered
	for( int i = 0; i < height; i++) { //till the small Image reaches the last row
		for( int j =0; j < width; j++) { //till the small Image reaches the last column
			if ( isMatchFound ( i , j + smallImageWidth-1) || isMatchFound( i, j) ) {
				continue;
			}
			if( matchFound(i,j)) {
				//cout << "true \t";
				matcher.push_back(i);
				matcher.push_back(j);
			}
		}
	}
	}
	//outputImageGeneration();
	//display();
}

//function that calculates if there is a match
bool ImageSearch::matchFound(int x,int y) const{
	int counter = 0;
	int comp =  smallImageHeight * smallImageWidth * percentagePixelMatch /100;
	//cout << "pixel" << percentagePixelMatch << endl;
	//cout << "comp" << comp << endl;
	for(int i = 0; i < smallImageHeight;i++) {
		for(int j = 0; j < smallImageWidth; j++) {
			if(isPixelMatching(i,j,x+i,y+j)) {
				counter++;
				if(counter == comp)
					return true;
			}
		}
	}	
	return false;
}	

bool ImageSearch::isMatchFound(int i, int j) const { 
	if(matcher.size() == 0) {
		return false;
	}
	for(size_t k = 0; k < matcher.size() ; k += 2) {
		if( (i >= matcher[k]) && (j >= matcher[k+1]) && i <=(matcher[k] + smallImageHeight) && j <= (matcher[k+1] + smallImageWidth) ) {
			return true;
		}
	}
	return false;
}
	

inline bool ImageSearch::isPixelMatching(int x, int y, int z, int w) const {
		const int smallIndex = getPixelIndex(x,y,smallImageWidth);
		const int bigIndex = getPixelIndex(z,w,bigImageWidth);
	  if( abs(bigImageBuffer[bigIndex] - smallImageBuffer[smallIndex]) <= pixelColorTolerance &&
			abs(bigImageBuffer[bigIndex+1] - smallImageBuffer[smallIndex+1]) <= pixelColorTolerance &&
			abs(bigImageBuffer[bigIndex+2] - smallImageBuffer[smallIndex+2]) <= pixelColorTolerance &&
			abs(bigImageBuffer[bigIndex+3] - smallImageBuffer[smallIndex+3]) <= pixelColorTolerance )
		return true;
	  else
		return false;
	}
	

inline int ImageSearch::getPixelIndex(int x, int y, int width) const {	return ( (x*width + y )*4 ); } //In a 2D array, pick an element, check its position and use the formula to caluculate its position in the buffer.

//Writes the output image
void ImageSearch::outputImageGeneration() {
	
	outputImg = bigImage;
	
#pragma omp parallel default(shared) 
{
	#pragma omp for
	for(size_t i = 0; i < matcher.size(); i+=2) {
		for( int j = matcher[i+1]; j < matcher[i+1] + smallImageHeight; j++) {
			if( (j == matcher[i+1]) ||(j == matcher[i+1] + smallImageHeight-1)) {
				for(int k =0; k < smallImageWidth+1; k++) {
					redBoxes(matcher[i] + k, j); //draw a red line along the height in column number 0 and column number last
				}
			}
			else {
				redBoxes(matcher[i], j); //draws a red line in each column of the first row.
				redBoxes(matcher[i]+smallImageWidth, j); //draws a red line in each column of the last row.
			}
		}
	}
}
outputImg.write(outputImageFile);
}

//function that draws the redboxes in the output file
void ImageSearch::redBoxes(int x,int y) {
	std::vector<unsigned char>& outputBuffer = outputImg.getBuffer();	
	int index = getPixelIndex(x, y, bigImageWidth);
	outputBuffer[index] = 255;
	outputBuffer[index+1] = 0;
	outputBuffer[index+2] = 0;
	//outputBuffer[index+3] = 255;
}

//function that displays the number of matches and position of matches				
void ImageSearch::display() const {
	for(size_t i = 0; i < matcher.size(); i+=2) {
		std::cout << "sub-image matched at:" << matcher[i] + 1 <<", " << matcher[i+1]+1 << ", " << matcher[i]+smallImageHeight + 1 << ", " << matcher[i+1] + smallImageWidth + 1 << std::endl; //display the 4 dimensions of the positions matching	
	}
	std::cout << "Number of Matches : " << matcher.size()/2 << "times" << std::endl;
	//std::cout << mask << std::endl;
}


//homework 9 starts here

//THis function decides if a normal or  masked Image search take place
void ImageSearch::maskInvoke(){
	if(mask == true) {
		maskMatch();
	}
	else {
		ImageMatcher();
	}	
}	


void ImageSearch::maskMatch() {
	
	bigImage.load(bigImageFile);
	smallImage.load(smallImageFile);
	//outputImg.load(outputImageFile);
	
	smallImageHeight = smallImage.getHeight();
	smallImageWidth = smallImage.getWidth();
	
	bigImageHeight = bigImage.getHeight();
	bigImageWidth = bigImage.getWidth();
	
	bigImageBuffer = bigImage.getBuffer(); //gets a reference to the flat image buffer.
	smallImageBuffer = smallImage.getBuffer();
		
	int width = bigImageWidth - smallImageWidth;
	int height = bigImageHeight - smallImageHeight;
	
	//cout << " height " << height << endl;
	//	cout << "width" << width << endl;
		
	std::vector<unsigned char> averageColor;
	
	for(int i = 0; i < height; i++) {
		for(int j = 0; j < width; j++) {
			averageColor = computeAverage(i,j);
			if(isMatchFound(i, j + (smallImageWidth - 1)) || isMatchFound( i, j) ) {
				continue;
			}
			
			
			//cout << averageColor;
			if(maskImageFound(i,j, averageColor)) {
				matcher.push_back(i);
				matcher.push_back(j);
			//	matcher.push_back(i+smallImageWidth);
			//	matcher.push_back(j+smallImageHeight);
			}
		}
	}
//outputImageGeneration();
//maskDisplay();
}	

//checks if the given input pixels are black
bool ImageSearch::isBlack(int x, int y) {
	const int smallIndex = getPixelIndex( x, y, smallImageWidth);
		if( smallImageBuffer[smallIndex] == 0 && smallImageBuffer[smallIndex + 1] == 0 && smallImageBuffer[smallIndex + 2] == 0 && smallImageBuffer[smallIndex + 3] == 255) {
			return true;
		}
	return false;
}

std::vector<unsigned char> ImageSearch::computeAverage(int x, int y) {

	long r = 0 , b = 0, g = 0, a = 0;
	std::vector<unsigned char> averageBackground;
	std::vector<unsigned char> temp;
	const int smallIndex = getPixelIndex(x, y ,smallImageWidth);
	int counter = 0;
	
	for(int i = 0; i < smallImageHeight; i++) {
		for(int j = 0; j < smallImageWidth; j++) {
			const int bigIndex = getPixelIndex(x+i, y+j, bigImageWidth);
			temp = blackMatchFound(bigImageBuffer, bigIndex, smallImageBuffer, smallIndex);
			
			r += (int)temp[0];
			g += (int)temp[1];
			b += (int)temp[2];
			a += (int)temp[3];			
			counter += (int)temp[4];
			
		}
	}
	
	if(isBlack(x,y)) {
	r /= counter;
	g /= counter;
	b /= counter;
	a /= counter;
	}
		
	averageBackground.push_back((unsigned char)r);
	averageBackground.push_back((unsigned char)g);
	averageBackground.push_back((unsigned char)b);
	averageBackground.push_back((unsigned char)a);
	
	return averageBackground;
}
	
std::vector<unsigned char> ImageSearch::blackMatchFound(const std::vector<unsigned char>& largeImageBuffer, int bigIndex, const std::vector<unsigned char>& smallImageBuffer, int smallIndex) const  {
	  std::vector<unsigned char> blackMatch( 4 );
	  if( smallImageBuffer[smallIndex] == 0 && smallImageBuffer[smallIndex + 1] == 0 && smallImageBuffer[smallIndex + 2] == 0 && smallImageBuffer[smallIndex + 3] == 255) {
	  	blackMatch[0] = bigImageBuffer[bigIndex];
	  	blackMatch[1] = bigImageBuffer[bigIndex+1];
	  	blackMatch[2] = bigImageBuffer[bigIndex+2];
	  	blackMatch[3] = bigImageBuffer[bigIndex+3];
		blackMatch[4] = 1;	  
	  }
	  return blackMatch;
}

bool ImageSearch::maskImageFound(int x,int y, std::vector<unsigned char> averageColor) {
	int counter = 0;
	int comp =  smallImageHeight * smallImageWidth * percentagePixelMatch /100;
	for(int i = 0; i < smallImageHeight;i++) {
		for(int j = 0; j < smallImageWidth; j++) {
			#pragma omp critical
			{
			if(isMaskedPixelMatching(i,j,x+i,y+j, averageColor) || isMaskedPixelMatchRotate90(i,j,x+i,y+j, averageColor) || isMaskedPixelMatchRotate180(i,j,x+i,y+j, averageColor)||  isMaskedPixelMatchRotate270(i,j,x+i,y+j, averageColor)) {
					counter++;
			}
			}
			if(counter >= comp)
					return true;
		}
	}	
	return false;
}

bool ImageSearch::isMaskedPixelMatching(int x, int y, int z, int w, std::vector<unsigned char> averageColor) {
	const int smallIndex = getPixelIndex(x,y,smallImageWidth);
	const int bigIndex = getPixelIndex(z,w,bigImageWidth);
	if( smallImageBuffer[smallIndex] == 0 && smallImageBuffer[smallIndex + 1] == 0 && smallImageBuffer[smallIndex + 2] == 0 && smallImageBuffer[smallIndex + 3] == 255) {	
		if( abs(bigImageBuffer[bigIndex] - averageColor[0]) <= pixelColorTolerance &&
			abs(bigImageBuffer[bigIndex+1] - averageColor[1]) <= pixelColorTolerance &&
			abs(bigImageBuffer[bigIndex+2] - averageColor[2]) <= pixelColorTolerance &&
			abs(bigImageBuffer[bigIndex+3] - averageColor[3]) <= pixelColorTolerance )
			return true;
		else
			return false;
	}
	else {
		if( abs(bigImageBuffer[bigIndex] - averageColor[0]) > pixelColorTolerance ||
			abs(bigImageBuffer[bigIndex+1] - averageColor[1]) > pixelColorTolerance ||
			abs(bigImageBuffer[bigIndex+2] - averageColor[2]) > pixelColorTolerance ||
			abs(bigImageBuffer[bigIndex+3] - averageColor[3]) > pixelColorTolerance ) {
				return true;
		}
		else {
				return false;
		}
	}	
	//return false;
}
	
//rotation

std::vector< unsigned char> ImageSearch::smallImageRotate180( std::vector< unsigned char > smallImageBuffer ){
    
    std::reverse( smallImageBuffer.begin(), smallImageBuffer.end() );
    
    return smallImageBuffer;
}

std::vector< unsigned char> ImageSearch::smallImageRotate270( std::vector< unsigned char > smallImageBuffer ){

    int rot = 0;
    unsigned char smallImageMatrix[ smallImageHeight ][ smallImageWidth * 4 ];
    unsigned char transposeSmallImageMatrix[ smallImageWidth * 4 ][ smallImageHeight ];
    
    rot = 0;
    for( int i = 0; i < smallImageHeight; i++ ){
        for( int j = 0; j < smallImageWidth * 4; j++ ){
            smallImageMatrix[i][j] = smallImageBuffer[rot];
            rot++;
        }
    }
    
    for( int i = 0; i < smallImageHeight; i++ ){
        for( int j = 0; j < smallImageWidth * 4; j++ ){
            transposeSmallImageMatrix[j][i] = smallImageMatrix[i][j];
        }
    }
    
    rot = 0;
    for ( int i = 0; i < smallImageWidth * 4; i++){
        for( int j = 0; j < smallImageHeight; j++ ){
            smallImageBuffer[ rot ] = transposeSmallImageMatrix[ i ][ j ];
            rot++;
        }
    }

    return smallImageBuffer;
}

std::vector< unsigned char> ImageSearch::smallImageRotate90( std::vector< unsigned char > smallImageBuffer ){
    
    std::vector< unsigned char > temporaryImageBuffer;
    temporaryImageBuffer = smallImageRotate270( smallImageBuffer );
    std::reverse( temporaryImageBuffer.begin(), temporaryImageBuffer.end() );
    
    return temporaryImageBuffer;
}


bool ImageSearch::isMaskedPixelMatchRotate90(int x, int y, int z, int w, std::vector<unsigned char> averageColor) {
	std::vector< unsigned char > temporaryImageBuffer;
	temporaryImageBuffer = smallImageRotate90(smallImageBuffer);
	const int tempIndex = getPixelIndex(x,y,smallImageWidth);
	const int bigIndex = getPixelIndex(z,w,bigImageWidth);
	if( temporaryImageBuffer[tempIndex] == 0 && temporaryImageBuffer[tempIndex + 1] == 0 && temporaryImageBuffer[tempIndex + 2] == 0 && temporaryImageBuffer[tempIndex + 3] == 255) {	
		if( abs(bigImageBuffer[bigIndex] - averageColor[0]) <= pixelColorTolerance &&
			abs(bigImageBuffer[bigIndex+1] - averageColor[1]) <= pixelColorTolerance &&
			abs(bigImageBuffer[bigIndex+2] - averageColor[2]) <= pixelColorTolerance &&
			abs(bigImageBuffer[bigIndex+3] - averageColor[3]) <= pixelColorTolerance )
			return true;
		else
			return false;
	}
	else {
		if( abs(bigImageBuffer[bigIndex] - averageColor[0]) > pixelColorTolerance ||
			abs(bigImageBuffer[bigIndex+1] - averageColor[1]) > pixelColorTolerance ||
			abs(bigImageBuffer[bigIndex+2] - averageColor[2]) > pixelColorTolerance ||
			abs(bigImageBuffer[bigIndex+3] - averageColor[3]) > pixelColorTolerance ) {
				return true;
		}
		else {
				return false;
		}
	}
}	
	
bool ImageSearch::isMaskedPixelMatchRotate180(int x, int y, int z, int w, std::vector<unsigned char> averageColor) {
	std::vector< unsigned char > temporaryImageBuffer;
	temporaryImageBuffer = smallImageRotate180(smallImageBuffer);
	const int tempIndex = getPixelIndex(x,y,smallImageWidth);
	const int bigIndex = getPixelIndex(z,w,bigImageWidth);
	if( temporaryImageBuffer[tempIndex] == 0 && temporaryImageBuffer[tempIndex + 1] == 0 && temporaryImageBuffer[tempIndex + 2] == 0 && temporaryImageBuffer[tempIndex + 3] == 255) {	
		if( abs(bigImageBuffer[bigIndex] - averageColor[0]) <= pixelColorTolerance &&
			abs(bigImageBuffer[bigIndex+1] - averageColor[1]) <= pixelColorTolerance &&
			abs(bigImageBuffer[bigIndex+2] - averageColor[2]) <= pixelColorTolerance &&
			abs(bigImageBuffer[bigIndex+3] - averageColor[3]) <= pixelColorTolerance )
			return true;
		else
			return false;
	}
	else {
		if( abs(bigImageBuffer[bigIndex] - averageColor[0]) > pixelColorTolerance ||
			abs(bigImageBuffer[bigIndex+1] - averageColor[1]) > pixelColorTolerance ||
			abs(bigImageBuffer[bigIndex+2] - averageColor[2]) > pixelColorTolerance ||
			abs(bigImageBuffer[bigIndex+3] - averageColor[3]) > pixelColorTolerance ) {
				return true;
		}
		else {
				return false;
		}
	}
}

bool ImageSearch::isMaskedPixelMatchRotate270(int x, int y, int z, int w, std::vector<unsigned char> averageColor) {
	std::vector< unsigned char > temporaryImageBuffer;
	temporaryImageBuffer = smallImageRotate270(smallImageBuffer);
	const int tempIndex = getPixelIndex(x,y,smallImageWidth);
	const int bigIndex = getPixelIndex(z,w,bigImageWidth);
	if( temporaryImageBuffer[tempIndex] == 0 && temporaryImageBuffer[tempIndex + 1] == 0 && temporaryImageBuffer[tempIndex + 2] == 0 && temporaryImageBuffer[tempIndex + 3] == 255) {	
		if( abs(bigImageBuffer[bigIndex] - averageColor[0]) <= pixelColorTolerance &&
			abs(bigImageBuffer[bigIndex+1] - averageColor[1]) <= pixelColorTolerance &&
			abs(bigImageBuffer[bigIndex+2] - averageColor[2]) <= pixelColorTolerance &&
			abs(bigImageBuffer[bigIndex+3] - averageColor[3]) <= pixelColorTolerance )
			return true;
		else
			return false;
	}
	else {
		if( abs(bigImageBuffer[bigIndex] - averageColor[0]) > pixelColorTolerance ||
			abs(bigImageBuffer[bigIndex+1] - averageColor[1]) > pixelColorTolerance ||
			abs(bigImageBuffer[bigIndex+2] - averageColor[2]) > pixelColorTolerance ||
			abs(bigImageBuffer[bigIndex+3] - averageColor[3]) > pixelColorTolerance ) {
				return true;
		}
		else {
				return false;
		}
	}
}

	


//Main function
int main(int argc, char *argv[]) {
	
	if(/*atoi(argv[4])*/ argc <= 5) {
		ImageSearch img(argv);
		img.maskInvoke();
		//img.ImageMatcher();
		img.outputImageGeneration();
		img.display();
	}
	
	else if(/*atoi(argv[5])*/ argc == 6) {
		ImageSearch img(argv,atoi(argv[5]));
		img.maskInvoke();
		//img.ImageMatcher();
		img.outputImageGeneration();
		img.display();
	}
	
	else {
		ImageSearch img(argv, atoi(argv[5]), atoi(argv[6]));
		img.maskInvoke();
		//img.ImageMatcher();
		img.outputImageGeneration();
		img.display();
		
	}

	return 0;
}