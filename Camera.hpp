#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <map>
#include <vector>

#include <boost/bind.hpp>
#include <boost/signals2.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>

#include <opencv2/opencv.hpp>

#include <spdlog/spdlog.h>

#include <Space.hpp>

namespace SPRITS
{
	enum class NewFrameEvent { COLOR, DEPTH };
	
	class Camera
	{
	private:
		std::thread thread_;
		cv::Mat debugFrame_;
		boost::tuple<bool, boost::tuple<int, int>, boost::tuple<int, int>, boost::tuple<int, int>> croppingData_;
		bool crop_, debug_;
		std::map<NewFrameEvent, boost::signals2::signal<void(const NewFrameEvent&, const cv::Mat&)>> signals_;
	protected:
		virtual void setCropping(boost::tuple<int, int> origin, boost::tuple<int, int> target) = 0;
		
		virtual void resetCropping() = 0;
		
		static void croppingEvent(int event, int x, int y, int flags, void* cam);
	public:
		Camera(bool crop = false, bool debug = false) : crop_(crop), debug_(debug), debugFrame_(cv::Mat(800, 800, CV_8UC3, cv::Scalar(255,0,255))), croppingData_(boost::make_tuple(false, boost::tuple<int,int>(), boost::tuple<int,int>(), boost::make_tuple(0, 0)))
		{
			if (crop)
			{
				cv::namedWindow("Crop");
				cv::setMouseCallback("Crop", &Camera::croppingEvent, this);
			} else if (debug)
				cv::namedWindow("Debug");
		}
		
		template <typename Observer>
		boost::signals2::connection subscribe(const NewFrameEvent& event, Observer&& observer, boost::signals2::connect_position position = boost::signals2::at_back)
		{
			return signals_[event].connect(std::forward<Observer>(observer), position);
		}
		
		void notify(const NewFrameEvent& event, const cv::Mat& frame)
		{
			if (event == NewFrameEvent::COLOR)
			{
				if (crop_ || debug_)
					frame.copyTo(debugFrame_);
			}
			if (!crop_)
				signals_[event](event, frame);
		}
		
		virtual ~Camera()
		{
			cv::destroyAllWindows();
			for (auto&& sig : signals_ | boost::adaptors::map_values) sig.disconnect_all_slots();
		}
		
		virtual void update() {
			if (crop_)
			{
				if (boost::get<0>(croppingData_) && (boost::get<1>(croppingData_) != boost::tuple<int,int>()) && (boost::get<2>(croppingData_) != boost::tuple<int,int>()))
					cv::rectangle(debugFrame_, cv::Point(boost::get<0>(boost::get<1>(croppingData_)), boost::get<1>(boost::get<1>(croppingData_))), cv::Point(boost::get<0>(boost::get<2>(croppingData_)), boost::get<1>(boost::get<2>(croppingData_))), cv::Scalar(0, 0, 255), 3, 8, 0);
				cv::putText(debugFrame_, "Press ENTER to submit.", cv::Point((debugFrame_.cols - cv::getTextSize("Press ENTER to submit.", cv::FONT_HERSHEY_SIMPLEX, 0.6, 3, NULL).width)/2, 25), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255)); // Add dynamic resizing
				cv::imshow("Crop", debugFrame_);
				if ((cv::waitKey(1) & 0xFF) == 10)
				{
					crop_ = false;
					cv::destroyWindow("Crop");
					if (debug_)
						cv::namedWindow("Debug");
				}
			}
			if (!crop_ && debug_)
			{
				cv::imshow("Debug", debugFrame_);
				cv::waitKey(1);
			}
		}
	};
	
	template<typename T> class CameraObserverDecorator;
	
	template<typename T> class CameraObserver
	{
	protected:
		CameraObserver(Camera *cam, Space<T> *spc) : cam_(cam), spc_(spc) { }
		Camera *cam_;
		Space<T> *spc_;
		std::map<NewFrameEvent, boost::signals2::connection> con_;
		friend class CameraObserverDecorator<T>;
	public:
		CameraObserver(Camera *cam, Space<T> *spc, const NewFrameEvent& event) : cam_(cam), spc_(spc) {
			con_[event] = cam->subscribe(event, boost::bind(&CameraObserver<T>::fire, this, _1, _2));
		}

		template<typename E, typename... Events>
		CameraObserver(Camera *cam, Space<T> *spc, E event, Events... events) : cam_(cam), spc_(spc)
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
		CameraObserverDecorator(CameraObserver<T> *component, const NewFrameEvent& event) : CameraObserver<T>(component->cam_, component->spc_), component_(component)
		{
			this->con_[event] = this->cam_->subscribe(event, boost::bind(&CameraObserver<T>::fire, this, _1, _2), boost::signals2::at_front);
		}
		
		virtual ~CameraObserverDecorator()
		{
			delete component_;
		}
		
		template<typename E, typename... Events>
		CameraObserverDecorator(CameraObserver<T> *component, E event, Events... events) : CameraObserver<T>(component->cam_, component->spc_), component_(component)
		{
			for (auto event : { event, [](const NewFrameEvent& event) { return event; }(std::forward<Events>(events)...) })
				this->con_[event] = this->cam_->subscribe(event, boost::bind(&CameraObserver<T>::fire, this, _1, _2), boost::signals2::at_front);
		}
	};

	void Camera::croppingEvent(int event, int x, int y, int flags, void* cam_){
		Camera* cam = (Camera*)cam_;
		if (boost::get<0>(cam->croppingData_)) {
			if (event == cv::EVENT_MOUSEMOVE) {
				boost::get<0>(boost::get<2>(cam->croppingData_)) = x;
				boost::get<1>(boost::get<2>(cam->croppingData_)) = y;
			} else if ((event == CV_EVENT_LBUTTONUP) && (boost::get<1>(cam->croppingData_) != boost::tuple<int,int>()) && (boost::get<2>(cam->croppingData_) != boost::tuple<int,int>()) && ((std::max(boost::get<0>(boost::get<1>(cam->croppingData_)), boost::get<0>(boost::get<2>(cam->croppingData_))) - std::min(boost::get<0>(boost::get<1>(cam->croppingData_)), boost::get<0>(boost::get<2>(cam->croppingData_)))) * (std::max(boost::get<1>(boost::get<1>(cam->croppingData_)), boost::get<1>(boost::get<2>(cam->croppingData_))) - std::min(boost::get<1>(boost::get<1>(cam->croppingData_)), boost::get<1>(boost::get<2>(cam->croppingData_)))) > 10))
			{
				boost::get<0>(cam->croppingData_) = false;
				cam->setCropping(boost::make_tuple(std::min(boost::get<0>(boost::get<1>(cam->croppingData_)), boost::get<0>(boost::get<2>(cam->croppingData_))) + boost::get<0>(boost::get<3>(cam->croppingData_)), std::min(boost::get<1>(boost::get<1>(cam->croppingData_)), boost::get<1>(boost::get<2>(cam->croppingData_))) + boost::get<1>(boost::get<3>(cam->croppingData_))), boost::make_tuple(std::max(boost::get<0>(boost::get<1>(cam->croppingData_)), boost::get<0>(boost::get<2>(cam->croppingData_))) + boost::get<0>(boost::get<3>(cam->croppingData_)), std::max(boost::get<1>(boost::get<1>(cam->croppingData_)), boost::get<1>(boost::get<2>(cam->croppingData_))) + boost::get<1>(boost::get<3>(cam->croppingData_))));
				boost::get<0>(boost::get<3>(cam->croppingData_)) += std::min(boost::get<0>(boost::get<1>(cam->croppingData_)), boost::get<0>(boost::get<2>(cam->croppingData_)));
				boost::get<1>(boost::get<3>(cam->croppingData_)) += std::min(boost::get<1>(boost::get<1>(cam->croppingData_)), boost::get<1>(boost::get<2>(cam->croppingData_)));
				boost::get<1>(cam->croppingData_) = boost::tuple<int,int>();
				boost::get<2>(cam->croppingData_) = boost::tuple<int,int>();
			}
		} else if (event == CV_EVENT_LBUTTONDOWN) {
			boost::get<0>(cam->croppingData_) = true;
			boost::get<0>(boost::get<1>(cam->croppingData_)) = x;
			boost::get<1>(boost::get<1>(cam->croppingData_)) = y;
		} else if (event == CV_EVENT_RBUTTONUP)
		{
			cam->croppingData_ = boost::make_tuple(false, boost::tuple<int,int>(), boost::tuple<int,int>(), boost::make_tuple(0, 0));
			cam->resetCropping();
		}
	}
}

#endif