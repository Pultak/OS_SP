/*#include "Blockable.h"
#include "Synchronization.h"
#include <list>
/*
std::list<Synchronization::Semaphore*> listeners;

const std::unique_ptr<Synchronization::Spinlock> listenersLock = std::make_unique<Synchronization::Spinlock>(0);

void Blockable::addListener(Synchronization::Semaphore* newListener) {
	listenersLock->lock();
	listeners.push_back(newListener);
	listenersLock->unlock();
}

void Blockable::removeListener(const kiv_os::THandle thisHandle) {
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

void Blockable::notifyRemoveListeners(kiv_os::THandle actualHandle) {
	listenersLock->lock();
	for (auto const& listener : listeners) {
		//wake up the slave
		if (!listener->notified) {
			listener->notified = true;
			listener->notifierHandle = actualHandle;
			listener->notify();

		}
	}
	listeners.clear();
	listenersLock->unlock();
}*/