#include <Camera.hpp>
#include <Space.hpp>

#include <feature_extractor.h>
#include <spdlog/spdlog.h>

using namespace SPRITS;

class FingerTracker : public CameraObserverDecorator<std::tuple<double, double, double>>
{
    FeatureExtractor *feature_extractor;
	uint16_t t_gamma[2048];
	uint8_t *depth_mid;
public:
	FingerTracker(CameraObserver<std::tuple<double, double, double>>* component) : CameraObserverDecorator<std::tuple<double, double, double>>(component, NewFrameEvent::DEPTH)
	{
		feature_extractor = new FeatureExtractor(640, 480);
		
	    for (int i = 0; i < 2048; ++i) {
	      float v = i/2048.0;
	      v = powf(v, 3)* 6;
	      t_gamma[i] = v*6*256;
	    }
		depth_mid = (uint8_t*)malloc(640*480*3);
	}
	
	void fire(const NewFrameEvent& event, const cv::Mat& frame)
	{
		if (event == NewFrameEvent::DEPTH)
		{
			int minDist = 255 * 5;
			int* pixelDist = new int[307200];
			
			uint16_t *depth = (uint16_t*)frame.data;
			
			for (int i = 0; i < 307200; ++i) {
				int pval = t_gamma[depth[i]];
				int lb = pval & 0xff;
				int mult = pval >> 8;
				int distance = 255 * mult + lb;
				
				pixelDist[i] = distance;
				
				if (distance < minDist)
					minDist = distance;
			}
			
			depth_mid = (uint8_t*)frame.data;
			feature_extractor->Process(depth_mid, pixelDist, minDist);
			
			spdlog::get("console")->debug("{} total fingers detected.", feature_extractor->GetNumFingerTips());
		}
	}
};
