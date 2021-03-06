//
//  CameraFrameConverter.hpp
//  ARToolkit
//
//  Created by Joseph Chow on 8/4/17.
//

#ifndef CameraFrameConverter_hpp
#define CameraFrameConverter_hpp

#include "ofMain.h"
#include "ofxiOS.h"
#include <memory>
#include <ARKit/ARKit.h>
#define STRINGIFY(A) #A

typedef std::shared_ptr<class ARProcessor>ARRef;

/**
     Processing class to help deal with ARKit stuff like grabbing and converting the camera feed,
     set anchors, etc.
 */
class ARProcessor {
    
    ARSession * session;

    float ambientIntensity;
    
    
    // ========== CAMERA IMAGE STUFF ================= //
    CVOpenGLESTextureRef yTexture;
    CVOpenGLESTextureRef CbCrTexture;
    CVOpenGLESTextureCacheRef _videoTextureCache;
    
    // mesh to render camera image
    ofMesh cameraPlane;
    
    // shader to color convert and render the camera image
    ofShader cameraShader;
    
    // this handles rotating the camera image to the correct orientation.
    ofMatrix4x4 rotation;
    
    // holds camera transform from ARKit
    ofMatrix4x4 cameraTransform;
    
    // holds camera projection from ARKit
    ofMatrix4x4 cameraProjection;
    
    // a reference to the current frame in the scene
    ARFrame * currentFrame;
    
    // to help reduce resource strain, making building the camera frame optional
    bool shouldBuildCameraFrame;

    // ========== SHADERS ================ //
    // keep camera image shader source in directly since this will never really have to change.
    // Shaders built with the help of
    // https://github.com/BradLarson/GPUImage
    
    //Specifically between these couple files
    // https://github.com/BradLarson/GPUImage/blob/167b0389bc6e9dc4bb0121550f91d8d5d6412c53/framework/Source/GPUImageColorConversion.m
    // https://github.com/BradLarson/GPUImage/blob/167b0389bc6e9dc4bb0121550f91d8d5d6412c53/framework/Source/GPUImageVideoCamera.m
    const std::string vertex_shader = STRINGIFY(
                                          attribute vec2 position;
                                          varying vec2 vUv;
                                          uniform mat4 rotationMatrix;
                                          
                                          const vec2 scale = vec2(0.5,0.5);
                                          void main(){
                                              vUv = position.xy * scale + scale;
                                              
                                              gl_Position = rotationMatrix * vec4(position,0.0,1.0);
                                          }
                                          

    );
    
    
   
    const std::string fragment_shader = STRINGIFY(
                                            precision highp float;
                                            
                                            // this is the yyuv texture from ARKit
                                            uniform sampler2D yMap;
                                            uniform sampler2D uvMap;
                                            varying vec2 vUv;
                                            
                                            
                                                void main(){
                                                
                                                // flip uvs so image isn't inverted.
                                                vec2 textureCoordinate = vec2(vUv.s,1.0 - vUv.t);
                                                
                                                // Using BT.709 which is the standard for HDTV
                                                mat3 colorConversionMatrix = mat3(
                                                                                  1.164,  1.164, 1.164,
                                                                                  0.0, -0.213, 2.112,
                                                                                  1.793, -0.533,   0.0
                                                                                  );
                                                
                                                
                                                mediump vec3 yuv;
                                                lowp vec3 rgb;
                                                
                                                yuv.x = texture2D(yMap, textureCoordinate).r - (16.0/255.0);
                                                yuv.yz = texture2D(uvMap, textureCoordinate).ra - vec2(0.5, 0.5);
                                                
                                                rgb = colorConversionMatrix * yuv;
                                                
                                                gl_FragColor = vec4(rgb,1.);
                                            }
                                            

                                            
                                            

        
    );
    
    // Converts the CVPixelBufferIndex into a OpenGL texture
    CVOpenGLESTextureRef createTextureFromPixelBuffer(CVPixelBufferRef pixelBuffer,int planeIndex,GLenum format=GL_LUMINANCE,int width=0,int height=0);
    
    // Constructs camera frame from pixel data
    void buildCameraFrame(CVPixelBufferRef pixelBuffer);
public:
    ARProcessor();
    ARProcessor(ARSession * session);
    ~ARProcessor();
    
    static ARRef create(ARSession * session){
        return ARRef(new ARProcessor(session));
    }
    static ARRef create(){
        return ARRef(new ARProcessor());
    }
    
    
    void updatePlanes();
    void addAnchor();
    void pauseSession();
    void startSession();
    
    void setup();
    void update();
    
    // draws the camera frame
    void draw();
    
    // alias for draw
    void drawCameraFrame();
  
};


#endif /* CameraFrameConverter_hpp */
