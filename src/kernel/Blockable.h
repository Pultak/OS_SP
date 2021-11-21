#pragma once


class Blockable {
private:
	std::list<SleepListener*> listeners;

	Synchronization::Spinlock* listenersLock = new Synchronization::Spinlock(0);

public:
	void addListener(SleepListener* newListener) {
		listenersLock->lock();
		listeners.push_back(newListener);
		listenersLock->unlock();
	}

	void removeListener(const kiv_os::THandle thisHandle) {
		listenersLock->lock();
		auto i = listeners.begin();
		while (i != listeners.end()) {
			//remove all inserted listeners
			if ((*i)->sleeperHandle == thisHandle) {

				listeners.erase(i++);
			}
		}
		listenersLock->unlock();
	}


	void notifyRemoveListeners() {
		listenersLock->lock();
		for (auto const& listener : listeners) {
			//wake up the slave
			listener->lock->unlock();
		}
		listeners.clear();
		listenersLock->unlock();
	}


	~Blockable() {
		delete listenersLock;
	}
};