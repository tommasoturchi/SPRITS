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
		Fl_Window *window;
		Fl_Box *colorBox;
		bool calibration;
		int waiting;
		std::map<NewFrameEvent, boost::signals2::signal<void(const NewFrameEvent&, const cv::Mat&)>> signals_;
		class CalibrationBox : public Fl_Box
		{
		private:
			bool calibrating;
			std::tuple<int, int> origin, target;
		public:
			CalibrationBox(int x, int y, int w, int h, const char* l = 0) : Fl_Box(x, y, w, h, l) { }
			
			int handle(int e)
			{
				auto coords = std::make_tuple<int, int>(Fl::event_x(), Fl::event_y());
				switch (e)
				{
					case FL_PUSH:
					origin = coords;
					return 1;
					case FL_DRAG:
					target = coords;
					calibrating = true;
					return 1;
					case FL_RELEASE:
					std::cout << "(" << std::get<0>(coords) << "," << std::get<1>(coords) << ") RELEASE!" << std::endl;
					calibrating = false;
					default:
					return Fl_Box::handle(e);
				}
			}
			
			void draw()
			{
				Fl_Box::draw();
				if (calibrating)
				{
					fl_line_style(FL_SOLID, 2);
					int width = std::abs(std::get<0>(origin) - std::get<0>(target));
					int height = std::abs(std::get<1>(origin) - std::get<1>(target));
					if (std::get<0>(origin) < std::get<0>(target))
					{
						if (std::get<1>(origin) < std::get<1>(target))
							fl_rect(std::get<0>(origin), std::get<1>(origin), width, height, FL_RED);
						else
							fl_rect(std::get<0>(origin), std::get<1>(target), width, height, FL_RED);
					} else
					{
						if (std::get<1>(origin) < std::get<1>(target))
							fl_rect(std::get<0>(target), std::get<1>(origin), width, height, FL_RED);
						else
							fl_rect(std::get<0>(target), std::get<1>(target), width, height, FL_RED);
					}
				}
			}
		};
	public:
		Camera(bool calibrate = true) : calibration(calibrate)
		{
			if (calibrate)
			{
				window = new Fl_Window(0, 0, 640, 520, "Calibration");
				window->begin();
				colorBox = new CalibrationBox(0, 0, 640, 480);
				Fl_Button* submit = new Fl_Button(10, 485, 300, 30, "OK");
				Fl_Button* reset = new Fl_Button(330, 485, 300, 30, "Reset");
				window->end();
				Fl::visual(FL_RGB);
				window->show();
			}
		}
		
		template <typename Observer>
		boost::signals2::connection subscribe(const NewFrameEvent& event, Observer&& observer, boost::signals2::connect_position position = boost::signals2::at_back)
		{
			return signals_[event].connect(std::forward<Observer>(observer), position);
		}
		
		void notify(const NewFrameEvent& event, const cv::Mat& frame)
		{
			if ((event == NewFrameEvent::COLOR) && (calibration))
			{
				cv::Mat outImage;
				outImage = frame;
				IplImage ipltemp = outImage;
				IplImage* colorIplImage = cvCreateImage(cvSize(frame.cols, frame.rows), 8, frame.channels());
				cvCopy(&ipltemp, colorIplImage);
				Fl_RGB_Image* colorFlImage = new Fl_RGB_Image((unsigned char*)colorIplImage->imageData, frame.cols, frame.rows, frame.channels(), colorIplImage->widthStep);
				Fl::lock();
				colorBox->image(colorFlImage);
				window->redraw();
				Fl::unlock();
				cvReleaseImage(&colorIplImage);
				colorFlImage->uncache();
				Fl::awake();
			} else if (!calibration)
				signals_[event](event, frame);
		}
		
		virtual ~Camera()
		{
			if (calibration)
			{
				window->hide();
				Fl::delete_widget(colorBox);
				delete window;
			}
			for (auto&& sig : signals_ | boost::adaptors::map_values) sig.disconnect_all_slots();
		}
		
		virtual void update() {
			//if (calibration && (waiting > 0))
				waiting = Fl::wait();
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