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
	std::set<int> seen_;
public:
	void setElement(int id)
	{
		points_.erase(points_.find(id));
		seen_.insert(id);
		notify(ElementEvent(REMOVE), id);
	}
	void setElement(int id, std::tuple<double, double, double> point)
	{
		if (seen_.find(id) == seen_.end())
			notify(ElementEvent(UPDATE), id);
		else
		{
			seen_.erase(seen_.find(id));
			notify(ElementEvent(ADD), id);
		}
		points_[id] = point;
	}
	
	std::tuple<double, double, double> getElement(int id)
	{
		return points_[id];
	}
};

#endif