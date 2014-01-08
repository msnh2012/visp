/*! \example tutorial-matching-surf-homography.cpp */
#include <visp/vpDisplayOpenCV.h>
#include <visp/vpHomography.h>
#include <visp/vpKeyPointSurf.h>
#include <visp/vpPixelMeterConversion.h>
#include <visp/vpVideoReader.h>

int main(int argc, const char **argv)
{  
#if defined(VISP_HAVE_OPENCV_NONFREE) && defined(VISP_HAVE_FFMPEG)
  int method = 0;

  if (argc > 1)
    method = atoi(argv[1]);

  if (method == 0)
    std::cout << "Uses Ransac to estimate the homography" << std::endl;
  else
    std::cout << "Uses a robust scheme to estimate the homography" << std::endl;

  vpImage<unsigned char> I;

  vpVideoReader reader;
  reader.setFileName("video-postcard.mpeg");
  reader.acquire(I);

  vpKeyPointSurf surf;
  surf.buildReference(I);

  vpImage<unsigned char> Idisp;
  Idisp.resize(I.getHeight(), 2*I.getWidth());
  Idisp.insert(I, vpImagePoint(0, 0));
  Idisp.insert(I, vpImagePoint(0, I.getWidth()));

  vpDisplayOpenCV d(Idisp, 0, 0, "Homography from matched Surf keypoints") ;
  vpDisplay::display(Idisp);
  vpDisplay::flush(Idisp);

  vpImagePoint corner_ref[4];
  corner_ref[0].set_ij(115,  64);
  corner_ref[1].set_ij( 83, 253);
  corner_ref[2].set_ij(282, 307);
  corner_ref[3].set_ij(330,  72);
  for (unsigned int i=0; i<4; i++) {
    vpDisplay::displayCross(Idisp, corner_ref[i], 12, vpColor::red);
  }
  vpDisplay::flush(Idisp);

  vpCameraParameters cam(840, 840, I.getWidth()/2, I.getHeight()/2);

  vpHomography curHref;

  while ( ! reader.end() )
  {
    reader.acquire(I);
    Idisp.insert(I, vpImagePoint(0, I.getWidth()));
    vpDisplay::display(Idisp);
    vpDisplay::displayLine(Idisp, vpImagePoint(0, I.getWidth()), vpImagePoint(I.getHeight(), I.getWidth()), vpColor::white, 2);

    unsigned int nbMatch = surf.matchPoint(I);

    std::vector<vpImagePoint> iPref(nbMatch), iPcur(nbMatch); // Coordinates in pixels (for display only)
    std::vector<double> mPref_x(nbMatch), mPref_y(nbMatch);
    std::vector<double> mPcur_x(nbMatch), mPcur_y(nbMatch);
    std::vector<bool> inliers(nbMatch);

    for (unsigned int i = 0; i < nbMatch; i++) {
      vpImagePoint matched_ref, matched_cur;
      surf.getMatchedPoints(i, matched_ref, matched_cur);
      vpPixelMeterConversion::convertPoint(cam, matched_ref, mPref_x[i], mPref_y[i]);
      vpPixelMeterConversion::convertPoint(cam, matched_cur, mPcur_x[i], mPcur_y[i]);

      // Store the image coordinates in pixel of the matched points
      iPref[i] = matched_ref;
      iPcur[i] = matched_cur;
    }

    double residual;
    if (method == 0)
      vpHomography::ransac(mPref_x, mPref_y, mPcur_x, mPcur_y, curHref, inliers, residual,
                           mPref_x.size()/2, 2.0/cam.get_px(), true);
    else
      vpHomography::robust(mPref_x, mPref_y, mPcur_x, mPcur_y, curHref, inliers, residual,
                           0.4, 4, true);

    vpImagePoint corner_ref[4], corner_cur[4];
    for (int i=0; i< 4; i++) {
      corner_cur[i] = vpHomography::project(cam, curHref, corner_ref[i]);
    }

    vpImagePoint offset(0, I.getWidth());
    for (int i=0; i< 4; i++) {
      vpDisplay::displayLine(Idisp,
                             corner_cur[i]       + offset,
                             corner_cur[(i+1)%4] + offset,
                             vpColor::blue, 3);
    }

    for (unsigned int i = 0; i < nbMatch; i++) {
      if(inliers[i] == true)
        vpDisplay::displayLine(Idisp, iPref[i], iPcur[i] + offset, vpColor::green);
      else
        vpDisplay::displayLine(Idisp, iPref[i], iPcur[i] + offset, vpColor::red);
    }

    vpDisplay::flush(Idisp);

    if (vpDisplay::getClick(Idisp, false))
      break;
  }

  vpDisplay::getClick(Idisp);
#endif
  return 0;
}