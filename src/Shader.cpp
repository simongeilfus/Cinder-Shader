#include "Shader.h"
#include "cinder/gl/Texture.h"


std::set<Shader*>  ShaderLibrary::mSet;

Shader::Shader() {
}

void Shader::createProgram()
{
    mObj = std::shared_ptr<Obj>( new Obj );
    mObj->mHandle = glCreateProgram();
}
    
void Shader::loadShader( ci::DataSourceRef shaderSourceRef, GLint shaderType, GLint geometryInputType, GLint geometryOutputType, GLint geometryOutputVertices )
{
    if( shaderSourceRef ){
        mSource[shaderType] = shaderSourceRef;
        mLastTime[shaderType] = ci::fs::last_write_time( shaderSourceRef->getFilePath() );
        GlslProg::loadShader( shaderSourceRef->getBuffer(), shaderType );
    
        if( shaderType == GL_GEOMETRY_SHADER_EXT ){
            glProgramParameteriEXT(mObj->mHandle, GL_GEOMETRY_INPUT_TYPE_EXT, geometryInputType);
            glProgramParameteriEXT(mObj->mHandle, GL_GEOMETRY_OUTPUT_TYPE_EXT, geometryOutputType);
            glProgramParameteriEXT(mObj->mHandle, GL_GEOMETRY_VERTICES_OUT_EXT, geometryOutputVertices);
            
            mGeometryData = GeomData( geometryInputType, geometryOutputType, geometryOutputVertices );
        }
    }
}

void Shader::loadShader( const char *shader, GLint shaderType, GLint geometryInputType, GLint geometryOutputType, GLint geometryOutputVertices ){
    GlslProg::loadShader( shader, shaderType );
        
    if( shaderType == GL_GEOMETRY_SHADER_EXT ){
        glProgramParameteriEXT(mObj->mHandle, GL_GEOMETRY_INPUT_TYPE_EXT, geometryInputType);
        glProgramParameteriEXT(mObj->mHandle, GL_GEOMETRY_OUTPUT_TYPE_EXT, geometryOutputType);
        glProgramParameteriEXT(mObj->mHandle, GL_GEOMETRY_VERTICES_OUT_EXT, geometryOutputVertices);
    }
}
    
void Shader::link()
{
    for( int i = 0; i < mSeparatedTransformFeedbackVarying.size(); i++ ){
        const char *attrib[] = { mSeparatedTransformFeedbackVarying[i].c_str() };
        glTransformFeedbackVaryingsEXT( getHandle(), 1, attrib, GL_SEPARATE_ATTRIBS_EXT );
    }
    
    if( mInterleavedTransformFeedbackVarying.size() ){
        const char* attribs[mInterleavedTransformFeedbackVarying.size()];
        for( int i = 0; i < mInterleavedTransformFeedbackVarying.size(); i++ ){
            attribs[i] = mInterleavedTransformFeedbackVarying[i].c_str();
        }
        glTransformFeedbackVaryingsEXT( getHandle(), mInterleavedTransformFeedbackVarying.size(), attribs, GL_INTERLEAVED_ATTRIBS_EXT );
    }
    
	glLinkProgram( mObj->mHandle );
    
}
    
    
bool Shader::update()
{
    bool reloadVertex   = false;
    bool reloadFragment = false;
    bool reloadGeometry = false;
    
    if( mSource.count( GL_VERTEX_SHADER_ARB ) && mLastTime[GL_VERTEX_SHADER_ARB] < ci::fs::last_write_time( mSource[GL_VERTEX_SHADER_ARB]->getFilePath() ) ){ 
        reloadVertex = true;
    }
    if( mSource.count( GL_FRAGMENT_SHADER_ARB ) && mLastTime[GL_FRAGMENT_SHADER_ARB] < ci::fs::last_write_time( mSource[GL_FRAGMENT_SHADER_ARB]->getFilePath() ) ){
        reloadFragment = true;
    }
    if( mSource.count( GL_GEOMETRY_SHADER_EXT ) && mLastTime[GL_GEOMETRY_SHADER_EXT] < ci::fs::last_write_time( mSource[GL_GEOMETRY_SHADER_EXT]->getFilePath() ) ){
        reloadGeometry = true;
    }
    
    if( reloadFragment || reloadVertex || reloadGeometry ){
        
        createProgram();
        
        if( mSource.count( GL_VERTEX_SHADER_ARB ) ){
            try{ loadShader( ci::loadFile( mSource[GL_VERTEX_SHADER_ARB]->getFilePath() ), GL_VERTEX_SHADER_ARB ); }
            catch( ci::gl::GlslProgCompileExc exc ){ std::cout << exc.what() << std::endl; }
        }
		if( mSource.count( GL_FRAGMENT_SHADER_ARB ) ){
            try{ loadShader( ci::loadFile( mSource[GL_FRAGMENT_SHADER_ARB]->getFilePath() ), GL_FRAGMENT_SHADER_ARB );}
            catch( ci::gl::GlslProgCompileExc exc ){ std::cout << exc.what() << std::endl; }
        }
		if( mSource.count( GL_GEOMETRY_SHADER_EXT ) ){
            try{ loadShader( ci::loadFile( mSource[GL_GEOMETRY_SHADER_EXT]->getFilePath() ), GL_GEOMETRY_SHADER_EXT, mGeometryData.mInputType, mGeometryData.mOutputType, mGeometryData.mOutputVertices );}
            catch( ci::gl::GlslProgCompileExc exc ){ std::cout << exc.what() << std::endl; }
        }
        
        link();
        
        return true;
    }
    
    return false;
}


void Shader::bindAttribLocation( const std::string &name, GLuint location )
{
    glBindAttribLocation( getHandle(), location, name.c_str() );
}

void Shader::transformFeedbackVarying( const std::string &name )
{
    mSeparatedTransformFeedbackVarying.push_back( name );
}    
void Shader::transformFeedbackVaryings( const char* names[], int n )
{
    for( int i = 0; i < n; i++ ) mInterleavedTransformFeedbackVarying.push_back( names[i] );
}    


void Shader::uniformTexture( const std::string &name, ci::gl::Texture tex, int slot )
{
    glActiveTexture( GL_TEXTURE0 + slot );
    glEnable( tex.getTarget() );
    glBindTexture( tex.getTarget(), tex.getId() );
    glDisable( tex.getTarget() );
    
	GLint loc = getUniformLocation( name );
	glUniform1i( loc, slot );
    
    glActiveTexture( GL_TEXTURE0 );
}