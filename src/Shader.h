#pragma once

#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"

#include <set>


class Shader : public ci::gl::GlslProg {
  public: 
	Shader();
    
    void bindAttribLocation( const std::string &name, GLuint location );
    
    void transformFeedbackVarying( const std::string &name );
    void transformFeedbackVaryings( const char* names[], int n );
    
    void createProgram();
    void loadShader( ci::DataSourceRef shaderSourceRef, GLint shaderType, GLint geometryInputType = GL_POINTS, GLint geometryOutputType = GL_TRIANGLES, GLint geometryOutputVertices = 0 );
	void loadShader( const char *shader, GLint shaderType, GLint geometryInputType = GL_POINTS, GLint geometryOutputType = GL_TRIANGLES, GLint geometryOutputVertices = 0);
	void link();

    bool update();
    
	void uniformTexture( const std::string &name, ci::gl::Texture tex, int slot = 0 );
    
    
protected:
    struct GeomData {
        GeomData( GLint geometryInputType = GL_POINTS, GLint geometryOutputType = GL_TRIANGLES, GLint geometryOutputVertices = 0 )
        : mInputType( geometryInputType ), mOutputType( geometryOutputType ), mOutputVertices( geometryOutputVertices )  {}
        
        GLint mInputType, mOutputType, mOutputVertices;
    };
    
    std::map<GLenum, ci::DataSourceRef> mSource;
    std::map<GLenum, GLuint>            mShader;
	std::map<GLenum, std::time_t>       mLastTime;
    std::vector< std::string >          mSeparatedTransformFeedbackVarying;
    std::vector< std::string >          mInterleavedTransformFeedbackVarying;
    GeomData                            mGeometryData;
    
};


class ShaderLibrary {
public:
    static void Add( Shader *shader )
    {
        if( mSet.count( shader ) <= 0 ) mSet.insert( shader );
    }
    static void CheckAndReloadAll()
    {
        for( std::set<Shader*>::iterator it = mSet.begin(); it != mSet.end(); ++it ){
            (*it)->update();
        }
    }
    
protected:
    static std::set<Shader*>  mSet;
};