#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <map>
#include <vector>

#include <boost/bind.hpp>
#include <boost/signals2.hpp>
#include <boost/range/adaptor/map.hpp>

#include <opencv2/opencv.hpp>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/fl_draw.H>

#include <Manifold.hpp>

namespace SPRITS
{
	enum class NewFrameEvent { COLOR, DEPTH };
	
	class Camera
	{
	private:
		Fl_Window* calibWin_;
		Fl_Window* debugWin_;
		Fl_Box* calibBox_;
		Fl_Box* debugBox_;
		Fl_Button* submitBut_;
		Fl_Button* resetBut_;
		bool calib_, debug_;
		std::map<NewFrameEvent, boost::signals2::signal<void(const NewFrameEvent&, const cv::Mat&)>> signals_;
		
		class CalibrationBox : public Fl_Box
		{
		private:
			bool select_;
			std::tuple<int, int> origin_, target_;
			Camera* cam_;
		public:
			CalibrationBox(int x, int y, int w, int h, Camera* cam) : Fl_Box(x, y, w, h), cam_(cam) { }
			
			int handle(int e)
			{
				auto coords = std::tuple<int, int>{Fl::event_x(), Fl::event_y()};
				switch (e)
				{
					case FL_PUSH:
					origin_ = coords;
					return 1;
					case FL_DRAG:
					target_ = coords;
					select_ = true;
					return 1;
					case FL_RELEASE:
					if (select_)
					{
						if (std::get<0>(origin_) < std::get<0>(target_))
						{
							if (std::get<1>(origin_) < std::get<1>(target_))
								cam_->setCalibration(origin_, target_);
							else
								cam_->setCalibration(std::tuple<int, int>{std::get<0>(origin_), std::get<1>(target_)}, std::tuple<int, int>{std::get<0>(target_), std::get<1>(origin_)});
						} else
						{
							if (std::get<1>(origin_) < std::get<1>(target_))
								cam_->setCalibration(std::tuple<int, int>{std::get<0>(target_), std::get<1>(origin_)}, std::tuple<int, int>{std::get<0>(origin_), std::get<1>(target_)});
							else
								cam_->setCalibration(target_, origin_);
						}
						select_ = false;
					}
					default:
					return Fl_Box::handle(e);
				}
			}
			
			void draw()
			{
				Fl_Box::draw();
				if (select_)
				{
					fl_line_style(FL_SOLID, 2);
					int width = std::abs(std::get<0>(origin_) - std::get<0>(target_));
					int height = std::abs(std::get<1>(origin_) - std::get<1>(target_));
					if (std::get<0>(origin_) < std::get<0>(target_))
					{
						if (std::get<1>(origin_) < std::get<1>(target_))
							fl_rect(std::get<0>(origin_), std::get<1>(origin_), width, height, FL_RED);
						else
							fl_rect(std::get<0>(origin_), std::get<1>(target_), width, height, FL_RED);
					} else
					{
						if (std::get<1>(origin_) < std::get<1>(target_))
							fl_rect(std::get<0>(target_), std::get<1>(origin_), width, height, FL_RED);
						else
							fl_rect(std::get<0>(target_), std::get<1>(target_), width, height, FL_RED);
					}
				}
			}
		};

		static void resetCallback(Fl_Widget* o, void* cam)
		{
			((Camera*)cam)->resetCalibration();
		}
		
		static void submitCallback(Fl_Widget* o, void* cam)
		{
			((Camera*)cam)->calibWin_->hide();
			delete ((Camera*)cam)->calibWin_;
			((Camera*)cam)->calib_ = false;
		}
	protected:
		virtual void setCalibration(std::tuple<int, int> origin, std::tuple<int, int> target) = 0;
		
		virtual void resetCalibration() = 0;
	public:
		Camera(bool calibrate = true, bool debug = false) : calib_(calibrate), debug_(debug)
		{
			if (calib_)
			{
				calibWin_ = new Fl_Window(0, 0, 0, 0, "Calibration");
				calibWin_->begin();
				calibBox_ = new CalibrationBox(0, 0, 0, 0, this);
				submitBut_ = new Fl_Button(0, 0, 0, 0, "OK");
				submitBut_->callback(submitCallback, (void*)&*this);
				calibWin_->callback(submitCallback, (void*)&*this);
				resetBut_ = new Fl_Button(0, 0, 0, 0, "Reset");
				resetBut_->callback(resetCallback, (void*)&*this);
				calibWin_->end();
			}
			if (debug_)
			{
				debugWin_ = new Fl_Window(0, 0, 0, 0, "Debug");
				debugWin_->begin();
				debugBox_ = new Fl_Box(0, 0, 0, 0);
				debugWin_->end();
			}
			Fl::visual(FL_RGB);
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
				if (calib_)
				{
					if ((frame.cols != calibBox_->w()) || (frame.rows != calibBox_->h()))
					{
						calibBox_->resize(0, 0, frame.cols, frame.rows);
						submitBut_->resize(10, frame.rows + 5, std::floor(frame.cols/2) - 20 , 30);
						resetBut_->resize(std::floor(frame.cols/2) + 10, frame.rows + 5, std::floor(frame.cols/2) - 20, 30);
						calibWin_->resize(0, 0, frame.cols, frame.rows + 40);
					}
					Fl_RGB_Image* colorFlImage = new Fl_RGB_Image((unsigned char*)frame.data, frame.cols, frame.rows, frame.channels());
					calibBox_->image(colorFlImage);
					calibWin_->redraw();
				} else if (debug_)
				{
					if ((frame.cols != debugBox_->w()) || (frame.rows != debugBox_->h()))
					{
						debugBox_->resize(0, 0, frame.cols, frame.rows);
						debugWin_->resize(0, 0, frame.cols, frame.rows);
					}
					Fl_RGB_Image* colorFlImage = new Fl_RGB_Image((unsigned char*)frame.data, frame.cols, frame.rows, frame.channels());
					debugBox_->image(colorFlImage);
					debugWin_->redraw();
				}
			}
			if (!calib_)
				signals_[event](event, frame);
		}
		
		virtual ~Camera()
		{
			if (calib_)
			{
				calibWin_->hide();
				delete calibWin_;
			}
			if (debug_)
			{
				debugWin_->hide();
				delete debugWin_;
			}
			for (auto&& sig : signals_ | boost::adaptors::map_values) sig.disconnect_all_slots();
		}
		
		virtual void update() {
			if (calib_ && !calibWin_->shown())
				calibWin_->show();
			if (!calib_ && debug_ && !debugWin_->shown())
				debugWin_->show();
			Fl::wait();
		}
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