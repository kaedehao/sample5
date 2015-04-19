//
//  Sample5Scene.h
//  Optix
//
//  Created by Hao Luo on 3/13/15.
//  Copyright (c) 2015 Hao Luo. All rights reserved.
//

#ifndef SMAPLE5_H
#define SMAPLE5_H

#include "glwidget.h"

#include <QImage>
#include <QKeyEvent>


class Sample5Scene : public SampleScene
{
public:
    Sample5Scene() : SampleScene(), m_frame_number( 0 ), m_adaptive_aa( false ), m_width( 1024u ), m_height( 768u ) {}
    //~Sample5Scene();

    //From SampleScene
    void initScene( InitialCameraData& camera_data );
    void trace( const RayGenCameraData& camera_date );
    //void doResize( unsigned int width, unsigned height );
    optix::Buffer getOutputBuffer();
    bool keyPressEvent( int key );

    void setAdaptiveAA( bool adaptive_aa ) { m_adaptive_aa = adaptive_aa; }
    bool adaptive_aa(){ return m_adaptive_aa; }

private:
    int getEntryPoint() { return m_adaptive_aa ? AdaptivePinhole: Pinhole; }
    void genRndSeeds( unsigned int width, unsigned int height );

    enum {
        Pinhole = 0,
        AdaptivePinhole = 1
    };

    void createGeometry();

    optix::Buffer m_rnd_seeds;
    unsigned int  m_frame_number;
    bool          m_adaptive_aa;

    unsigned int  m_width;
    unsigned int  m_height;

    std::string   texpath( const std::string& base );
    std::string   texture_path;
};
#endif // SMAPLE5_H
