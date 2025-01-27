// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>

#include <osg/Geometry>
#include <osg/ShapeDrawable>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Point>
#include <osg/PointSprite>
#include <osg/AlphaFunc>
#include <osgwTools/Shapes.h>
#include <osgwTools/Version.h>

#include <osg/io_utils>




// Allows you to change the animation play rate:
//   '+' speed up
//   '-' slow down
//   'p' pause
class PlayStateHandler
    : public osgGA::GUIEventHandler
{
public:
    PlayStateHandler()
      : elapsedTime( 0. ),
        scalar( 1. ),
        paused( false )
    {}

    double getCurrentTime() const
    {
        if( paused )
            return( elapsedTime );
        else
            return( elapsedTime + ( timer.time_s() * scalar ) );
    }

    virtual bool handle( const osgGA::GUIEventAdapter & event_adaptor,
                         osgGA::GUIActionAdapter & action_adaptor )
    {
        bool handled = false;
        switch( event_adaptor.getEventType() )
        {
            case ( osgGA::GUIEventAdapter::KEYDOWN ):
            {
                int key = event_adaptor.getKey();
                switch( key )
                {
                    case '+': // speed up
                    {
                        elapsedTime = getCurrentTime();
                        timer.setStartTick( timer.tick() );

                        // Increase speed by 33%
                        scalar *= ( 4./3. );

                        handled = true;
                    }
                    break;
                    case '-': // slow down
                    {
                        elapsedTime = getCurrentTime();
                        timer.setStartTick( timer.tick() );

                        // Decrease speed by 25%
                        scalar *= .75;

                        handled = true;
                    }
                    break;
                    case 'p': // pause
                    {
                        elapsedTime = getCurrentTime();
                        timer.setStartTick( timer.tick() );

                        paused = !paused;

                        handled = true;
                    }
                    break;

                }
            }
        }
        return( handled );
    }

private:
    osg::Timer timer;
    double elapsedTime;
    double scalar;
    bool paused;
};



// m and n are the dimensions of the texture that store the position values.
// m * n is the total number of point instances that will be rendered to
// create the streamline.
const int m( 256 );
const int n( 1 );

// Distance between points. Smaller values look better at near distances,
// larger values look better at far distances. Could possible vary this
// value dynamically...
const float dX( .25f );



void
createSLPoint( osg::Geometry& geom, int nInstances,
               const osg::Vec4 color, const float radius )
{
    // Configure a Geometry to draw a single tri pair, but use the draw instanced PrimitiveSet
    // to draw the tri pair multiple times.
    osg::Vec3Array* v = new osg::Vec3Array;
    v->resize( 4 );
    geom.setVertexArray( v );
    (*v)[ 0 ] = osg::Vec3( -radius, -radius, 0. );
    (*v)[ 1 ] = osg::Vec3( radius, -radius, 0. );
    (*v)[ 2 ] = osg::Vec3( -radius, radius, 0. );
    (*v)[ 3 ] = osg::Vec3( radius, radius, 0. );

    osg::Vec2Array* tc = new osg::Vec2Array;
    tc->resize( 4 );
    geom.setTexCoordArray( 1, tc );
    (*tc)[ 0 ] = osg::Vec2( 0., 0. );
    (*tc)[ 1 ] = osg::Vec2( 1., 0. );
    (*tc)[ 2 ] = osg::Vec2( 0., 1. );
    (*tc)[ 3 ] = osg::Vec2( 1., 1. );

    // Streamline color. Blending is non-saturating, so it never
    // reaches full intensity white. Alpha is modulated with the
    // point sprint texture alpha, so the value here is a maximum
    // for the "densist" part of the point sprint texture.
    osg::Vec4Array* c = new osg::Vec4Array;
    c->resize( 1 );
    geom.setColorArray( c );
    geom.setColorBinding( osg::Geometry::BIND_OVERALL );
    (*c)[ 0 ] = color;

#if (OSGWORKS_OSG_VERSION >= 20800 )
    geom.addPrimitiveSet( new osg::DrawArrays( GL_TRIANGLE_STRIP, 0, 4, nInstances ) );
#endif


    osg::StateSet* ss = geom.getOrCreateStateSet();

    // Specify the texture.
    osg::Texture2D *tex = new osg::Texture2D();
    tex->setImage( osgDB::readImageFile( "splotch.png" ) );
    ss->setTextureAttributeAndModes( 1, tex, osg::StateAttribute::ON );

    // Keep pixels with a significant alpha value (discard low-alpha pixels).
    osg::AlphaFunc* af = new osg::AlphaFunc( osg::AlphaFunc::GREATER, 0.05f );
    ss->setAttributeAndModes( af );
}


// Create an array of xyz float position values for each point in the streamline.
float*
createPositionArray( const osg::Vec3& position, int m, int n )
{
    float* pos = new float[ m * n * 3 ];
    float* posI = pos;
    const float x( position.x() ), y( position.y() ), z( position.z() );

    int iIdx;
    for( iIdx = 0; iIdx < m*n; iIdx++ )
    {
        *posI++ = x + iIdx*dX;
        *posI++ = y;
        *posI++ = z + ( 4. * sin( (float)iIdx / (float)(m*n) * osg::PI * 2. ) );
    }

    return pos;
}


// Create a scene graph and state set configured to render a streamline using draw instanced.
osg::Group*
createInstanced( const int m, const int n )
{
    // Essentially a top level Group, a single Geode child, and the
    // Geode contains a Geometry for each streamline, configured to
    // draw a single tri pair. A PrimitiveSet per streamline / per
    // Geometry uses draw instanced so that
    // multiple tri pairs render along the length of the streamline.
    osg::Group* grp = new osg::Group;
    osg::Geode* geode = new osg::Geode;
    grp->addChild( geode );

    const float pointRadius = .5f;

    osg::Geometry* geom = new osg::Geometry;
    // Note:
    // Display Lists and draw instanced are mutually exclusive. Disable
    // display lists and use buffer objects instead.
    geom->setUseDisplayList( false );
    geom->setUseVertexBufferObjects( true );
    osg::Vec3 loc( 0., 0., 0. );
    osg::Vec4 color( .6, .4, 1., 1. );
    createSLPoint( *geom, m*n, color, pointRadius );
    geode->addDrawable( geom );
    {
        // specify the position texture. The vertex shader will index into
        // this texture to obtain position values for each streamline point.
        float* pos = createPositionArray( loc, m, n );
        osg::Image* iPos = new osg::Image;
        iPos->setImage( m, n, 1, GL_RGB32F_ARB, GL_RGB, GL_FLOAT,
            (unsigned char*) pos, osg::Image::USE_NEW_DELETE );
        osg::Texture2D* texPos = new osg::Texture2D( iPos );
        texPos->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST );
        texPos->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST );
        geom->getOrCreateStateSet()->setTextureAttribute( 0, texPos );
    }

    // Note:
    // OSG has no idea where our vertex shader will render the points. For proper culling
    // and near/far computation, set an approximate initial bounding box.
    osg::BoundingBox bb( osg::Vec3( 0., -1., -4. )+loc, osg::Vec3( m*n*dX, 1., 4. )+loc );
    geom->setInitialBound( bb );


    // 2nd streamline
    geom = new osg::Geometry;
    geom->setUseDisplayList( false );
    geom->setUseVertexBufferObjects( true );
    loc.set( 1., -3., 0.5 );
    color.set( 1., .7, .5, 1. );
    createSLPoint( *geom, m*n, color, pointRadius );
    geode->addDrawable( geom );
    {
        // specify the position texture. The vertex shader will index into
        // this texture to obtain position values for each streamline point.
        float* pos = createPositionArray( loc, m, n );
        osg::Image* iPos = new osg::Image;
        iPos->setImage( m, n, 1, GL_RGB32F_ARB, GL_RGB, GL_FLOAT,
            (unsigned char*) pos, osg::Image::USE_NEW_DELETE );
        osg::Texture2D* texPos = new osg::Texture2D( iPos );
        texPos->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST );
        texPos->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST );
        geom->getOrCreateStateSet()->setTextureAttribute( 0, texPos );
    }

    bb = osg::BoundingBox( osg::Vec3( 0., -1., -4. )+loc, osg::Vec3( m*n*dX, 1., 4. )+loc );
    geom->setInitialBound( bb );


    // 3rd streamline
    geom = new osg::Geometry;
    geom->setUseDisplayList( false );
    geom->setUseVertexBufferObjects( true );
    loc.set( -0.5, 2., -0.5 );
    color.set( .5, 1., .6, 1. );
    createSLPoint( *geom, m*n, color, pointRadius );
    geode->addDrawable( geom );
    {
        // specify the position texture. The vertex shader will index into
        // this texture to obtain position values for each streamline point.
        float* pos = createPositionArray( loc, m, n );
        osg::Image* iPos = new osg::Image;
        iPos->setImage( m, n, 1, GL_RGB32F_ARB, GL_RGB, GL_FLOAT,
            (unsigned char*) pos, osg::Image::USE_NEW_DELETE );
        osg::Texture2D* texPos = new osg::Texture2D( iPos );
        texPos->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST );
        texPos->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST );
        geom->getOrCreateStateSet()->setTextureAttribute( 0, texPos );
    }

    bb = osg::BoundingBox( osg::Vec3( 0., -1., -4. )+loc, osg::Vec3( m*n*dX, 1., 4. )+loc );
    geom->setInitialBound( bb );


    osg::StateSet* ss = geode->getOrCreateStateSet();

    osg::ref_ptr< osg::Shader > vertexShader = osg::Shader::readShaderFile(
        osg::Shader::VERTEX, osgDB::findDataFile( "streamline3.vs" ) );

    osg::ref_ptr< osg::Program > program = new osg::Program();
    program->addShader( vertexShader.get() );
    ss->setAttribute( program.get(),
        osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );

    // Note:
    // We will render the streamline points with depth test on and depth write disabled,
    // with an order independent blend. This means we need to draw the streamlines last
    // (so use bin # 10) but we don't need the depth sort, so use bin name "RenderBin".
    ss->setRenderBinDetails( 10, "RenderBin" );

    // Tells the shader the dimensions of our texture: m x n.
    // Required to compute correct texture coordinates from the instance ID.
    osg::ref_ptr< osg::Uniform > sizesUniform =
        new osg::Uniform( "sizes", osg::Vec2( (float)m, (float)n ) );
    ss->addUniform( sizesUniform.get() );

    // Tell the shader the total number of instances: m * n.
    // Required for animation based on the instance ID.
    osg::ref_ptr< osg::Uniform > totalInstancesUniform =
        new osg::Uniform( "totalInstances", (float)(m * n) );
    ss->addUniform( totalInstancesUniform.get() );

    // Specify the number of traces in the streamline set.
    osg::ref_ptr< osg::Uniform > numTracesUniform =
        new osg::Uniform( "numTraces", 5 );
    ss->addUniform( numTracesUniform.get() );

    // Specify the trace interval in seconds. This is the time interval
    // from a single sample point being the head of trace N, to being
    // the head of trace N+1.
    osg::ref_ptr< osg::Uniform > traceIntervalUniform =
        new osg::Uniform( "traceInterval", 1.f );
    ss->addUniform( traceIntervalUniform.get() );

    // Specify the trace length in number of sample points.
    // Alpha of rendered point fades linearly over the trace length.
    osg::ref_ptr< osg::Uniform > traceLengthUniform =
        new osg::Uniform( "traceLength", 14 );
    ss->addUniform( traceLengthUniform.get() );

    // Note:
    // It turns out that SRC_ALPHA, ONE_MINUS_SRC_ALPHA actually is
    // non-saturating. Give it a color just shy of full intensity white,
    // and the result will never saturate to white no matter how many
    // times it is overdrawn.
    osg::ref_ptr< osg::BlendFunc > bf = new osg::BlendFunc(
        GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    ss->setAttributeAndModes( bf.get() );

    // Note:
    // Leave the depth test enabled, but mask off depth writes (4th param is false).
    // This allows us to render the streamline points in any order, front to back
    // or back to front, and not lose any points by depth testing against themselves.
    osg::ref_ptr< osg::Depth > depth = new osg::Depth( osg::Depth::LESS, 0., 1., false );
    ss->setAttributeAndModes( depth.get() );

    // Texture unit uniform for the position texture sampler.
    osg::ref_ptr< osg::Uniform > texPosUniform =
        new osg::Uniform( "texPos", 0 );
    ss->addUniform( texPosUniform.get() );


    return grp;
}

// Make some opaque boxes to show that depth testing works properly.
osg::Group*
createOpaque()
{
    osg::ref_ptr< osg::Group > grp = new osg::Group;
    osg::Geode* geode = new osg::Geode;
    grp->addChild( geode );

#if 0
    // debug, show the origin
    osg::Geometry* geom = osgwTools::makeWirePlane( osg::Vec3( -1., -1., 0. ),
        osg::Vec3( 2., 0., 0. ), osg::Vec3( 0., 2., 0. ), osg::Vec2s( 2, 2) );
    geom->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    geode->addDrawable( geom );
#endif

    osg::Box* box = new osg::Box( osg::Vec3( 16., 0., 0. ), 4., 4., 6. );
    osg::ShapeDrawable* shape = new osg::ShapeDrawable( box );
    shape->setColor( osg::Vec4( .2, .2, 1., 1. ) );
    geode->addDrawable( shape );

    box = new osg::Box( osg::Vec3( 48., 0., 0. ), 4., 4., 6. );
    shape = new osg::ShapeDrawable( box );
    shape->setColor( osg::Vec4( .2, .8, .2, 1. ) );
    geode->addDrawable( shape );

    return( grp.release() );
}

int
main( int argc,
      char ** argv )
#if (OSGWORKS_OSG_VERSION < 20800 )
{
    osg::notify( osg::ALWAYS ) << "Requires OSG version 2.8 or higher." << std::endl;
    return( 1 );
}
#else
{
    osg::ref_ptr< osg::Group > root = new osg::Group;
    root->addChild( createInstanced( m, n ) );
    root->addChild( createOpaque() );

    osgViewer::Viewer viewer;
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.setUpViewOnSingleScreen( 0 );
    viewer.setSceneData( root.get() );

    viewer.setCameraManipulator( new osgGA::TrackballManipulator );

    // Create a PlayStateHandler to track elapsed simulation time
    // and play/pause state.
    osg::ref_ptr< PlayStateHandler > psh = new PlayStateHandler;
    viewer.addEventHandler( psh.get() );

    while (!viewer.done())
    {
        // Get time from the PlayStateHandler.
        double simTime = psh->getCurrentTime();
        viewer.frame( simTime );
    }

    return( 0 );
}
#endif
