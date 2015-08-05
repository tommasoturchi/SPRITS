#ifndef MANIFOLD_HPP
#define MANIFOLD_HPP

#include <functional>
#include <map>
#include <vector>
#include <utility>

#include <boost/bind.hpp>

namespace SPRITS
{
	enum class ElementEvent { ADD, REMOVE, UPDATE };
		
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
	public:
		ManifoldObserver(Manifold<T>* man) : man_(man), con_(man->subscribe(boost::bind(&ManifoldObserver::fire, this, _1, _2))) { }
		
		virtual void fire(const ElementEvent& event, int id) = 0;
	private:
		Manifold<T>* man_;
		boost::signals2::connection con_;
	};
}

#endif