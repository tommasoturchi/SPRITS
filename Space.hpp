#ifndef SPACE_HPP
#define SPACE_HPP

#include <functional>
#include <map>
#include <vector>
#include <utility>

#include <boost/bind.hpp>

namespace SPRITS
{
	enum ElementEvent_type { ADD, REMOVE, UPDATE };

	class ElementEvent
	{
	public:
	  ElementEvent(ElementEvent_type type) : type_(type) {}
	  const ElementEvent_type get_state() const { return type_; }
	  const char* get_state_as_string() const {
		  static const char* ttype[] = { "ADD", "REMOVE", "UPDATE" };
		  return ttype[type_];
	  }
	private:
	  ElementEvent_type type_;
	};
	  
	template<typename T> class Space
	{
	private:
		boost::signals2::signal<void(const ElementEvent&, int)> signal_;
	public:
		template <typename Observer>
		boost::signals2::connection subscribe(Observer&& observer)
		{
			return signal_.connect(std::forward<Observer>(observer));
		}
		
		virtual T getElement(int id) = 0;
		
		virtual void setElement(int id) = 0;
		
		virtual void setElement(int id, T point) = 0;
		
		void notify(const ElementEvent& event, int id)
		{
			signal_(event, id);
		}
		
		virtual ~Space()
		{
			signal_.disconnect_all_slots();
		}
	};
	
	template<typename T> class SpaceObserver
	{
	protected:
		Space<T>* spc_;
		boost::signals2::connection con_;
	public:
		SpaceObserver(Space<T>* spc) : spc_(spc), con_(spc->subscribe(boost::bind(&SpaceObserver::fire, this, _1, _2))) { }
		
		virtual ~SpaceObserver()
		{
			con_.disconnect();
		}
		
		virtual void fire(const ElementEvent& event, int id) = 0;
	};
}

#endif