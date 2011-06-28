//  gracing - a idiot (but physically powered) racing game 
//  Copyright (C) 2010 gianni masullo
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#include "util.hh"


#define ULCORNER_X(r) r.UpperLeftCorner.X
#define ULCORNER_Y(r) r.UpperLeftCorner.Y

#define LLCORNER_X(r) r.UpperLeftCorner.X
#define LLCORNER_Y(r) r.LowerRightCorner.Y

#define URCORNER_X(r) r.LowerRightCorner.X
#define URCORNER_Y(r) r.UpperLeftCorner.Y

#define LRCORNER_X(r) r.LowerRightCorner.X
#define LRCORNER_Y(r) r.LowerRightCorner.Y


void Util::drawRectWithBackgroung(irr::video::IVideoDriver *driver, 
    irr::video::ITexture* background , 
    irr::core::rect<irr::s32> dstRect, 
    bool useAlphaChannel,
    unsigned modeHoriz,
    unsigned modeVert)
{
   irr::video::SMaterial material; 
   // Store and clear the projection matrix 
   irr::core::matrix4 oldProjMat = driver->getTransform(irr::video::ETS_PROJECTION); 
   driver->setTransform(irr::video::ETS_PROJECTION,irr::core::matrix4()); 
   // Store and clear the view matrix 
   irr::core::matrix4 oldViewMat = driver->getTransform(irr::video::ETS_VIEW); 
   driver->setTransform(irr::video::ETS_VIEW,irr::core::matrix4()); 
   // Store and clear the world matrix 
   irr::core::matrix4 oldWorldMat = driver->getTransform(irr::video::ETS_WORLD); 
   driver->setTransform(irr::video::ETS_WORLD, irr::core::matrix4()); 

   // Find the positions of corners 
   irr::core::vector2df corner[4]; 
   corner[0] = irr::core::vector2df(ULCORNER_X(dstRect),ULCORNER_Y(dstRect));
   corner[1] = irr::core::vector2df(URCORNER_X(dstRect),URCORNER_Y(dstRect));
   corner[2] = irr::core::vector2df(LLCORNER_X(dstRect),LLCORNER_Y(dstRect));
   corner[3] = irr::core::vector2df(LRCORNER_X(dstRect),LRCORNER_Y(dstRect));

   // Find the uv coordinates of the sourceRect 
   double ampX=dstRect.getSize().Width / (float)background->getSize().Width;
   double ampY=dstRect.getSize().Height / (float)background->getSize().Height;
   irr::core::vector2df uvCorner[4]; 
   uvCorner[0] = irr::core::vector2df(0.,0.); //irr::core::vector2df(sourceRect.UpperLeftCorner.X,sourceRect.UpperLeftCorner.Y); 
   uvCorner[1] = irr::core::vector2df(ampX,0.);  //irr::core::vector2df(sourceRect.LowerRightCorner.X,sourceRect.UpperLeftCorner.Y); 
   uvCorner[2] = irr::core::vector2df(0.,ampY); //irr::core::vector2df(sourceRect.UpperLeftCorner.X,sourceRect.LowerRightCorner.Y); 
   uvCorner[3] = irr::core::vector2df(ampX,ampY); //irr::core::vector2df(sourceRect.LowerRightCorner.X,sourceRect.LowerRightCorner.Y); 

   irr::video::S3DVertex vertices[4]; 
   irr::u16 indices[6] = { 0, 1, 2, 3 ,2 ,1 }; 

   irr::video::SColor color(255,255,255,255);
   //bool useAlphaChannel=false;

   // Convert pixels to world coordinates 
   float screenWidth = driver->getScreenSize().Width; 
   float screenHeight = driver->getScreenSize().Height; 
   for (int x = 0; x < 4; x++) { 
      float screenPosX = ((corner[x].X/screenWidth)-0.5f)*2.0f; 
      float screenPosY = ((corner[x].Y/screenHeight)-0.5f)*-2.0f; 
      vertices[x].Pos = irr::core::vector3df(screenPosX,screenPosY,1); 
      vertices[x].TCoords = uvCorner[x]; 
      vertices[x].Color = color; 
   } 

   material.Lighting = false; 
   material.ZWriteEnable = false; 
   material.ZBuffer = false; 
   material.TextureLayer[0].Texture = background; 
   material.MaterialTypeParam = irr::video::pack_texureBlendFunc(irr::video::EBF_SRC_ALPHA, 
       irr::video::EBF_ONE_MINUS_SRC_ALPHA, 
       irr::video::EMFN_MODULATE_1X, 
       irr::video::EAS_TEXTURE | irr::video::EAS_VERTEX_COLOR); 

   if (useAlphaChannel) 
      material.MaterialType = irr::video::EMT_ONETEXTURE_BLEND; 
   else 
      material.MaterialType = irr::video::EMT_SOLID; 

   driver->setMaterial(material); 
   driver->drawIndexedTriangleList(&vertices[0],4,&indices[0],2); 

   // Restore projection and view matrices 
   driver->setTransform(irr::video::ETS_PROJECTION,oldProjMat); 
   driver->setTransform(irr::video::ETS_VIEW,oldViewMat); 
   driver->setTransform(irr::video::ETS_WORLD,oldWorldMat); 
}

void Util::draw2DImage_v2(irr::video::IVideoDriver *driver, 
    irr::video::ITexture* texture , 
    irr::core::rect<irr::s32> sourceRect, 
    irr::core::position2d<irr::s32> position, 
    irr::core::position2d<irr::s32> rotationPoint, 
    irr::f32 rotation, 
    irr::core::vector2df scale, 
    bool useAlphaChannel, 
    irr::video::SColor color)
{ 

   irr::video::SMaterial material; 

   // Store and clear the projection matrix 
   irr::core::matrix4 oldProjMat = driver->getTransform(irr::video::ETS_PROJECTION); 
   driver->setTransform(irr::video::ETS_PROJECTION,irr::core::matrix4()); 

   // Store and clear the view matrix 
   irr::core::matrix4 oldViewMat = driver->getTransform(irr::video::ETS_VIEW); 
   driver->setTransform(irr::video::ETS_VIEW,irr::core::matrix4()); 

   // Store and clear the world matrix 
   irr::core::matrix4 oldWorldMat = driver->getTransform(irr::video::ETS_WORLD); 
   driver->setTransform(irr::video::ETS_WORLD, irr::core::matrix4()); 

   // Find the positions of corners 
   irr::core::vector2df corner[4]; 

   corner[0] = irr::core::vector2df(position.X,position.Y); 
   corner[1] = irr::core::vector2df(position.X+sourceRect.getWidth()*scale.X,position.Y); 
   corner[2] = irr::core::vector2df(position.X,position.Y+sourceRect.getHeight()*scale.Y); 
   corner[3] = irr::core::vector2df(position.X+sourceRect.getWidth()*scale.X,position.Y+sourceRect.getHeight()*scale.Y); 

   // Rotate corners 
   if (rotation != 0.0f) 
      for (int x = 0; x < 4; x++) 
         corner[x].rotateBy(rotation,irr::core::vector2df(rotationPoint.X, rotationPoint.Y)); 


   // Find the uv coordinates of the sourceRect 
   irr::core::vector2df uvCorner[4]; 
   uvCorner[0] = irr::core::vector2df(sourceRect.UpperLeftCorner.X,sourceRect.UpperLeftCorner.Y); 
   uvCorner[1] = irr::core::vector2df(sourceRect.LowerRightCorner.X,sourceRect.UpperLeftCorner.Y); 
   uvCorner[2] = irr::core::vector2df(sourceRect.UpperLeftCorner.X,sourceRect.LowerRightCorner.Y); 
   uvCorner[3] = irr::core::vector2df(sourceRect.LowerRightCorner.X,sourceRect.LowerRightCorner.Y); 
   for (int x = 0; x < 4; x++) { 
      float uvX = uvCorner[x].X/(float)texture->getSize().Width; 
      float uvY = uvCorner[x].Y/(float)texture->getSize().Height; 
      uvCorner[x] = irr::core::vector2df(uvX,uvY); 
   } 

   // Vertices for the image 
   irr::video::S3DVertex vertices[4]; 
   irr::u16 indices[6] = { 0, 1, 2, 3 ,2 ,1 }; 

   // Convert pixels to world coordinates 
   float screenWidth = driver->getScreenSize().Width; 
   float screenHeight = driver->getScreenSize().Height; 
   for (int x = 0; x < 4; x++) { 
      float screenPosX = ((corner[x].X/screenWidth)-0.5f)*2.0f; 
      float screenPosY = ((corner[x].Y/screenHeight)-0.5f)*-2.0f; 
      vertices[x].Pos = irr::core::vector3df(screenPosX,screenPosY,1); 
      vertices[x].TCoords = uvCorner[x]; 
      vertices[x].Color = color; 
   } 

   material.Lighting = false; 
   material.ZWriteEnable = false; 
   material.ZBuffer = false; 
   material.TextureLayer[0].Texture = texture; 
   material.MaterialTypeParam = irr::video::pack_texureBlendFunc(irr::video::EBF_SRC_ALPHA, 
       irr::video::EBF_ONE_MINUS_SRC_ALPHA, 
       irr::video::EMFN_MODULATE_1X, 
       irr::video::EAS_TEXTURE | irr::video::EAS_VERTEX_COLOR); 

   if (useAlphaChannel) 
      material.MaterialType = irr::video::EMT_ONETEXTURE_BLEND; 
   else 
      material.MaterialType = irr::video::EMT_SOLID; 

   driver->setMaterial(material); 
   driver->drawIndexedTriangleList(&vertices[0],4,&indices[0],2); 

   // Restore projection and view matrices 
   driver->setTransform(irr::video::ETS_PROJECTION,oldProjMat); 
   driver->setTransform(irr::video::ETS_VIEW,oldViewMat); 
   driver->setTransform(irr::video::ETS_WORLD,oldWorldMat); 
}
