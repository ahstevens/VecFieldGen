#pragma once

#include <vector>
#include <algorithm>

class Object;

namespace BroadcastSystem
{
	enum EVENT {
		MOUSE_CLICK,
		MOUSE_UNCLICK,
		MOUSE_MOVE,
		MOUSE_SCROLL,
		KEY_PRESS,
		KEY_UNPRESS,
		KEY_REPEAT
	};

	class Listener
	{
	public:
		virtual ~Listener() {}
		virtual void receiveEvent(Object* obj, const int event, void* data) = 0;	
	};

	class Broadcaster
	{
	public:
		void attach(Listener *obs)
		{
			m_vpListeners.push_back(obs);
		}

		void detach(Listener *obs)
		{
			m_vpListeners.erase(std::remove(m_vpListeners.begin(), m_vpListeners.end(), obs), m_vpListeners.end());
		}

	protected:
		virtual void notify(Object* obj, const int event, void* data = NULL)
		{
			for (auto obs : m_vpListeners) obs->receiveEvent(obj, event, data);
		}

	private:
		std::vector<Listener *> m_vpListeners;
	};
 }