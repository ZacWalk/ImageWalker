#pragma once

namespace Delegate
{
	class Base
	{
		Base(const Base &other);
		void operator=(const Base &other);
	public:
		Base() {};
		virtual void InvokeDelegate0() = 0;
	};	

	template<class T>
	class Delegate0 : public Base
	{
		T *_p;
		void (__thiscall T::*_func)(void);
	public:
		Delegate0(T *p, void (__thiscall T::*func)(void)) : _p(p), _func(func) {}
		void InvokeDelegate0() { (_p->*_func)(); };
	};

	class List0
	{
	private:
		typedef std::vector<Delegate::Base*> DelegateVector;
		DelegateVector _delegates;

	public:

		List0()
		{
		}

		~List0()
		{
			std::for_each(_delegates.begin(), _delegates.end(), IW::delete_object());
		}

		void Bind(Delegate::Base *d)
		{
			_delegates.push_back(d);
		}

		template<class T>
		void Bind(T *pT, void (__thiscall T::*func)())
		{
			Bind(new Delegate::Delegate0<T>(pT, func));
		}

		void Remove(Delegate::Base *d)
		{
			DelegateVector::iterator i = std::find(_delegates.begin(), _delegates.end(), d);
			if (_delegates.end() != i)
				_delegates.erase(i);
		}

		void Invoke()
		{		
			std::for_each(_delegates.begin(), _delegates.end(), InvokeDelegate);
		}

		static void InvokeDelegate(Delegate::Base *d)
		{
			d->InvokeDelegate0();
		}
	};
}


