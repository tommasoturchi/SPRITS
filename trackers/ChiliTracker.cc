#include <Camera.hpp>
#include <Space.hpp>

#include <algorithm>
#include <tuple>
#include <spdlog/spdlog.h>

#include <chilitags/chilitags.hpp>

#define HAS_MULTITHREADING

#define PERSISTENCE 8 // The number of frames in which a tag should be absent before being removed from the output of find(). 0 means that tags disappear directly if they are not detected.
#define GAIN 0.3f // A value between 0 and 1 corresponding to the weight of the previous (filtered) position in the new filtered position. 0 means that the latest position of the tag is returned.

using namespace SPRITS;

class ChiliTracker : public CameraObserverDecorator<std::tuple<double, double, double>>
{
    chilitags::Chilitags trackedChilitags;
	std::set<int> alive;
public:
	ChiliTracker(CameraObserver<std::tuple<double, double, double>>* component) : CameraObserverDecorator<std::tuple<double, double, double>>(component, NewFrameEvent::COLOR)
	{
	    trackedChilitags.setFilter(PERSISTENCE, GAIN);
	}
	
	void fire(const NewFrameEvent& event, const cv::Mat& frame)
	{
		if (event == NewFrameEvent::COLOR)
		{
			auto tags = trackedChilitags.find(frame, chilitags::Chilitags::ASYNC_DETECT_PERIODICALLY);
			std::set<int> tracked, notfound, newfound;
			for (const auto & tag : tags)
			{
				const cv::Mat_<cv::Point2f> corners(tag.second);
				cv::Point2f center = 0.5 * (corners(0) + corners(2));
				spc_->setElement(tag.first, std::make_tuple<double, double, double>(center.x / frame.rows, center.y / frame.cols, std::atan2(corners(1).y - corners(0).y, corners(1).x - corners(0).x)));
				tracked.insert(tag.first);
				alive.insert(tag.first);
			}
			std::set_difference(alive.begin(), alive.end(), tracked.begin(), tracked.end(), std::inserter(notfound, notfound.end()));
			for (const auto & id : notfound)
			{
				spc_->setElement(id);
				alive.erase(id);
			}
		}
	}
};
