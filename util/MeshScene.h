
/*
 * Copyright (c) 2008 - 2009 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and proprietary
 * rights in and to this software, related documentation and any modifications thereto.
 * Any use, reproduction, disclosure or distribution of this software and related
 * documentation without an express license agreement from NVIDIA Corporation is strictly
 * prohibited.
 *
 * TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS*
 * AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED,
 * INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS BE LIABLE FOR ANY
 * SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT
 * LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
 * BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR
 * INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGES
 */

//-----------------------------------------------------------------------------
//
//  MeshScene.h
//  
//-----------------------------------------------------------------------------

#ifndef MESHSCENE_H
#define MESHSCENE_H

#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_aabb_namespace.h>
#include <sutil.h>
#include <string>

//------------------------------------------------------------------------------
//
// MeshScene class
//
//------------------------------------------------------------------------------
class MeshScene : public SampleScene
{
public:
   MeshScene();
   MeshScene(bool accel_cache_loaded, bool accel_caching_on, bool accel_large_mesh);
   virtual ~MeshScene() {}

  // Setters for controlling application behavior
   void setMesh( const char* filename )             { m_filename = filename;             }
   void setAccelCaching( bool onoff )               { m_accel_caching_on = onoff;        }
   void setTraverser( const char* traverser )       { m_accel_traverser = traverser;     }
   void setBuilder( const char* builder )           { m_accel_builder = builder;         }
   void setRefine( const char* ref )                { m_accel_refine = ref;              }
   void setLargeMesh( bool f)                       { m_accel_large_mesh = f;            }
   void loadAccelCache();
   void saveAccelCache();

protected:
  std::string getCacheFileName();

  std::string   m_filename;

  std::string   m_accel_builder;
  std::string   m_accel_traverser;
  std::string   m_accel_refine;
  bool          m_accel_cache_loaded;
  bool          m_accel_caching_on;
  bool          m_accel_large_mesh;

  optix::GeometryGroup m_geometry_group;

   const static int WIDTH;
   const static int HEIGHT;
};


#endif // MESHSCENE_H
