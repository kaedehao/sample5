//#if defined(__APPLE__)
//#  include <OpenGL/gl.h>
//#else
//#  include <GL/glew.h>
//#  if defined(_WIN32)
//#    include <GL/wglew.h>
//#  endif
//#  include <GL/gl.h>
//#endif

#include "sample5.h"
#include "glwidget.h"
#include <ImageLoader.h>
#include "commonStructs.h"
#include "src/random.h"
#include <QColor>

#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu.h>
#include <iostream>
//#include <sstream>
//#include <cstdlib>
//#include <algorithm>


#define NUM_SPHERE 2

//----------------------------------------------------------------------------------------------------------------------
Sample5Scene::Sample5Scene()
    : m_camera_changed( true ), m_use_vbo_buffer( true ), m_num_devices( 0 ), m_cpu_rendering_enabled( false ), m_frame_number( 0 ), m_adaptive_aa( false ), m_width( 1024u ), m_height( 768u )
{
    m_context = optix::Context::create();
}
//----------------------------------------------------------------------------------------------------------------------
Sample5Scene::~Sample5Scene(){
    // clean up
    getOutputBuffer()->destroy();
    m_context->destroy();
    m_context = 0;
}

const char* const Sample5Scene::ptxpath( const std::string& target, const std::string& base )
{
    static std::string path;
    path = "ptx/" + target + "_generated_" + base + ".ptx";
    return path.c_str();
}

void Sample5Scene::cleanUp()
{
  m_context->destroy();
  m_context = 0;
}

std::string Sample5Scene::texpath( const std::string& base )
{
    texture_path = "/Users/haoluo/qt-workspace/sample5/data";
    return texture_path + "/" + base;
}

void Sample5Scene::resize(unsigned int width, unsigned int height)
{
  try {
    Buffer buffer = getOutputBuffer();
    buffer->setSize( width, height );

    if(m_use_vbo_buffer)
    {
      buffer->unregisterGLBuffer();
      glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer->getGLBOId());
      glBufferData(GL_PIXEL_UNPACK_BUFFER, buffer->getElementSize() * width * height, 0, GL_STREAM_DRAW);
      glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
      buffer->registerGLBuffer();
    }

  } catch( Exception& e ){
    sutilReportError( e.getErrorString().c_str() );
    exit(2);
  }

  // Let the user resize any other buffers
  //doResize( width, height );
}

//----------------------------------------------------------------------------------------------------------------------
void Sample5Scene::genRndSeeds( unsigned int width, unsigned int height )
{
  unsigned int* seeds = static_cast<unsigned int*>( m_rnd_seeds->map() );
  fillRandBuffer( seeds, width*height );
  m_rnd_seeds->unmap();
}

void Sample5Scene::initScene( InitialCameraData& camera_data )
{
    // context
    m_context->setRayTypeCount(2);
    m_context->setEntryPointCount(2);
    m_context->setStackSize( 2800 );

    m_context["max_depth"]->setInt( 10u );
    m_context["radiance_ray_type"]->setUint( 0u );
    m_context["shadow_ray_type"]->setUint( 1u );
    m_context["frame_number"]->setUint( 0u );
    m_context["scene_epsilon"]->setFloat( 1.e-4f );
    m_context["ambient_light_color"]->setFloat( 0.4f, 0.4f, 0.4f );

    // Render result bufffer
    m_context["output_buffer"]->set( createOutputBuffer(RT_FORMAT_UNSIGNED_BYTE4, m_width, m_height) );
    //m_outputBuffer = m_context["output_buffer"]->getBuffer();

    // Pinhole Camera ray gen and exception program
    std::string ptx_path = ptxpath( "sample5", "pinhole_camera.cu" );
    m_context->setRayGenerationProgram( Pinhole, m_context->createProgramFromPTXFile(ptx_path, "pinhole_camera" ) );
    m_context->setExceptionProgram( Pinhole, m_context->createProgramFromPTXFile(ptx_path, "exception" ) );

    // Adaptive Pinhole Camera ray gen and exception program
    ptx_path = ptxpath( "sample5", "adaptive_pinhole_camera.cu" );
    m_context->setRayGenerationProgram( AdaptivePinhole, m_context->createProgramFromPTXFile( ptx_path, "pinhole_camera" ) );
    m_context->setExceptionProgram(     AdaptivePinhole, m_context->createProgramFromPTXFile( ptx_path, "exception" ) );

    m_context["bad_color"]->setFloat( 1.0f, 0.0f, 0.0f);

    // Miss program
    Program miss_program = m_context->createProgramFromPTXFile( ptxpath( "sample5", "envmap.cu" ), "envmap_miss" );
    m_context->setMissProgram( 0, miss_program );
    const float3 default_color = make_float3(1.0f, 1.0f, 1.0f);
    m_context["envmap"]->setTextureSampler( loadTexture( m_context, texpath("autumn.ppm"), default_color) );
    m_context["bg_color"]->setFloat( make_float3( 0.3f, 0.3f, 0.3f ) );

    // Lights
    BasicLight lights[] = {
        { make_float3( 60.0f, 40.0f, 0.0f ), make_float3( 1.0f, 1.0f, 1.0f ), 1 }
        };

    Buffer light_buffer = m_context->createBuffer( RT_BUFFER_INPUT );
    light_buffer->setFormat( RT_FORMAT_USER );
    light_buffer->setElementSize( sizeof(BasicLight) );
    light_buffer->setSize( sizeof(lights)/sizeof(lights[0]) );
    memcpy( light_buffer->map(), lights, sizeof(lights) );
    light_buffer->unmap();

    m_context["lights"]->set( light_buffer );
    // Set up camera
    camera_data = InitialCameraData( make_float3( 0.0f, 0.0f, 5.0f ), // eye
                                     make_float3( 0.0f, 0.0f, 0.0f ), // lookat
                                     make_float3( 0.0f, 1.0f, 0.0f ), // up
                                     60.0f );                         // vfov

    m_context["eye"]->setFloat( make_float3( 0.0f, 0.0f, 0.0f ) );
    m_context["U"]->setFloat( make_float3( 0.0f, 0.0f, 0.0f ) );
    m_context["V"]->setFloat( make_float3( 0.0f, 0.0f, 0.0f ) );
    m_context["W"]->setFloat( make_float3( 0.0f, 0.0f, 0.0f ) );

    // Painting camera
    //m_context->setPrintEnabled(1);
    //m_context->setPrintBufferSize(1028);
    m_context["camera_paint_map"]->setTextureSampler( loadTexture( m_context, texpath("paint_camera/magic_bg.ppm"), default_color) );
    // Posing camera
    m_context["camera_pose_map"]->setTextureSampler( loadTexture( m_context, texpath("paint_camera/cubist.ppm"), default_color) );
    // Paint or Pose or both
    m_context["paint_camera_type"]->setUint( 0u );

    // Variance buffers
    Buffer variance_sum_buffer = m_context->createBuffer( RT_BUFFER_INPUT_OUTPUT | RT_BUFFER_GPU_LOCAL,
                                                          RT_FORMAT_FLOAT4,
                                                          m_width, m_height );
    memset( variance_sum_buffer->map(), 0, m_width*m_height*sizeof(float4) );
    variance_sum_buffer->unmap();
    m_context["variance_sum_buffer"]->set( variance_sum_buffer );

    Buffer variance_sum2_buffer = m_context->createBuffer( RT_BUFFER_INPUT_OUTPUT | RT_BUFFER_GPU_LOCAL,
                                                           RT_FORMAT_FLOAT4,
                                                           m_width, m_height );
    memset( variance_sum2_buffer->map(), 0, m_width*m_height*sizeof(float4) );
    variance_sum2_buffer->unmap();
    m_context["variance_sum2_buffer"]->set( variance_sum2_buffer );

    // Sample count buffer
    Buffer num_samples_buffer = m_context->createBuffer( RT_BUFFER_INPUT_OUTPUT | RT_BUFFER_GPU_LOCAL,
                                                         RT_FORMAT_UNSIGNED_INT,
                                                         m_width, m_height );
    memset( num_samples_buffer->map(), 0, m_width*m_height*sizeof(unsigned int) );
    num_samples_buffer->unmap();
    m_context["num_samples_buffer"]->set( num_samples_buffer);

    // RNG seed buffer
    m_rnd_seeds = m_context->createBuffer( RT_BUFFER_INPUT_OUTPUT | RT_BUFFER_GPU_LOCAL,
                                           RT_FORMAT_UNSIGNED_INT,
                                           m_width, m_height );
    m_context["rnd_seeds"]->set( m_rnd_seeds );
    genRndSeeds( m_width, m_height );

    // Populate scene hierarchy
    createGeometry();

    // Prepare to run
    m_context->validate();
    m_context->compile();

}
//----------------------------------------------------------------------------------------------------------------------

// Return whether we processed the key or not
bool Sample5Scene::keyPressEvent( int key )
{
    switch ( key )
    {
        case Qt::Key_A:
            m_adaptive_aa = !m_adaptive_aa;
            m_camera_changed = true;
            glWidget::setContinuousMode( m_adaptive_aa ? glWidget::CDProgressive : glWidget::CDNone );
            return true;
    }
    return false;
}

Buffer Sample5Scene::getOutputBuffer()
{
    return m_context["output_buffer"]->getBuffer();
}

Buffer Sample5Scene::createOutputBuffer( RTformat format,
                                        unsigned int width,
                                        unsigned int height )
{
  // Set number of devices to be used
  // Default, 0, means not to specify them here, but let OptiX use its default behavior.
  if(m_num_devices)
  {
    int max_num_devices    = Context::getDeviceCount();
    int actual_num_devices = std::min( max_num_devices, std::max( 1, m_num_devices ) );
    std::vector<int> devs(actual_num_devices);
    for( int i = 0; i < actual_num_devices; ++i ) devs[i] = i;
    m_context->setDevices( devs.begin(), devs.end() );
  }

  Buffer buffer;

  if ( m_use_vbo_buffer && !m_cpu_rendering_enabled )
  {
    /*
      Allocate first the memory for the gl buffer, then attach it to OptiX.
    */
    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    size_t element_size;
    m_context->checkError(rtuGetSizeForRTformat(format, &element_size));
    glBufferData(GL_ARRAY_BUFFER, element_size * width * height, 0, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    buffer = m_context->createBufferFromGLBO(RT_BUFFER_OUTPUT, vbo);
    buffer->setFormat(format);
    buffer->setSize( width, height );
  }
  else {
    buffer = m_context->createBuffer( RT_BUFFER_OUTPUT, format, width, height);
  }

  return buffer;
}

void
Sample5Scene::setNumDevices( int ndev )
{
  m_num_devices = ndev;

  if (m_cpu_rendering_enabled && m_num_devices > 0) {
    rtContextSetAttribute(m_context.get()->get(), RT_CONTEXT_ATTRIBUTE_CPU_NUM_THREADS, sizeof(m_num_devices), &m_num_devices);
  }
}

void
Sample5Scene::enableCPURendering(bool enable)
{
  // Is CPU mode already enabled
  std::vector<int> devices = m_context->getEnabledDevices();
  bool isCPUEnabled = false;
  for(std::vector<int>::const_iterator iter = devices.begin(); iter != devices.end(); ++iter)
  {
    if (m_context->getDeviceName(*iter) == "CPU") {
      isCPUEnabled = true;
      break;
    }
  }

  // Already in desired state, good-bye.
  if (isCPUEnabled == enable)
    return;

  if (enable)
  {
    // Turn on CPU mode

    int ordinal;
    for(ordinal = m_context->getDeviceCount()-1; ordinal >= 0; ordinal--)
    {
      if (m_context->getDeviceName(ordinal) == "CPU") {
        break;
      }
    }
    if (ordinal < 0)
      throw Exception("Attempting to enable CPU mode, but no CPU device found");
    m_context->setDevices(&ordinal, &ordinal+1);
  } else
  {
    // Turn off CPU mode

    // For now, simply grab the first device
    int ordinal = 0;
    m_context->setDevices(&ordinal, &ordinal+1);
  }

  // Check this here, in case we failed to make it into GPU mode.
  updateCPUMode();
}

void
Sample5Scene::updateCPUMode()
{
  m_cpu_rendering_enabled = m_context->getDeviceName(m_context->getEnabledDevices()[0]) == "CPU";
  if (m_cpu_rendering_enabled)
    m_use_vbo_buffer = false;
}
//----------------------------------------------------------------------------------------------------------------------
void Sample5Scene::createGeometry()
{
    // Sphere geometry
    std::string sphere_ptx( ptxpath( "sample5", "sphere.cu") );
    Geometry sphere = m_context->createGeometry();
    sphere->setPrimitiveCount( 1u );
    sphere->setBoundingBoxProgram( m_context->createProgramFromPTXFile( sphere_ptx, "bounds" ) );
    sphere->setIntersectionProgram( m_context->createProgramFromPTXFile( sphere_ptx, "intersect" ) );
    sphere["sphere"]->setFloat( 0, 0, 0, 1.0f );

    // Sphere Shell geometry
    std::string shell_ptx( ptxpath( "sample5", "sphere_shell.cu") );
    Geometry glass_sphere = m_context->createGeometry();
    glass_sphere->setPrimitiveCount( 1u );
    glass_sphere->setBoundingBoxProgram( m_context->createProgramFromPTXFile( shell_ptx, "bounds" ) );
    glass_sphere->setIntersectionProgram( m_context->createProgramFromPTXFile( shell_ptx, "intersect" ) );
    glass_sphere["center"]->setFloat( 0, 0, 0 );

    glass_sphere["radius1"]->setFloat( 0.96f );
    glass_sphere["radius2"]->setFloat( 1.0f );

    // Floor geometry
    std::string pgram_ptx( ptxpath( "sample5", "parallelogram.cu" ) );
    Geometry parallelogram = m_context->createGeometry();
    parallelogram->setPrimitiveCount( 1u );
    parallelogram->setBoundingBoxProgram( m_context->createProgramFromPTXFile( pgram_ptx, "bounds" ) );
    parallelogram->setIntersectionProgram( m_context->createProgramFromPTXFile( pgram_ptx, "intersect" ) );
    float3 anchor = make_float3( -16.0f, 0.01f, -8.0f );
    float3 v1 = make_float3( 32.0f, 0.0f, 0.0f );
    float3 v2 = make_float3( 0.0f, 0.0f, 16.0f );
    float3 normal = cross( v1, v2 );
    normal = normalize( normal );
    float d = dot( normal, anchor );
    v1 *= 1.0f/dot( v1, v1 );
    v2 *= 1.0f/dot( v2, v2 );
    float4 plane = make_float4( normal, d );
    parallelogram["plane"]->setFloat( plane );
    parallelogram["v1"]->setFloat( v1 );
    parallelogram["v2"]->setFloat( v2 );
    parallelogram["anchor"]->setFloat( anchor );


    // Normal material
    Program normal_ch = m_context->createProgramFromPTXFile( ptxpath( "sample5", "normal_shader.cu" ), "closest_hit_radiance" );
    Material normal_matl = m_context->createMaterial();
    normal_matl->setClosestHitProgram( 0, normal_ch );

    // Phong material
    Program phong_ch = m_context->createProgramFromPTXFile( ptxpath( "sample5", "phong.cu" ),  "closest_hit_radiance" );
    Program phong_ah = m_context->createProgramFromPTXFile( ptxpath( "sample5", "phong.cu" ), "any_hit_shadow" );
    Material metal_matl = m_context->createMaterial();
    metal_matl->setClosestHitProgram( 0, phong_ch );
    metal_matl->setAnyHitProgram( 1, phong_ah );
    metal_matl["Ka"]->setFloat( 0.2f, 0.5f, 0.5f );
    metal_matl["Kd"]->setFloat( 0.2f, 0.7f, 0.8f );
    metal_matl["Ks"]->setFloat( 0.9f, 0.9f, 0.9f );
    metal_matl["phong_exp"]->setFloat( 64 );
    metal_matl["reflectivity"]->setFloat( 0.5f,  0.5f,  0.5f);

    // Glass material
    Program glass_ch = m_context->createProgramFromPTXFile( ptxpath( "sample5", "glass.cu" ), "closest_hit_radiance" );
    Program glass_ah = m_context->createProgramFromPTXFile( ptxpath( "sample5", "glass.cu" ), "any_hit_shadow" );
    Material glass_matl = m_context->createMaterial();
    glass_matl->setClosestHitProgram( 0, glass_ch );
    glass_matl->setAnyHitProgram( 1, glass_ah );

    glass_matl["importance_cutoff"]->setFloat( 01e-2f );
    glass_matl["cutoff_color"]->setFloat( 0.034f, 0.055f, 0.085f );
    glass_matl["fresnel_exponent"]->setFloat( 3.0f );
    glass_matl["fresnel_minimum"]->setFloat( 0.1f );
    glass_matl["fresnel_maximum"]->setFloat( 1.0f );
    glass_matl["refraction_index"]->setFloat( 1.4f );
    glass_matl["refraction_color"]->setFloat( 1.0f, 1.0f, 1.0f );
    glass_matl["reflection_color"]->setFloat( 1.0f, 1.0f, 1.0f );
    glass_matl["refraction_maxdepth"]->setInt( 10 );
    glass_matl["reflection_maxdepth"]->setInt( 5 );
    float3 extinction = make_float3(.83f, .83f, .83f);
    glass_matl["extinction_constant"]->setFloat( log(extinction.x), log(extinction.y), log(extinction.z) );
    glass_matl["shadow_attenuation"]->setFloat( 0.6f, 0.6f, 0.6f );

    // Checker material for floor
    Program check_ch = m_context->createProgramFromPTXFile(ptxpath( "sample5", "checker.cu" ), "closest_hit_radiance" );
    Program check_ah = m_context->createProgramFromPTXFile(ptxpath( "sample5", "checker.cu" ), "any_hit_shadow" );
    Material floor_matl = m_context->createMaterial();
    floor_matl->setClosestHitProgram( 0, check_ch );
    floor_matl->setAnyHitProgram( 1, check_ah );

    floor_matl["Kd1"]->setFloat( 0.8f, 0.3f, 0.15f);
    floor_matl["Ka1"]->setFloat( 0.8f, 0.3f, 0.15f);
    floor_matl["Ks1"]->setFloat( 0.0f, 0.0f, 0.0f);
    floor_matl["Kd2"]->setFloat( 0.9f, 0.85f, 0.05f);
    floor_matl["Ka2"]->setFloat( 0.9f, 0.85f, 0.05f);
    floor_matl["Ks2"]->setFloat( 0.0f, 0.0f, 0.0f);
    floor_matl["inv_checker_size"]->setFloat( 32.0f, 16.0f, 1.0f );
    floor_matl["phong_exp1"]->setFloat( 0.0f );
    floor_matl["phong_exp2"]->setFloat( 0.0f );
    floor_matl["reflectivity1"]->setFloat( 0.0f, 0.0f, 0.0f);
    floor_matl["reflectivity2"]->setFloat( 0.0f, 0.0f, 0.0f);



    // Initial transform matrix
    const float x=0.0f, y=1.0f, z=0.0f;
    // Matrices are row-major.
    float m[16] = { 1, 0, 0, x,
                    0, 1, 0, y,
                    0, 0, 1, z,
                    0, 0, 0, 1 };

    // Create GIs for each piece of geometry
    Group top_level_group = m_context->createGroup();
    Transform transform;
    GeometryGroup geometrygroup;
    GeometryInstance instance;
    for (int i =0; i<NUM_SPHERE; ++i)
    {
        // Create geometry instance
        if(i>0) // metal sphere
            instance = m_context->createGeometryInstance( sphere, &metal_matl, &metal_matl+1 );
        else
            instance = m_context->createGeometryInstance( glass_sphere, &glass_matl, &glass_matl+1 );

        // place instance in geometry group
        geometrygroup = m_context->createGeometryGroup();
        geometrygroup->setChildCount( 1 );
        geometrygroup->setChild( 0, instance );

        // Create acceleration oject for geometry group
        geometrygroup->setAcceleration( m_context->createAcceleration( "NoAccel",  "NoAccel" ) );
        geometrygroup->getAcceleration()->markDirty();

        // Create transform node
        transform = m_context->createTransform();
        transform->setMatrix( 0, m, 0 );
        m[3] += 3.0f;
        transform->setChild( geometrygroup );

        top_level_group->addChild( transform );
    }

    // Floor
    instance = m_context->createGeometryInstance( parallelogram, &floor_matl, &floor_matl+1 );
    geometrygroup = m_context->createGeometryGroup();
    geometrygroup->setChildCount( 1 );
    geometrygroup->setChild( 0 , instance );
    geometrygroup->setAcceleration( m_context->createAcceleration( "NoAccel",  "NoAccel" ) );
    top_level_group->addChild( geometrygroup );


    top_level_group->setAcceleration( m_context->createAcceleration( "Trbvh", "Bvh" ) );
    top_level_group->getAcceleration()->markDirty();

    // mark acceleration as dirty

    m_context["top_object"]->set( top_level_group );
    m_context["top_shadower"]->set( top_level_group );
}

//----------------------------------------------------------------------------------------------------------------------
void
Sample5Scene::trace( const RayGenCameraData& camera_data, bool& display )
{
  trace(camera_data);
}

void Sample5Scene::trace( const RayGenCameraData& camera_data )
{
//    std::cout<<"eye: "
//            <<camera_data.eye.x<<", "
//            <<camera_data.eye.y<<", "
//            <<camera_data.eye.z<<", "
//            <<std::endl;

//    std::cout<<"U: "
//            <<camera_data.U.x<<", "
//            <<camera_data.U.y<<", "
//            <<camera_data.U.z<<", "
//            <<std::endl;

//    std::cout<<"V: "
//            <<camera_data.V.x<<", "
//            <<camera_data.V.y<<", "
//            <<camera_data.V.z<<", "
//            <<std::endl;

//    std::cout<<"W: "
//            <<camera_data.W.x<<", "
//            <<camera_data.W.y<<", "
//            <<camera_data.W.z<<", "
//            <<std::endl;
    if ( m_camera_changed ) {
        m_frame_number = 0u;
        m_camera_changed = false;
    }

    //launch it
    m_context["eye"]->setFloat( camera_data.eye );
    m_context["U"]->setFloat( camera_data.U );
    m_context["V"]->setFloat( camera_data.V );
    m_context["W"]->setFloat( camera_data.W );
    m_context["frame_number"]->setUint( m_frame_number++ );

    Buffer buffer = m_context["output_buffer"]->getBuffer();
    RTsize buffer_width, buffer_height;
    buffer->getSize( buffer_width, buffer_height );

    m_context->launch( 0,//getEntryPoint(),
                       static_cast<unsigned int>(buffer_width),
                       static_cast<unsigned int>(buffer_height) );

//    QImage img(m_width,m_height,QImage::Format_RGB32);
//    QColor color;
//    int idx;
//    void* data = getOutputBuffer()->map();
//    typedef struct { float r; float g; float b; float a;} rgb;
//    rgb* rgb_data = (rgb*)data;
//    for(unsigned int i=0; i<m_width*m_height; ++i){
//        //std::cout<<rgb_data[i].r<<","<<rgb_data[i].g<<","<<rgb_data[i].b<<std::endl;


////        if(rgb_data[i].r == NULL || rgb_data[i].g == NULL || rgb_data[i].b == NULL || rgb_data[i].a == NULL){
////           // std::cout<<"NULL"<<std::endl;
////            rgb_data[i].r = 1.0f;
////            rgb_data[i].g = 0.0f;
////            rgb_data[i].b = 0.0f;
////            rgb_data[i].a = 1.0f;
////        }

////        if(float(rgb_data[i].r) > 1.0f || float(rgb_data[i].r) < 0.0f || float(rgb_data[i].b) > 1.0f || float(rgb_data[i].b) < 0.0f || float(rgb_data[i].g) > 1.0f || float(rgb_data[i].g) < 0.0f || float(rgb_data[i].a) > 1.0f || float(rgb_data[i].a) < 0.0f){
////           // std::cout<<"OUT"<<std::endl;
////            rgb_data[i].r = 0.0f;
////            rgb_data[i].g = 1.0f;
////            rgb_data[i].b = 0.0f;
////            rgb_data[i].a = 1.0f;
////        }

////        float red = rgb_data[i].r; if(red>1.0) red=1.0;
////        float green = rgb_data[i].g; if(green>1.0) green=1.0;
////        float blue = rgb_data[i].b; if(blue>1.0) blue=1.0;
////        float alpha = rgb_data[i].a; if(alpha>1.0) alpha=1.0;
////        color.setRgbF(red,green,blue,alpha);
//        color.setRgbF(rgb_data[i].r,rgb_data[i].g,rgb_data[i].b,rgb_data[i].a);
//        idx = floor((float)i/m_width);

//        img.setPixel(i-(idx*m_width), idx, color.rgb());

//    }
//    getOutputBuffer()->unmap();
//    img.save("sample5.png","PNG");
//    std::cout<<"sample5 image saved!"<<std::endl;

//    return img;
}

//-----------------------------------------------------------------------------
//
// Scene update
//
//-----------------------------------------------------------------------------
void Sample5Scene::updateGeometry( float radius)//, float center )
{
    Group            top_level_group = m_context["top_object"]-> getGroup();
    Transform        transform       = top_level_group->getChild<Transform>( 0 );
    GeometryGroup    geometrygroup   = transform->getChild<GeometryGroup>();
    GeometryInstance instance        = geometrygroup->getChild( 0 );
    Geometry         geometry          = instance->getGeometry();

    // Sphere
    //geometry["sphere"]->setFloat( center, 0, 0, radius );

    // Sphere Shell
    geometry["radius2"]->setFloat( radius );        // Set Radius
    //geometry["center"]->setFloat( center, 0, 0 );   // set center

    // Mark Dirty
    int childCount = top_level_group->getChildCount();
    for ( int i = 0; i < childCount; i++) {
        if( top_level_group->getChildType( i ) == RT_OBJECTTYPE_TRANSFORM ){
            transform = top_level_group->getChild<Transform>( i );
            geometrygroup = transform->getChild<GeometryGroup>();
        }else{
            geometrygroup = top_level_group->getChild<GeometryGroup>( i );
            //geometrygroup = transform->getChild<GeometryGroup>();
        }
        geometrygroup->getAcceleration()->markDirty();
    }
    top_level_group->getAcceleration()->markDirty();
}

void Sample5Scene::updateMaterial( float refraction_index )
{
    Group            top_level_group = m_context["top_object"]-> getGroup();
    Transform        transform       = top_level_group->getChild<Transform>( 0 );
    GeometryGroup    geometrygroup   = transform->getChild<GeometryGroup>();
    GeometryInstance instance        = geometrygroup->getChild( 0 );
    Material         material        = instance->getMaterial( 0 );

    material["refraction_index"]->setFloat( refraction_index );
}

void Sample5Scene::updateLights(int index, float pos){
    index = 0;
    /* Lights */
    BasicLight lights[] = {
        { make_float3( pos, 40.0f, 0.0f ), make_float3( 1.0f, 1.0f, 1.0f ), 1 }
    };

    memcpy( m_context["lights"]->getBuffer()->map(), lights, sizeof(lights) );
    m_context["lights"]->getBuffer()->unmap();
}

void Sample5Scene::updateAcceleration( bool accel )
{
    if( accel )
        m_context["top_object"]->getGroup()->setAcceleration( m_context->createAcceleration( "Trbvh", "Bvh" ) );
    else
        m_context["top_object"]->getGroup()->setAcceleration( m_context->createAcceleration( "NoAccel", "NoAccel" ) );
}

void Sample5Scene::updatePaintCamera( float scale )
{
    m_context["paint_camera_scale"]->setFloat( scale );
}

void Sample5Scene::paintCameraType( unsigned int type )
{
    m_context["paint_camera_type"]->setUint( type );
}
