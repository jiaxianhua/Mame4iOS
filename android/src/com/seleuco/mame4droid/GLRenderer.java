/*
 * This file is part of MAME4droid.
 *
 * Copyright (C) 2015 David Valdeita (Seleuco)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses>.
 *
 * Linking MAME4droid statically or dynamically with other modules is
 * making a combined work based on MAME4droid. Thus, the terms and
 * conditions of the GNU General Public License cover the whole
 * combination.
 *
 * In addition, as a special exception, the copyright holders of MAME4droid
 * give you permission to combine MAME4droid with free software programs
 * or libraries that are released under the GNU LGPL and with code included
 * in the standard release of MAME under the MAME License (or modified
 * versions of such code, with unchanged license). You may copy and
 * distribute such a system following the terms of the GNU GPL for MAME4droid
 * and the licenses of the other code concerned, provided that you include
 * the source code of that other code when and as the GNU GPL requires
 * distribution of source code.
 *
 * Note that people who make modified versions of MAME4idroid are not
 * obligated to grant this special exception for their modified versions; it
 * is their choice whether to do so. The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception.
 *
 * MAME4droid is dual-licensed: Alternatively, you can license MAME4droid
 * under a MAME license, as set out in http://mamedev.org/
 */

package com.seleuco.mame4droid;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.IntBuffer;
import java.util.Arrays;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
import javax.microedition.khronos.opengles.GL11;
import javax.microedition.khronos.opengles.GL11Ext;

import android.graphics.Bitmap;
import android.opengl.GLSurfaceView.Renderer;
import android.opengl.GLUtils;
import android.util.Log;
import android.widget.Toast;

public class GLRenderer implements Renderer {
    
    protected int emuTextureId = -1;
    protected int blendTextureId = -1;
    
    final protected static int BLEND_TEXTURE_SZ = 64; 
 
	private final int[] mCrop;
	
	protected int ax = 0;
	protected int ay = 0;
	
    protected ByteBuffer byteBuffer = null;
        
    protected boolean emuTextureInit = false;
    protected boolean blendTextureInit = false;    
    protected boolean isRGB = false;
    protected boolean isAltPath = false;
   
    protected boolean smooth = false;
    
    protected Bitmap blendBmp = null; 
    
	protected MAME4droid mm = null;
	
	protected boolean warn = false;
    
	public void setMAME4droid(MAME4droid mm) 
	{
		this.mm = mm;
		if(mm==null)return;

		isRGB = mm.getPrefsHelper().isRenderRGB();
		isAltPath = mm.getPrefsHelper().isAltGLPath();
	}
	
    public GLRenderer()
    {
        mCrop = new int[4];
    }

	public void changedEmulatedSize(){
        //Log.v("mm","changedEmulatedSize "+shortBuffer+" "+Emulator.getScreenBuffer());
        if(Emulator.getScreenBuffer()==null)return;
        byteBuffer = Emulator.getScreenBuffer();
        emuTextureInit = false;
        if(isAltPath)
          blendTextureInit = false;
	}
	
	private int getP2Size(GL10 gl,int size){
		//String exts = gl.glGetString(GL10.GL_EXTENSIONS);		
		//if(exts.indexOf("GL_ARB_texture_non_power_of_two")!=-1 )
		  //return size;
		if(size<=64)
			return 64;
		else if(size<=128)
			return 128;		
		else if(size<=256)
			return 256;
		else if(size<=512)
			return 512;
		else if(size<=1024)
			return 1024;
		else if(size<=2048)
			return 2048;
		else if(size<=4096)
			return 4096;
		else 
			return 8192;
	}
	
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
    	
        Log.v("mm","onSurfaceCreated ");
        
        gl.glHint(GL10.GL_PERSPECTIVE_CORRECTION_HINT, GL10.GL_FASTEST);

        gl.glClearColor(0.5f, 0.5f, 0.5f, 1);
        gl.glClear(GL10.GL_COLOR_BUFFER_BIT | GL10.GL_DEPTH_BUFFER_BIT);
        
        gl.glShadeModel(GL10.GL_FLAT);
        gl.glEnable(GL10.GL_TEXTURE_2D);
         
        
        gl.glDisable(GL10.GL_DITHER);
        gl.glDisable(GL10.GL_LIGHTING);
        //gl.glDisable(GL10.GL_BLEND);
        gl.glDisable(GL10.GL_CULL_FACE);        
        gl.glDisable(GL10.GL_DEPTH_TEST);
        gl.glDisable(GL10.GL_MULTISAMPLE);
          
        gl.glEnable(GL10.GL_BLEND);
        
        emuTextureInit=false;
        if(isAltPath)
            blendTextureInit = false;
    }
       
    public void onSurfaceChanged(GL10 gl, int w, int h) {
        Log.v("mm","sizeChanged: ==> new Viewport: ["+w+","+h+"]");

        gl.glViewport(0, 0, w, h);

        gl.glMatrixMode(GL10.GL_PROJECTION);
        gl.glLoadIdentity();
        gl.glOrthof (0f, w, h, 0f, -1f,1f);

        
        gl.glFrontFace(GL10.GL_CCW);
        
        gl.glClearColor(0.5f, 0.5f, 0.5f, 1);
        gl.glClear(GL10.GL_COLOR_BUFFER_BIT | GL10.GL_DEPTH_BUFFER_BIT);
        
        emuTextureInit=false;
        if(isAltPath)
            blendTextureInit = false;
    }
    
    protected boolean isSmooth(){
    	return Emulator.isFrameFiltering();
    }
    
	private void releaseTexture(GL10 gl) {
		if (emuTextureId != -1) {
			gl.glDeleteTextures(1, new int[] { emuTextureId }, 0);
		}		
		if (blendTextureId != -1) {
			gl.glDeleteTextures(1, new int[] { blendTextureId }, 0);
		}		
	}
	
	public void dispose(GL10 gl) {
		releaseTexture(gl);
	}	
    
    protected void createEmuTexture(final GL10 gl) {
        
    	if (gl != null) {
    		
    		if(emuTextureId==-1 || smooth!=isSmooth())
    		{
    	    	int[] mTextureNameWorkspace = new int[1];
    	    	int textureId = -1;
    	    	
    			if (emuTextureId != -1) {
    				gl.glDeleteTextures(1, new int[] { emuTextureId }, 0);
    			}
    	    	
	            gl.glGenTextures(1, mTextureNameWorkspace, 0);
	
	            textureId = mTextureNameWorkspace[0];
	            gl.glBindTexture(GL10.GL_TEXTURE_2D, textureId);
	            
	            smooth = isSmooth();
	            	
	            gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MIN_FILTER,
	            	  smooth ? GL10.GL_LINEAR : GL10.GL_NEAREST);
	            gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MAG_FILTER,
	                  smooth ? GL10.GL_LINEAR : GL10.GL_NEAREST);
	                  
	            gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_WRAP_S,GL10.GL_CLAMP_TO_EDGE);
	            gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_WRAP_T,GL10.GL_CLAMP_TO_EDGE);
	
	            gl.glTexEnvf(GL10.GL_TEXTURE_ENV, GL10.GL_TEXTURE_ENV_MODE,GL10.GL_REPLACE);
	            
	            emuTextureId = textureId;
	            emuTextureInit = false;
    		}
    		
    	    if(!emuTextureInit)
    	    {      
    	        gl.glBindTexture(GL10.GL_TEXTURE_2D, emuTextureId);
    	    	
    	      	ByteBuffer tmp = ByteBuffer.allocate(getP2Size(gl,Emulator.getEmulatedWidth()) * getP2Size(gl,Emulator.getEmulatedHeight()) * (isRGB ? 4 : 2));
    	       	byte a[] = tmp.array();
    	       	Arrays.fill(a, (byte)0);
    	        	
    	       	gl.glTexImage2D(GL10.GL_TEXTURE_2D, 0,  isRGB ? GL10.GL_RGBA : GL10.GL_RGB,
    	        			getP2Size(gl,Emulator.getEmulatedWidth()), 
    	        			getP2Size(gl,Emulator.getEmulatedHeight()), 
    	                0,  isRGB ? GL10.GL_RGBA : GL10.GL_RGB,
    	                		isRGB ? GL10.GL_UNSIGNED_BYTE : GL10.GL_UNSIGNED_SHORT_5_6_5, tmp);
    	        	        	        	
    	        emuTextureInit = true;   	    
    	    }
    		
            final int error = gl.glGetError();
            if (error != GL10.GL_NO_ERROR) {
                Log.e("GLRender", "createEmuTexture GLError: " + error);
            }    		
        }    	
    }
        
    private void createBlendTexture(GL10 gl) 
    {
    	if (gl != null) {
    		
    		if(blendTextureId == -1)
    		{
	        	int[] mTextureNameWorkspace = new int[1];
	        	int textureName = -1;
	    	
	    		gl.glGenTextures(1, mTextureNameWorkspace, 0);
	    	    
	    	    textureName = mTextureNameWorkspace[0];    	   
	    	    gl.glBindTexture(GL10.GL_TEXTURE_2D, textureName);
	    	    
	    	    gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MIN_FILTER, GL10.GL_NEAREST);
	    	    gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MAG_FILTER, GL10.GL_NEAREST);
	    	    
	    	    gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_WRAP_S, GL10.GL_REPEAT);
	    	    gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_WRAP_T, GL10.GL_REPEAT);
	
	    	    //gl.glTexEnvf(GL10.GL_TEXTURE_ENV, GL10.GL_TEXTURE_ENV_MODE, GL10.GL_MODULATE);
	    	    //gl.glTexEnvf(GL10.GL_TEXTURE_ENV, GL10.GL_TEXTURE_ENV_MODE,GL10.GL_BLEND);
	    	    gl.glTexEnvf(GL10.GL_TEXTURE_ENV, GL10.GL_TEXTURE_ENV_MODE,GL10.GL_REPLACE);
	    	    
	    	    blendTextureId = textureName;
	    	    blendTextureInit = false;
    		}
    		
    		if(!blendTextureInit)
    		{
    		    gl.glBindTexture(GL10.GL_TEXTURE_2D, blendTextureId);
    		    
	    	    blendBmp = Emulator.getFilterBitmap();
	    	        	    
	            if(!isAltPath)
	            {
		            int tw = BLEND_TEXTURE_SZ;
		            int th = BLEND_TEXTURE_SZ;
		            ByteBuffer byteBuffer = ByteBuffer.allocateDirect( tw * th * 4);
		            byteBuffer.order(ByteOrder.BIG_ENDIAN);
		            IntBuffer ib = byteBuffer.asIntBuffer();
		            int[] pixels = new int[blendBmp.getWidth() * blendBmp.getHeight()];
		            blendBmp.getPixels(pixels, 0, blendBmp.getWidth(), 0, 0, blendBmp.getWidth(), blendBmp.getHeight());
		           
		          	for(int c = 0; c < th; c++)
		           	{            		
		           	   int a = 	(c % blendBmp.getHeight()) * blendBmp.getWidth();
		           	   for(int f = 0; f < tw; f++)
		           	   {            		   
		           		   int b = f % blendBmp.getWidth() + a;
		           	       ib.put(pixels[b] << 8 | pixels[b] >>> 24);            	       
		           	   }
		           	}        	
		            gl.glTexImage2D(GL10.GL_TEXTURE_2D, 0,  GL10.GL_RGBA, tw, th, 
		                    0,  GL10.GL_RGBA , GL10.GL_UNSIGNED_BYTE , byteBuffer);
		        	           
		            ax = (tw / blendBmp.getWidth()) * blendBmp.getWidth();
		            ay = (th / blendBmp.getHeight()) * blendBmp.getHeight();
		            
		    	    mCrop[0] = 0; // u
		    	    mCrop[1] = ay; // v
		    	    mCrop[2] = ax; // w
		    	    mCrop[3] = -ay; // h
	            }
	            else
	            {
	            	GLUtils.texImage2D(GL10.GL_TEXTURE_2D, 0, blendBmp, 0);
	                int width = Emulator.getWindow_width();
	                int height = Emulator.getWindow_height();
	            	mCrop[0] = 0; // u
	        	    mCrop[1] = height; // v
	        	    mCrop[2] = width; // w
	        	    mCrop[3] = -height; // h
	            }
	            
	    	    ((GL11) gl).glTexParameteriv(GL10.GL_TEXTURE_2D,GL11Ext.GL_TEXTURE_CROP_RECT_OES, mCrop, 0);
    		}
    		
            final int error = gl.glGetError();
            if (error != GL10.GL_NO_ERROR) {
                Log.e("GLRender loadTexture2", "Texture Load GLError: " + error);
            }
        }
    }
	    
    //long target = -1;
    
    synchronized public void onDrawFrame(GL10 gl) 
    {        
        // Log.v("mm","onDrawFrame called "+shortBuffer); 
        gl.glClearColor(255, 255, 255, 1.0f);
        gl.glClear(GL10.GL_COLOR_BUFFER_BIT | GL10.GL_DEPTH_BUFFER_BIT);
    	        
        /*while(target != -1){
        	if(System.nanoTime() > target)
        		target = -1;
        }                       
        target = System.nanoTime() + 1000000000/60;*/
        
    	if(byteBuffer==null){
    		ByteBuffer buf = Emulator.getScreenBuffer();
    		if(buf==null)return;
            byteBuffer = buf;
    	}
    	             	
        byteBuffer.rewind();
        byteBuffer.order(ByteOrder.nativeOrder());
                
        try
        {     	
            createEmuTexture(gl);
        }
        catch(java.lang.OutOfMemoryError e)
        {     
        	  if(!warn)
			  mm.runOnUiThread(new Runnable() {																							
				    public void run() {
				    	Toast.makeText(mm, "Not enought memory to create texture. Try lowering resolution or disable HQx...", Toast.LENGTH_LONG).show();
				    }
			  });
        	  warn = true;
			  return;
        }
        
        gl.glBindTexture(GL10.GL_TEXTURE_2D, emuTextureId);
                      
        int width = Emulator.getEmulatedWidth();
        int height = Emulator.getEmulatedHeight();
       
        if(!isAltPath)
        {
        	gl.glTexSubImage2D(GL11.GL_TEXTURE_2D, 0, 0, 0, width, height, 
        		isRGB ? GL10.GL_RGBA : GL10.GL_RGB, isRGB ? GL10.GL_UNSIGNED_BYTE : GL10.GL_UNSIGNED_SHORT_5_6_5, byteBuffer);
        }
        else
        {

        	gl.glTexImage2D(GL10.GL_TEXTURE_2D, 0,  isRGB ? GL10.GL_RGBA : GL10.GL_RGB,
    			width, height, 0,  isRGB ? GL10.GL_RGBA : GL10.GL_RGB,
    					isRGB ? GL10.GL_UNSIGNED_BYTE : GL10.GL_UNSIGNED_SHORT_5_6_5, byteBuffer);
        }
         
	    mCrop[0] = 0; // u
	    mCrop[1] = height; // v
	    mCrop[2] = width; // w
	    mCrop[3] = -height; // h
	        
	    ((GL11) gl).glTexParameteriv(GL10.GL_TEXTURE_2D,GL11Ext.GL_TEXTURE_CROP_RECT_OES, mCrop, 0);	
	    
	    ((GL11Ext) gl).glDrawTexiOES(0, 0, 0,Emulator.getWindow_width(),Emulator.getWindow_height());
     
	    Bitmap curBlendBmp = Emulator.getFilterBitmap(); 	    
	    if(curBlendBmp!=null)	    
	    {	    	
	    	blendTextureInit = (blendBmp==curBlendBmp) && blendTextureInit;
	    			
	    	createBlendTexture(gl);
	    	
		    gl.glBlendFunc(GL10.GL_DST_COLOR, GL10.GL_ZERO);
		    gl.glBindTexture(GL10.GL_TEXTURE_2D, blendTextureId);
		    if(Emulator.isInMAME())
		    {
		    	if(!isAltPath)
		    	{
		            for(int x=0; x<Emulator.getWindow_width(); x+=ax)     
		        	    for(int y=0; y<Emulator.getWindow_height(); y+=ay)		        		
		        		   ((GL11Ext) gl).glDrawTexiOES(x, y, 0,ax,ay);
		    	}
		    	else
		    	{
		    	    ((GL11Ext) gl).glDrawTexiOES(0, 0, 0,Emulator.getWindow_width(),Emulator.getWindow_height());
		    	}
		    }
	    }
    }    
}