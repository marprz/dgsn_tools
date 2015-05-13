
#include <vector>
#include <deque>
#include <iostream>
#include <fstream>
#include <tuple>
#include <algorithm>

#include <marble/MarbleWidget.h>
#include </home/marta/marble/sources/src/lib/marble/geodata/data/GeoDataCoordinates.h>
#include </home/marta/marble/sources/src/lib/marble/geodata/data/GeoDataLineString.h>
#include </home/marta/marble/sources/src/lib/marble/GeoPainter.h>
#include <marble/RenderPlugin.h>
#include <marble/Quaternion.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <QTimeLine>
#include <qmath.h>
#include <QApplication>

#include <cstdio>

using namespace Marble;
using namespace cv;

int currentNb = 0;
double prevTime = 0;
GeoDataCoordinates prevSatCoor;
std::deque< double > listPrevTimes;

class Signal
{
    public:
        Signal( double t ) : mTime(t){}
        Signal( double t, GeoDataCoordinates aCoor ) : mTime(t), mPosition( aCoor ){}
        void addGS( GeoDataCoordinates aPos )
        {
            mGS.push_back( aPos );
        }

        double mTime;
        GeoDataCoordinates mPosition;
        std::vector< GeoDataCoordinates > mGS;
        int nbGS = 0;
};

    QString const mapTheme = "earth/bluemarble/bluemarble.dgml";

    // Enabled plugins. Everything else will be disabled
    QStringList const features = QStringList() << "stars" << "atmosphere";

    // Camera starting point: Position and zoom level
    double const sourceZoomLevel = 2;

    // Camera destination point: Position and zoom level
    double const zoomLevel = 2;
    double const destinationZoomLevel = zoomLevel;

    // Minimum zoom level (in the middle of the animation)
    double const jumpZoomLevel = zoomLevel;
    //double const jumpZoomLevel = 5.5;

    // Frames per second
    int const fps = 60;

    // Target video file name
    std::string const videoFile = "NewTestNames2.avi";

    // Video resolution
    Size frameSize( 1280, 720 );

typedef std::vector< std::tuple< double, GeoDataCoordinates > > SatSignal;
typedef std::tuple< double, GeoDataCoordinates > SingleSignal;

bool compareSignals( const SingleSignal& first, const SingleSignal& second )
{
    double a = std::get<0>( first );
    double b = std::get<0>( second );
    if ( a>b )
        return true;
    else
        return false;
}


class Satellite
{
    public:
    double time( int index ) { return std::get< 0 >( sig.at( index ) ); }

    void push_back( std::tuple< double, GeoDataCoordinates > aSignal )
    {
        sig.push_back( aSignal );
    }

    int size()
    { 
        return sig.size(); 
    }

    bool operator()( const SingleSignal& first, const SingleSignal& second )
    {
        bool ret;
        if( std::get<0>( first ) < std::get<0>( second ) )
            ret = true;
        else 
            ret = false;
        return ret;
    }

    void sort();
    std::vector< SingleSignal > sig;
};

void Satellite::sort()
{
    int n=size();
    std::cout << "n=" << n << std::endl;
    for( int i=1; i<n; ++i )
    {
        double currTime = time(i);
        std::cout << currTime << " ";
        if( currTime < time(i-1) )
        {
            int j=i-1;
            SingleSignal temp = sig.at(i);
            while( j>=0 && currTime<time(j) )
            {
                sig.at(j+1) = sig.at(j);
                --j;
            }
            sig.at(j) = sig.at(i);
        }
    }
    std::cout << "Posortowane: " << std::endl;
    for( int i=0; i<size(); ++i )
    {
        std::cout << i+1 << ": " << time(i) << (std::get<1>( sig.at(i))).latitude() << " " << (std::get<1>(sig.at(i))).longitude() << std::endl;
    }
}

GeoDataCoordinates fromCarthesianToLatLon( const double& x, const double& y, const double& z )
{
    double REarth = 6378410;
    double lat, lon, h, r;
    r = sqrt( x*x + y*y + z*z );
    lat = asin( z/r )*180/3.14;
    lon = atan( y/x )*180/3.14;
    if( x<0 )
    {
        if( y>0 )
            lon += 180;
        else
            lon -= 180;
    }
    else
    {
        if( x==0 )
        {
            if( y>0 )
                lon = 90;
            else
                lon =-90;
        }
    }
    h = r-REarth;
    GeoDataCoordinates ret( lon, lat, h, GeoDataCoordinates::Degree );
    return ret;
}

class SatPathWidget : public MarbleWidget
{
    public:
        virtual void customPaint( GeoPainter* painter );
        void readData();
        std::vector< GeoDataCoordinates > CalcVector;

        Satellite PrecSatellites;
        Satellite CalcSatellites;
        Satellite GS;
        int satNb; 
};

GeoDataCoordinates coordinates;

void SatPathWidget::readData()
{
    std::string fileCalcName("");
    std::string filePrecName("beaconNames.sat");
    std::string fileGSName("namesGS");
    std::fstream filePrec;
    std::fstream fileCalc;
    std::fstream fileGS;
    filePrec.open( filePrecName.c_str(), std::ios::in );
    if( !filePrec.is_open() )
        std::cout << filePrecName << " is not open" << std::endl;

    fileCalc.open( fileCalcName.c_str(), std::ios::in );
    if( !fileCalc.is_open() )
        std::cout << fileCalcName << " is not open" << std::endl;

    fileGS.open( fileGSName.c_str(), std::ios::in );
    if( !fileGS.is_open() )
        std::cout << fileGSName << " is not open" << std::endl;

    int satId;
    double t;
    int i=0, nr = 1; 
    double x, y, z;
    double px = 0, py = 0, pz = 0;
    long double r;
    double lat, lon;
    double temp1, temp2; 
    std::vector< GeoDataCoordinates > GSVector;

// PREC. SAT. POSITIONS:
    while( filePrec >> x >> y >> z >> t >> satId )
    {
        if( satId == 0 )
            PrecSatellites.push_back( std::make_tuple( t, fromCarthesianToLatLon( x, y, z) ) );
    }
    PrecSatellites.sort();

// CALCULATED SAT. POSITIONS:
    while( fileCalc >> satId >> t >> x >> y >> z >> temp1 )
    {
        if( satId == 0 )
        {
            CalcSatellites.push_back( std::make_tuple( t, fromCarthesianToLatLon( x, y, z) ) );
        }
    }

// GROUND STATIONS:
    while( fileGS >> x >> y >> z >> temp1 >> t >> satId >> temp2 )
    {
        if( satId == 0 )
        {
            GS.push_back( std::make_tuple( t, fromCarthesianToLatLon( x, y, z) ) );
        }
    }

    satNb = PrecSatellites.size();
    std::cout << "satNb: " << satNb << std::endl;
}

void SatPathWidget::customPaint( GeoPainter* painter )
{
    int intIt=0;
    painter->setPen(Qt::green);
    std::vector< std::tuple< double, GeoDataCoordinates > >::iterator satIter;
    int counter = 0;
    double currTime;
    GeoDataCoordinates currSatCoor;
    for( ; counter<satNb && intIt<=currentNb ; ++counter )
    {
        GeoDataCoordinates tempCoor = std::get<1>( PrecSatellites.sig.at( counter ) );
        painter->drawEllipse( tempCoor, 3, 3 );
        ++intIt;
        currTime = PrecSatellites.time( counter );
        currSatCoor = tempCoor;
    }

    // CALCULATED SAT
    painter->setPen(Qt::red);
    for( counter = 0; counter<CalcSatellites.size() ; ++counter )
    {
        if( CalcSatellites.time( counter )<=currTime )
        {
            GeoDataCoordinates tempCoor = std::get<1>( CalcSatellites.sig.at( counter ) );
            painter->drawEllipse( tempCoor, 5, 5 );
        }
    }

    painter->setPen(Qt::white);
    for( counter = 0; counter<GS.size() ; ++counter )
    {
        
        double currGStime = GS.time( counter );
        double minTime = intIt>4 ? PrecSatellites.time( intIt-5 ) : PrecSatellites.time( 0 );
        double maxTime = PrecSatellites.time( intIt-1 );

        if( GS.time( counter )<=currTime )
        {
            GeoDataCoordinates tempCoor = std::get<1>( GS.sig.at( counter ) );
            painter->drawEllipse( tempCoor, 2, 2 );
        }

        // ALL LINES TO CURRENT
        if( GS.time( counter )==currTime )
        {
            Marble::GeoDataLineString line;
            painter->setPen(Qt::magenta);
            bool posCalculated = false;
            for( int i=0; i<CalcSatellites.size(); ++i )
            {
                if( CalcSatellites.time( i ) == currTime )
                    painter->setPen(Qt::magenta);
            }
            GeoDataCoordinates tempCoor = std::get<1>( GS.sig.at( counter ) );
            painter->drawEllipse( tempCoor, 4, 4 );
            line << currSatCoor;
            line << tempCoor;
            painter->drawPolyline( line );
            painter->setPen(Qt::white);
        }
    }
    prevTime = currTime;
    prevSatCoor = currSatCoor;
}

void interpolate( SatPathWidget* widget, qreal value, int satNb )
{
    double tempValue = satNb*value;

    int satIndex = floor( tempValue );

    if( satIndex<0 )
        satIndex = 0;
    if( satIndex >= satNb-1 )
        satIndex = satNb-2;
    
    int param = 2;
    currentNb = satIndex;
    satIndex = satIndex/param;
    satIndex *= param;
    if( satIndex>=0 && satIndex<widget->satNb-1 && satNb>1 )
    {
        GeoDataCoordinates source = std::get<1>( widget->PrecSatellites.sig.at(satIndex) );
        GeoDataCoordinates destination = std::get<1>( widget->PrecSatellites.sig.at(satIndex+1) );
 
        qreal lon, lat;
        Quaternion::slerp( source.quaternion(), destination.quaternion(), value ).getSpherical( lon, lat );
        coordinates.setLongitude( lon );
        coordinates.setLatitude( lat );
        
        widget->centerOn( coordinates );
        //widget->setRadius( exp(jumpZoomLevel) + (value < 0.5 ? exp(sourceZoomLevel*(1.0-2*value)) : exp(destinationZoomLevel*(2*value-1.0))) );
    }
//    widget->setRadius( exp(jumpZoomLevel) );//+ exp(destinationZoomLevel*(2*value-1.0)) );
}

void animatedFlight( SatPathWidget *mapWidget )
{
    // Length of the video
    QTimeLine timeLine( 80 * 700 );
    //QTimeLine timeLine( satNb * 1000 );
    
    int satNb = mapWidget->satNb;
    mapWidget->resize( frameSize.width, frameSize.height );
    mapWidget->setDistance( 8000 );
    VideoWriter videoWriter( videoFile, VideoWriter::fourcc('D','I','V','X'), fps, frameSize );
    Mat buffer;
    buffer.create(frameSize, CV_8UC3);
    timeLine.setCurveShape( QTimeLine::LinearCurve );
    //timeLine.setCurveShape( QTimeLine::EaseInOutCurve );
    int const frameTime = qRound( 1000.0 / fps );
    for ( int i=1; i<=timeLine.duration(); i+=frameTime ) {
        printf("[%i%% done]\r", cvRound( (100.0*i)/timeLine.duration() ) );
        fflush(stdout);
        interpolate( mapWidget, timeLine.valueForTime( i ), satNb );
        QImage screenshot = QPixmap::grabWidget( mapWidget ).toImage().convertToFormat( QImage::Format_RGB888 );
        Mat converter( frameSize, CV_8UC3 );
        converter.data = screenshot.bits();
        cvtColor( converter, buffer, COLOR_RGB2BGR );
        //cvtColor( converter, buffer, CV_RGB2BGR );
        videoWriter.write( buffer );
    }
    for ( int i=0; i<fps; ++i ) {
        videoWriter.write( buffer ); // one second stand-still at end
    }
    printf("Wrote %s\n", videoFile.c_str());
}

int main(int argc, char** argv)
{
    QApplication app(argc,argv);
    SatPathWidget *mapWidget = new SatPathWidget;
    mapWidget->readData();
    mapWidget->setMapThemeId(mapTheme);
    foreach( RenderPlugin* plugin, mapWidget->renderPlugins() ) {
        if ( !features.contains( plugin->nameId() ) ) {
            plugin->setEnabled( false );
        }
    }

    animatedFlight( mapWidget );
    return 0;
}
