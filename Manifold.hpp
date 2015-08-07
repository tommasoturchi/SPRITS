#ifndef MANIFOLD_HPP
#define MANIFOLD_HPP

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
	  
	template<typename T> class Manifold
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
	};
	
	template<typename T> class ManifoldObserver
	{
	protected:
		Manifold<T>* man_;
		boost::signals2::connection con_;
	public:
		ManifoldObserver(Manifold<T>* man) : man_(man), con_(man->subscribe(boost::bind(&ManifoldObserver::fire, this, _1, _2))) { }
		
		virtual void fire(const ElementEvent& event, int id) = 0;
	};
}

#endif