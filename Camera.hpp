#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <map>
#include <vector>

#include <boost/bind.hpp>
#include <boost/signals2.hpp>
#include <boost/range/adaptor/map.hpp>

#include <opencv2/opencv.hpp>

#include <Manifold.hpp>

namespace SPRITS
{
	enum class NewFrameEvent { COLOR, DEPTH };
	
	class Camera
	{
	private:
		std::map<NewFrameEvent, boost::signals2::signal<void(const NewFrameEvent&, const cv::Mat&)>> signals_;
	public:
		template <typename Observer>
		boost::signals2::connection subscribe(const NewFrameEvent& event, Observer&& observer, boost::signals2::connect_position position = boost::signals2::at_back)
		{
			return signals_[event].connect(std::forward<Observer>(observer), position);
		}
		
		void notify(const NewFrameEvent& event, const cv::Mat& frame)
		{
			signals_[event](event, frame);
		}
		
		virtual ~Camera()
		{
			for (auto&& sig : signals_ | boost::adaptors::map_values) sig.disconnect_all_slots();
		}
		
		virtual void update() { }
	};
	
	template<typename T> class CameraObserverDecorator;
	
	template<typename T> class CameraObserver
	{
	protected:
		CameraObserver(Camera *cam, Manifold<T> *man) : cam_(cam), man_(man) { }
		Camera *cam_;
		Manifold<T> *man_;
		std::map<NewFrameEvent, boost::signals2::connection> con_;
		friend class CameraObserverDecorator<T>;
	public:
		CameraObserver(Camera *cam, Manifold<T> *man, const NewFrameEvent& event) : cam_(cam), man_(man) {
			con_[event] = cam->subscribe(event, boost::bind(&CameraObserver<T>::fire, this, _1, _2));
		}

		template<typename E, typename... Events>
		CameraObserver(Camera *cam, Manifold<T> *man, E event, Events... events) : cam_(cam), man_(man)
		{
			for (auto event : { event, [](const NewFrameEvent& event) { return event; }(std::forward<Events>(events)...) })
				con_[event] = cam->subscribe(event, boost::bind(&CameraObserver<T>::fire, this, _1, _2));
		}
		
		virtual ~CameraObserver()
		{
			for (auto&& con : con_ | boost::adaptors::map_values) con.disconnect();
		}
		
		virtual void fire(const NewFrameEvent& event, const cv::Mat& frame) = 0;
	};
	
	template<typename T> class CameraObserverDecorator : public CameraObserver<T>
	{
	private:
		CameraObserver<T> *component_;
	public:
		CameraObserverDecorator(CameraObserver<T> *component, const NewFrameEvent& event) : CameraObserver<T>(component->cam_, component->man_), component_(component)
		{
			this->con_[event] = this->cam_->subscribe(event, boost::bind(&CameraObserver<T>::fire, this, _1, _2), boost::signals2::at_front);
		}
		
		template<typename E, typename... Events>
		CameraObserverDecorator(CameraObserver<T> *component, E event, Events... events) : CameraObserver<T>(component->cam_, component->man_), component_(component)
		{
			for (auto event : { event, [](const NewFrameEvent& event) { return event; }(std::forward<Events>(events)...) })
				this->con_[event] = this->cam_->subscribe(event, boost::bind(&CameraObserver<T>::fire, this, _1, _2), boost::signals2::at_front);
		}
	};
}

#endif