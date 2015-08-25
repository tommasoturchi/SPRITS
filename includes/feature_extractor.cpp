/** Copyright 2011 chaitya@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/**
 * @author chaitya@gmail.com
 */
#include <feature_extractor.h>

#include <math.h>
#include <vector>
#include <cstdlib>
#include <iostream>

float FeatureExtractor::getAngle(int i, int j, int k, int w, int h) {
  int x1 = i % w;
  int y1 = i / w;
  int x2 = j % w;
  int y2 = j / w;
  int x3 = k % w;
  int y3 = k / w;
  float d12 = (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1);
  float d13 = (x3-x1)*(x3-x1) + (y3-y1)*(y3-y1);
  float d23 = (x2-x3)*(x2-x3) + (y2-y3)*(y2-y3);
  float theta = acos((d13-d12-d23)/(-2*sqrt(d12)*sqrt(d23)));
  return theta * 180 / 3.14159265;
}

int FeatureExtractor::getCentroid(int i, int j, int k, int w, int h) {
  int x1 = i % w;
  int y1 = i / w;
  int x2 = j % w;
  int y2 = j / w;
  int x3 = k % w;
  int y3 = k / w;
  int x = (x1+x2+x3)/3;
  int y = (y1+y2+y3)/3;
  return y * w + x;
}

// Processes the depth map to detect fingertips. We do most of the work in the
// back buffer |depth_mid| of size width*height*3.
// TODO (chaitya@gmail.com): Optimize the implementation. The current implementation is
// extremely inefficient.
void FeatureExtractor::Process(uint8_t *depth_mid, const int *pixelDist, const int minDist) {
  int i, r, c;
  bool prevPixelBlack = true, prevPixelHand = false, prevPixelBorder = false;

  // Initialize data structures
  for (i=0; i<307200; i++) {
    depth_mid[3*i+0] = 0;
    depth_mid[3*i+1] = 0;
    depth_mid[3*i+2] = 0;
    segBuff[i] = 0;
    borderPixels[i] = 0;
  }

  prevPixelBlack = true;
  prevPixelHand = false;
  prevPixelBorder = false;
  int borderCnt = 0;
  int lastHandIndex;
  // Traverse the depth map row-wise to mark blank, border and hand pixels
  for (i=0; i<307200; i++) {
    // Only consider pixels that are within 100 units from the closest pixel.
    if (abs(minDist - pixelDist[i]) < 100) {
      // If the previous pixel is empty, mark this pixel as a border pixel
      if (prevPixelBlack) {
        depth_mid[3*i+0] = 255;
        depth_mid[3*i+1] = 255;
        depth_mid[3*i+2] = 255;
        borderPixels[borderCnt++] = i;
        prevPixelBlack = false;
        prevPixelBorder = true;
        prevPixelHand = false;
      } else { // Otherwise mark this pixel as a hand pixel
        depth_mid[3*i+0] = 50;
        depth_mid[3*i+1] = 100;
        depth_mid[3*i+2] = 50;
        prevPixelBlack = false;
        prevPixelBorder = false;
        prevPixelHand = true;
        lastHandIndex = i;
      }
    } else {
      if (prevPixelHand) {
        // If previous pixel was a hand pixel, mark this pixel as a border pixel
        depth_mid[3*lastHandIndex+0] = 255;
        depth_mid[3*lastHandIndex+1] = 255;
        depth_mid[3*lastHandIndex+2] = 255;
        borderPixels[borderCnt++] = lastHandIndex;
      }
      prevPixelBlack = true;
      prevPixelHand = false;
      prevPixelBorder = false;
    }
  }

  prevPixelBlack = true;
  prevPixelHand = false;
  prevPixelBorder = false;
  // Traverse the depth map column-wise to mark blank, border and hand pixels
  for (int col = 0; col < width; col++) {
    for (int row = 0; row < height; row++) {
      i = row * width + col;
      if (depth_mid[3*i+0] != 0 && depth_mid[3*i+0] != 255) { // hand
        if (prevPixelBlack) {
          depth_mid[3*i+0] = 255;
          depth_mid[3*i+1] = 255;
          depth_mid[3*i+2] = 255;
          borderPixels[borderCnt++] = i;
          prevPixelBlack = false;
          prevPixelBorder = true;
          prevPixelHand = false;
        } else {
          prevPixelHand = true;
          prevPixelBlack = false;
          prevPixelBorder = false;
          lastHandIndex = i;
        }
      } else if (depth_mid[3*i+0] == 255) { // border
        prevPixelBlack = false;
        prevPixelBorder = true;
        prevPixelHand = false;
      } else if (depth_mid[3*i+0] == 0) { // blank
        if (prevPixelHand) {
          depth_mid[3*lastHandIndex+0] = 255;
          depth_mid[3*lastHandIndex+1] = 255;
          depth_mid[3*lastHandIndex+2] = 255;
          borderPixels[borderCnt++] = lastHandIndex;
        }
        prevPixelBlack = true;
        prevPixelBorder = false;
        prevPixelHand = false;
      }
    }
  }

  // Isolate true border pixels by removing false border pixels
  for (int i=0; i<307200; i++) {
    if (depth_mid[3*i+0] == 255) {
      r = i / width;
      c = i % width;
      bool touchingHand = false, touchingBlack = false;
      for (int cc = c-1; cc <= c+1; cc++) {
        for (int rr = r-1; rr <= r+1; rr++) {
          int ii = rr * width + cc;
          if (ii >= 0 && ii < 307200 &&
            depth_mid[3*ii+0] != 0 && depth_mid[3*ii+0] != 255) {
            // pixel <rr,cc> is adjacent to a hand pixel
            touchingHand = true;
          }
          if (ii >= 0 && ii < 307200 && depth_mid[3*ii+0] == 0) {
            // pixel <rr,cc> is adjacent to a blank pixel
            touchingBlack = true;
          }
          if (touchingHand && touchingBlack) break;
        }
        if (touchingHand && touchingBlack) break;
      }
      if (!touchingBlack || !touchingHand) {
        // Mark <rr,cc> as blank if it not adjacent to both blank and hand pixels
        depth_mid[3*i+0] = 0;
        depth_mid[3*i+1] = 0;
        depth_mid[3*i+2] = 0;
      }
    }
  }

  std::vector<int> contour;
  // Compute the border contour
  for (i=0; i<307200; i++) {
    if (depth_mid[3*i+0] == 255 && depth_mid[3*i+1] == 255 && depth_mid[3*i+2] == 255) {
      // Find contour containing pixel i
      contour.push_back(i);
      segBuff[i] = 1;
      for (int dir = 0; dir < 2; dir++) {
        int curr = i;
        while (true) {
          r = curr / width;
          c = curr % width;
          bool done = false;

          int ii = r * width + (c-1);
          if (ii < 0 || ii >= 307200 || segBuff[ii] == 1 || depth_mid[3*ii+0] != 255) {
            ii = r * width + (c+1);
            if (ii < 0 || ii >= 307200 || segBuff[ii] == 1 || depth_mid[3*ii+0] != 255) {
              ii = (r-1) * width + c;
              if (ii < 0 || ii >= 307200 || segBuff[ii] == 1 || depth_mid[3*ii+0] != 255) {
                ii = (r+1) * width + c;
                if (ii < 0 || ii >= 307200 || segBuff[ii] == 1 || depth_mid[3*ii+0] != 255) {
                  ii = (r-1) * width + (c-1);
                  if (ii < 0 || ii >= 307200 || segBuff[ii] == 1 || depth_mid[3*ii+0] != 255) {
                    ii = (r-1) * width + (c+1);
                    if (ii < 0 || ii >= 307200 || segBuff[ii] == 1 || depth_mid[3*ii+0] != 255) {
                      ii = (r+1) * width + (c-1);
                      if (ii < 0 || ii >= 307200 || segBuff[ii] == 1 || depth_mid[3*ii+0] != 255) {
                        ii = (r+1) * width + (c+1);
                        if (ii < 0 || ii >= 307200 || segBuff[ii] == 1 || depth_mid[3*ii+0] != 255) {
                          done = true;
                        }
                      }
                    }
                  }
                }
              }
            }
          }
          if (done) break;
          if (dir == 0)
            contour.push_back(ii);
          else
            contour.insert(contour.begin(), ii);
          segBuff[ii] = 1;
          curr = ii;
        }
      }
      break;
    }
  }

  std::vector<int>::iterator it1 = contour.begin();
  // Append first 50 contour points at the end of the contour
  for (int counter = 0; it1 != contour.end() && counter < 50; ++it1, ++counter) {
    extContour[counter] = *it1;
  }
  for (int counter = 0; counter < 50; counter++) {
    contour.push_back(extContour[counter]);
  }

  // The value of K in the K-curvature algorithm
  int curv = 10;
  // Number of fingertip
  int tipNum = 0;
  // Detect fingertips as points on the contour, using K-curvature algorithm.
  for (std::vector<int>::iterator it = contour.begin(); it != contour.end(); ++it) {
    int i = *it;
    std::vector<int>::iterator it1 = it;
    for (int counter = 0; it1 != contour.end() && counter < curv; ++it1, ++counter);
    if (it1 != contour.end()) {
      int j = *it1;
      std::vector<int>::iterator it2 = it1;
      for (int counter = 0;it2 != contour.end() && counter < curv; ++it2, ++counter);
      if (it2 != contour.end()) {
        int k = *it2;
        float angle = getAngle(i, j, k, width, height);
        if (angle < 30) {
          int center = getCentroid(i, j, k, width, height);
          if (depth_mid[3*center+0] != 255 && depth_mid[3*center+0] != 0) {
            // Fingertip is valid only if center pixel is a hand pixel.
            if (tipNum < 10) {
              fingerVectors[tipNum].start = ((i/width + k/width) / 2) * width + (i%width + k%width) / 2;
              fingerVectors[tipNum].end = j;
              tipNum++;
            }
            for (int counter = 0;it != contour.end() && counter < 20; ++it, ++counter);
          }
        }
      }
    }
  }
  numFingerTips = tipNum;

  // Draw the GUI markers for fingertips.
  for (tipNum = 0; tipNum < numFingerTips; tipNum++) {
    r = fingerVectors[tipNum].end / width;
    c = fingerVectors[tipNum].end % width;
    for (int a = c-5; a < c+5; a++) {
      for (int b = r-5; b < r+5; b++) {
        if (a >= 0 && a < width && b >= 0 && b < height) {
          int t = b * width + a;
          depth_mid[3*t+0] = 255;
          depth_mid[3*t+1] = 0;
          depth_mid[3*t+2] = 0;
        }
      }
    }
  }
}
