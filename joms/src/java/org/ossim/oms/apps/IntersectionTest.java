//-----------------------------------------------------------------------------
// File:  ChipperTest.java
//
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
// Usage: java -Djava.library.path=<path to libjoms.so> -cp <jar> org.ossim.jni.test.IntersectionTest
//
// Example:
// $ java -Djava.library.path=$HOME/code/osgeo_ossim/trunk/ossimjni/java/build/lib -cp $HOME/code/osgeo_ossim/trunk/ossimjni/java/build/lib/ossimjni.jar org.ossim.jni.test.ChipperTest
// 
//-----------------------------------------------------------------------------
//
package org.ossim.oms.apps;

import joms.oms.Chipper;
import joms.oms.ImageData;
import joms.oms.Init;
import joms.oms.Util;
import joms.oms.Keywordlist;
import joms.oms.KeywordlistIterator;
import joms.oms.StringPair;

import java.io.File;

public class IntersectionTest
{
 
   /**
    * @param args
    */
   public static void main( String[] args )
   {
      // Copy the args with app name for c++ initialize.
      String[] newArgs = new String[args.length + 1];
      newArgs[0] = "org.ossim.oms.apps.IntersectionTest";
      System.arraycopy(args, 0, newArgs, 1, args.length);

      // Initialize ossim stuff:
      int argc = Init.instance().initialize(newArgs.length, newArgs);

      try
      {
         double ecefVector[]   = new double[3];
         double ecefPt[]       = new double[3];
         double latLon[]       = new double[2];
         double latLonHeight[]  = new double[3];
         double ecefResultPt[] = new double[3];
         
         ecefVector[0] = -1.0;
         ecefVector[1] = 0.0;
         ecefVector[2] = 0.0;

         ecefPt[0] = 100000000.0;
         ecefPt[1] = 0.0;
         ecefPt[2] = 0.0;

         System.out.println("*********Intersection Test*********");


         if(Util.intersectWgs84EllipsoidToLatLon(latLon, ecefPt, ecefVector))
         {
            System.out.println("Intersected Ellipsoid!");

            System.out.printf("ECEF Point: %f, %f, %f\n",
                              ecefPt[0], ecefPt[1], ecefPt[2]);
            System.out.printf("ECEF Vector: %f, %f, %f\n",
                              ecefVector[0], ecefVector[1], ecefVector[2]);
            System.out.printf("Lat Lon Intersection: %f, %f\n",
                              latLon[0], latLon[1]);
         }
         if(Util.intersectWgs84Ellipsoid(ecefResultPt, ecefPt, ecefVector))
         {
            System.out.println("Intersected Ellipsoid!");

            System.out.printf("ECEF Point: %f, %f, %f\n",
                              ecefPt[0], ecefPt[1], ecefPt[2]);
            System.out.printf("ECEF Vector: %f, %f, %f\n",
                              ecefVector[0], ecefVector[1], ecefVector[2]);
            System.out.printf("ECEF Intersection: %f, %f\n",
                              ecefResultPt[0], ecefResultPt[1], ecefResultPt[2]);
         }
         if(Util.intersectElevationToLatLonHeight(latLonHeight, ecefPt, ecefVector))
         {
            System.out.println("Intersected Elevation!");

            System.out.printf("ECEF Point: %f, %f, %f\n",
                              ecefPt[0], ecefPt[1], ecefPt[2]);
            System.out.printf("ECEF Vector: %f, %f, %f\n",
                              ecefVector[0], ecefVector[1], ecefVector[2]);
            System.out.printf("Lat Lon Height Intersection: %f, %f, %f\n",
                              latLonHeight[0], latLonHeight[1], latLonHeight[2]);         }
      }
      catch( Exception e )
      {
         System.err.println("Caught Exception: " + e.getMessage());
      }

      
   } // End of main:
   
} // Matches: public class ChipperTest
