#include <fstream>
#include <iostream>
#include <getopt.h>
#include <string>
#include <cmath>
#include <iomanip>

double calculateDistance( double x1, double y1, double z1, double x2, double y2, double z2 )
{
    return sqrt( pow( x1-x2, 2 ) + pow( y1-y2, 2 ) + pow( z1-z2, 2 ) );
}

int main( int argc, char* argv[] )
{
    char* inFileName; 
//    char* outFileName;
    std::fstream inFile, outFile;
    int c = -1;
    c = getopt( argc, argv, "f:" );
    switch( c ){
    case 'f':
        inFileName = optarg;
	break;
    default:
	std::cout << "Missing file name" << std::endl;
    }
    inFile.open( inFileName, std::ios::in );

    if( !inFile.is_open() )
	std::cout << "Incorrect file name!" << std::endl;

    std::string outFileName(inFileName);
    outFileName = std::string("out_")+outFileName;
    outFile.open( outFileName.c_str(), std::ios::out );

    if( outFile.is_open() )
	std::cout << "Creating output file: \"" << outFileName << "\"..." << std::endl;

    double clight = 299792458; // [m/s]
    double t0, sx, sy, sz, x, y, z, distance;
    long double t;
    inFile >> t0; 
    outFile << t0 << "\n";
   
    inFile >> sx >> sy >> sz;
    
    while( inFile >> x >> y >> z )
    {
        distance = calculateDistance( sx, sy, sz, x, y, z );
	t = t0 + distance/clight;
	outFile << x << " " << y << " " << z << " " << std::setprecision(15) << t << "\n";    
    }
    inFile.close();
    outFile.close();
    std::cout << "DONE!" << std::endl;
    return 0;
}
