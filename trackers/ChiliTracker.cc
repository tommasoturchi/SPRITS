#include <Camera.hpp>
#include <Manifold.hpp>

#include <algorithm>
#include <tuple>

#include <chilitags/chilitags.hpp>

using namespace SPRITS;

class ChiliTracker : public CameraObserverDecorator<std::tuple<double, double, double>>
{
    chilitags::Chilitags trackedChilitags;
	bool tracking = false;
	std::set<int> alive;
public:
	ChiliTracker(CameraObserver<std::tuple<double, double, double>>* component) : CameraObserverDecorator<std::tuple<double, double, double>>(component, NewFrameEvent::COLOR)
	{
	    trackedChilitags.setFilter(0, 0.0f);
	}
	
	void fire(const NewFrameEvent& event, const cv::Mat& frame)
	{
		auto tags = trackedChilitags.find(frame, tracking?chilitags::Chilitags::TRACK_ONLY:chilitags::Chilitags::TRACK_AND_DETECT);
		std::set<int> stillalive, dead, newborns;
		for (const auto & tag : tags)
		{
			const cv::Mat_<cv::Point2f> corners(tag.second);
			cv::Point2f center = 0.5 * (corners(0) + corners(2));
			man_->setElement(tag.first, std::make_tuple<double, double, double>(center.x / frame.rows, center.y / frame.cols, std::atan2(corners(1).y - corners(0).y, corners(1).x - corners(0).x)));
			stillalive.insert(tag.first);
		}
		std::set_difference(alive.begin(), alive.end(), stillalive.begin(), stillalive.end(), std::inserter(dead, dead.end()));
		std::set_difference(stillalive.begin(), stillalive.end(), alive.begin(), alive.end(), std::inserter(newborns, newborns.end()));
		for (const auto & id : dead)
		{
			man_->setElement(id);
			alive.erase(id);
		}
		for (const auto & id : newborns)
			alive.insert(id);
		tracking = !tags.empty();
	}
};
