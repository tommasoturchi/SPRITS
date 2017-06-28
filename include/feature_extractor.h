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

#ifndef FEATURE_EXTRACTOR_H_
#define FEATURE_EXTRACTOR_H_

#include <libfreenect.h>

/**
 * This class detects the fingertips from the depth map obtained from the
 * Kinect sensor.
 *
 * @author chaitya@gmail.com
 */
class FeatureExtractor {
  public:
  // A struct holding a vector segment.
  struct VectorSegment {
  public:
    VectorSegment() {}
    // Starting pixel of the segment
    int start;
    // Ending pixel of the segment
    int end;
  };

  // Create a FeatureExtractor object for extracting fingertip features
  // from a depth map of dimensions |w| x |h|.
  FeatureExtractor(int w, int h)
    : width(w),
      height(h) {}
  ~FeatureExtractor() {}

  // Processes the depth map to detect fingertips. We do most of the work in the
  // back buffer |depth_mid| of size width*height*3.
  // @param {uint8_t*} depth_mid The depth map
  // @param {int*} pixelDist The absolute pixel distance from the camera
  // @param {int} The distance of th closest pixel from the camera
  void Process(uint8_t *depth_mid, const int *pixelDist, const int minDist);

  // Returns the vectors for the fingertips detected.
  VectorSegment* GetFingerVectors() { return fingerVectors; }

  // Returns the number of fingertips detected.
  int GetNumFingerTips() { return numFingerTips; }

  private:
  float getAngle(int i, int j, int k, int w, int h);
  int getCentroid(int i, int j, int k, int w, int h);

  int pixelDist[307200];
  short segBuff[307200];
  short borderPixels[307200];
  int extContour[50];
  int width, height;
  int numFingerTips;
  VectorSegment fingerVectors[10];
};

#endif  //FEATURE_EXTRACTOR_H_
