#ifndef PLANE_CC
#define PLANE_CC

#include <tuple>
#include <spdlog/spdlog.h>

#include <Space.hpp>

using namespace SPRITS;

class Plane : public Space<std::tuple<double, double, double>>
{
private:
	std::map<int, std::tuple<double, double, double>> points_;
public:
	void setElement(int id)
	{
		points_.erase(points_.find(id));
		notify(ElementEvent(REMOVE), id);
	}
	void setElement(int id, std::tuple<double, double, double> point)
	{
		if (points_.find(id) == points_.end())
		{
			points_[id] = point;
			notify(ElementEvent(ADD), id);
		} else
		{
			points_[id] = point;
			notify(ElementEvent(UPDATE), id);
		}
	}
	
	std::tuple<double, double, double> getElement(int id)
	{
		return points_[id];
	}
};

#endif