package org.ossim.oms.util;

import java.awt.image.*;
import java.awt.*;
import java.awt.geom.Rectangle2D;
import javax.imageio.ImageTypeSpecifier;

public class ImageGenerator {

  Image createErrorImage(int w, int h, String message) 
  {
    BufferedImage errorImage = new BufferedImage(w, h, BufferedImage.TYPE_INT_RGB);
    Graphics2D g = (Graphics2D)errorImage.getGraphics();
    FontMetrics fontMetrics= g.getFontMetrics();

    //float fontHeight = fontMetrics.height;
    String[] stringArray = message.split("\n");
    float originY = 0.0f;
    float originX = 0.0f;
    g.setPaint(Color.white);
    g.setStroke(new BasicStroke(1));

    for ( int i = 0; i < stringArray.length; i++ )
    {
        Rectangle2D bounds = fontMetrics.getStringBounds(stringArray[i], g);
        originY+=bounds.getHeight();
        originX = (float)(w/2.0f - (bounds.getWidth()/2.0f));
        if(originX < 0) originX = 0;
        g.drawString(stringArray[i], originX, originY);
    }
    //g.setPaint(Color.red);
    //g.setStroke(new BasicStroke(4));
    //g.drawLine(0, 0, w, h);
    //g.drawLine(w, 0, 0, h);

    return errorImage;
  }

  static BufferedImage convertRGBAToIndexed(BufferedImage src)
  {
    BufferedImage dest = new BufferedImage(src.getWidth(), src.getHeight(), BufferedImage.TYPE_BYTE_INDEXED);
    Graphics g = dest.getGraphics();
    g.setColor(new Color(231, 20, 189));
    g.fillRect(0, 0, dest.getWidth(), dest.getHeight()); //fill with a hideous color and make it transparent
    dest = makeTransparent(dest, 0, 0);
    dest.createGraphics().drawImage(src, 0, 0, null);
    return dest;
  }

  static BufferedImage makeTransparent(BufferedImage image, int x, int y) 
  {
    ColorModel cm = image.getColorModel();
    if (!(cm instanceof IndexColorModel))
      return image; //sorry...
    IndexColorModel icm = (IndexColorModel) cm;
    WritableRaster raster = image.getRaster();
    int pixel = raster.getSample(x, y, 0); //pixel is offset in ICM's palette
    int size = icm.getMapSize();
    byte[] reds = new byte[size];
    byte[] greens = new byte[size];
    byte[] blues = new byte[size];
    icm.getReds(reds);
    icm.getGreens(greens);
    icm.getBlues(blues);
    IndexColorModel icm2 = new IndexColorModel(8, size, reds, greens, blues, pixel);
    return new BufferedImage(icm2, raster, image.isAlphaPremultiplied(), null);
  }
  static BufferedImage optimizeRaster(Raster image, ColorModel colorModel, def hints)//String mimeType, Boolean transparentFlag)
  {
    BufferedImage result
    String mimeTypeTest = hints.type?.toLowerCase()
    Boolean transparentFlag = hints.transparent
    if(transparentFlag == null) transparentFlag = false

    if(mimeTypeTest?.contains("jpeg"))
    {
      transparentFlag = false
    }
    if ( image.numBands == 1 )
    {
      result = ImageGenerator.convertToColorIndexModel( image.dataBuffer,
               image.width,
               image.height,
               transparentFlag )
    }
    else
    {
      Boolean isRasterPremultiplied = true
      Hashtable<?, ?> properties = null
      result = new BufferedImage(
              colorModel,
              image,
              isRasterPremultiplied,
              properties
      )
      if ( image.numBands == 3 )
      {
        if ( transparentFlag )
        {
          result = TransparentFilter.fixTransparency( new TransparentFilter(), result )
        }
        if ( mimeTypeTest?.contains( "gif" ) )
        {
          result = ImageGenerator.convertRGBAToIndexed( result )
        }
      }
    }
    result
  }
  // NEED To figure out the JAI Image  dependency.  Can't get it to 
  // work.  Will comment out for now
/*
  static BufferedImage renderedImageToBufferedImage(RenderedImage renderedImage, HashMap hints = [:])
  {
    BufferedImage image = null
    SampleModel sampleModel = renderedImage.sampleModel
    ColorModel  colorModel  = renderedImage.colorModel
    Raster      raster      = renderedImage.data
    Boolean keepBands = hints?.keepBands
    if(hints.keepBands)
    {
      if(!hints.type.contains("tiff"))
      {
        // The only type we will support raw band output is TIFF.
        // this way we can send back the raw tiff without modification
        //
        keepBands = false;
      }
    }
    if ( raster )
    {
      if ( (!hints.keepBands) && (raster.numBands > 3 ))
      {
        def planarImage = JaiImage.bufferedToPlanar( new BufferedImage( colorModel, raster, true, null ) )
        planarImage.data
        def modifiedImage = JaiImage.selectBandsForRendering( planarImage )

        if ( modifiedImage )
        {
          raster = modifiedImage.data
          colorModel = modifiedImage.colorModel
        }
      }

      try
      {
        image = ImageGenerator.optimizeRaster( raster, colorModel, hints )
      }
      catch ( e )
      {
        e.printStackTrace()
      }
    }
    image 
  }
 */ 
  static RenderedImage convertToColorIndexModel( DataBuffer dataBuffer, int width, int height, boolean transparentFlag )
  {
      ImageTypeSpecifier isp = ImageTypeSpecifier.createGrayscale( 8, DataBuffer.TYPE_BYTE, false );
      ColorModel colorModel;
      SampleModel sampleModel = isp.getSampleModel( width, height );
      if ( !transparentFlag )
      {
        colorModel = isp.getColorModel();
      }
      else
      {
        int[] lut = new int[256];
        for ( int i = 0; i  < lut.length; i++ ) {
          lut[i] = ( ( 0xff << 24 ) | ( i << 16 ) | ( i << 8 ) | ( i ) );
        }
        lut[0] = 0xff000000;
        colorModel = new IndexColorModel( 8, lut.length, lut, 0, true, 0, DataBuffer.TYPE_BYTE );
      }
      WritableRaster raster = WritableRaster.createWritableRaster( sampleModel, dataBuffer, null );
      return new BufferedImage( colorModel, raster, false, null );
  }
}