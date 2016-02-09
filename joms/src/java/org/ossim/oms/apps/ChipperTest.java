//-----------------------------------------------------------------------------
// File:  ChipperTest.java
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description: Java test/example app for ossimjni Info class.
//
// Usage: java -Djava.library.path=<path_to_libossimjni-swig> -cp <path_to_ossimjni.jar> org.ossim.jni.test.ChipperTest <image>
//
// Example:
// $ java -Djava.library.path=$HOME/code/osgeo_ossim/trunk/ossimjni/java/build/lib -cp $HOME/code/osgeo_ossim/trunk/ossimjni/java/build/lib/ossimjni.jar org.ossim.jni.test.ChipperTest
// 
//-----------------------------------------------------------------------------
// $Id: ChipperTest.java 22320 2013-07-19 15:21:03Z dburken $

package org.ossim.oms.apps;

import joms.oms.Chipper;
import joms.oms.ImageData;
import joms.oms.Init;
import joms.oms.Keywordlist;
import joms.oms.KeywordlistIterator;
import joms.oms.StringPair;

import java.io.File;

public class ChipperTest
{
   static
   {
      System.loadLibrary( "ossimjni-swig" );
   }
   
   /**
    * @param args
    */
   public static void main( String[] args )
   {
      // Copy the args with app name for c++ initialize.
      String[] newArgs = new String[args.length + 1];
      newArgs[0] = "org.ossim.oms.apps.ChipperTest";
      System.arraycopy(args, 0, newArgs, 1, args.length);

      // Initialize ossim stuff:
      int argc = Init.instance().initialize(newArgs.length, newArgs);

      if ( argc == 3 )
      {
         try
         {
            String outputFile = newArgs[1];
            String optionsFile = newArgs[2];
            System.out.println( "outputFile: " + outputFile );
            System.out.println( "optionsFile: " + optionsFile );
            
            joms.oms.Chipper chipper = new joms.oms.Chipper();
            joms.oms.ImageData chip  = new joms.oms.ImageData();
            joms.oms.Keywordlist kwl = new joms.oms.Keywordlist();

            kwl.addFile( optionsFile );

            if ( chipper.initialize( kwl ) )
            {
            	String str;
            	str = kwl.findKey("cut_width");
            	int width = Integer.parseInt(str);
            	str = kwl.findKey("cut_height");
            	int height = Integer.parseInt(str);
            	int numbands = 3;
                byte[] buffer = new byte[width*height*numbands];
               
               //---
               // Return status:
               // OSSIM_STATUS_UNKNOWN = 0,
               // OSSIM_NULL           = 1, not initialized
               // OSSIM_EMPTY          = 2, initialized but blank or empty
               // OSSIM_PARTIAL        = 3, contains some null/invalid values
               // OSSIM_FULL           = 4  all valid data
               //---
               int status = chipper.getChip( buffer, false );
               if ( ( status == 3 ) || ( status == 4 ) )
               {
                  System.out.println( "No problems" );
               }
               else
               {
                  System.err.println("Chipper::getChip failed!");
                  String statusString = null;
                  if ( status == 0)
                  {
                     statusString = "unknown";
                  }
                  else if ( status == 1 )
                  {
                     statusString = "null";
                  }
                  else if ( status == 2 )
                  {
                     statusString = "empty";
                  }
                  System.err.println("Return status: " + statusString);
               }
            }
            else
            {
               System.err.println("Chipper::initialize failed!");
            }
         }
         catch( Exception e )
         {
            System.err.println("Caught Exception: " + e.getMessage());
         }
      }
      else
      {
         System.out.println( "Usage: org.ossim.oms.apps.ChipperTest <outputFile> <options.kwl>");
      }
      
   } // End of main:
   
} // Matches: public class ChipperTest
